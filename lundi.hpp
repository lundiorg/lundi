#pragma once

#include <vector>
#include <functional>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/view/reverse_view.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <istream>
#include <lua.hpp>

#include "lundi/variant.hpp"
#include "lundi/proxy.hpp"
#include "lundi/make_function.hpp"

namespace lua {

class exception : public virtual std::exception {
public:
    exception(const std::string& s) : s(s) {}
    exception(std::string&& s) : s(std::move(s)) {}

    char const* what() const noexcept override { return s.data(); }
private:
    std::string s;
};

namespace detail {

class function_wrapper {
public:
    virtual int operator()(lua_State *state) = 0;
    virtual ~function_wrapper() {};
};

struct fetch_parameter {
    lua_State *state;
    fetch_parameter(lua_State *s) : state(s) {}

    void operator()(std::string& t) const {
        t = lua_tostring(state, -1);
        lua_pop(state, 1);
    }

    void operator()(int& t) const {
        t = lua_tonumber(state, -1);
        lua_pop(state, 1);
    }
};

template<typename Ret, typename... Args>
class function_wrapper_impl : public function_wrapper {
public:
    typedef std::function<Ret(Args...)> FuncType;

    function_wrapper_impl(FuncType const &f) : func(f) {}

    int operator()(lua_State *state) {
        boost::fusion::vector<Args...> params;
        boost::fusion::reverse_view<decltype(params)> r_params(params);
        for_each(r_params, fetch_parameter(state));
        variant result = invoke(func, params);
        boost::apply_visitor(detail::push_variant(state), result);
        return 1;
    }

private:
    FuncType func;
};

template<typename Ret, typename... Args>
function_wrapper  *make_wrapper(std::function<Ret(Args...)> const &function) {
    return new function_wrapper_impl<Ret, Args...>(function);
}

inline
int dispatch_to_wrapper(lua_State *state) {
    void *light_ud = lua_touserdata(state, lua_upvalueindex(1));
    function_wrapper *wrapper = reinterpret_cast<function_wrapper *>(light_ud);
    return (*wrapper)(state);
}

template<typename StreamT>
static const char *read_stream(StreamT &stream, std::vector<char> &buffer, size_t &size) {
    stream.read(&buffer[0], buffer.size());
    size = stream.gcount();
    return &buffer[0];
}

template<typename StreamT>
static const char *stream_reader(lua_State *L, void *data, size_t *size) {
    auto &info = *reinterpret_cast<std::pair<std::reference_wrapper<StreamT>, std::vector<char>>*>(data);
    return read_stream<StreamT>(info.first, info.second, *size);
}

} // detail

template<typename StreamT>
char const *stream_name(StreamT &stream) {
    return "<stream>";
}

class state {
    lua_State *state_;
    std::function<void(std::string const&)> error_func_;

    variant peek(int index) {
        switch(lua_type(state_, index)) {
            case LUA_TNUMBER:
                return lua_tonumber(state_, index);
            case LUA_TBOOLEAN:
                return static_cast<bool>(lua_toboolean(state_, index));
            case LUA_TSTRING:
                return lua_tostring(state_, index);
            // TODO : function?
        }
        return nil();
    }

    variant pop() {
        variant value = peek(-1);
        lua_pop(state_, 1);
        return value;
    }

    // handles error values returned by various C API function
    // It isn't intended to be called directly!
    void protect (int err) {
        if (err != 0)
        {
            // check if any error message is present
            std::string error_msg;
            if (!lua_isstring(state_, -1)) {
                error_msg = "<no error message>";
            }
            else {
                error_msg = std::string(lua_tostring(state_, -1));
                lua_pop(state_, -1); // remove error message
            }
            if (error_func_)
                error_func_(error_msg);
        }
    }

    variant call_r(int nargs) {
        protect(lua_pcall(state_, nargs, 1, 0));
        return pop();
    }

    template<typename T, typename... Args>
    variant call_r(int nargs, T t, Args... args) {
        variant value = t;
        boost::apply_visitor(detail::push_variant(state_), value);
        return call_r(nargs + 1, args...);
    }

    void register_wrapper(std::string const &name, detail::function_wrapper *wrapper) {
        lua_pushlightuserdata(state_, wrapper);
        lua_pushcclosure(state_, detail::dispatch_to_wrapper, 1);
        lua_setglobal(state_, name.c_str());
    }

    std::vector<detail::function_wrapper *> wrappers;

public:
    template<typename Functor>
    state(Functor&& error_func) 
        : state_(luaL_newstate())
        , error_func_(std::forward<Functor>(error_func)) {
        // loads basic, table, I/O, string and math libraries
        luaL_openlibs(state_);
    }

    void set_global(std::string const &name, variant value) {
        boost::apply_visitor(detail::push_variant(state_), value);
        lua_setglobal(state_, name.c_str());
    }

    // This overload is used to fix "broken" bool overload with literals
    // It might be needed to set up a proper bool type from scratch.
    template <std::size_t N>
    void set_global(std::string const& name, char const(&value)[N]) { set_global(name, std::string(value)); }

    variant get_global(std::string const &name) {
        lua_getglobal(state_, name.c_str());
        return pop();
    }

    proxy<variant, state> operator[](std::string name) {
        return proxy<variant, state>(std::move(name), *this);
    }

    template<typename StreamT>
    typename boost::enable_if<boost::is_base_of<std::istream, StreamT>, void>::type eval(StreamT &stream) {
        using namespace detail;
        auto reader_info = std::make_pair(std::ref(stream), std::vector<char>(4096));
        protect(lua_load(state_, &stream_reader<StreamT>, &reader_info, stream_name(stream)));
        protect(lua_pcall(state_, 0, LUA_MULTRET, 0));
    }

    void eval(std::string const &program) {
        protect(luaL_dostring(state_, program.c_str()));
    }

    template<typename... Args>
        variant call(std::string const &name, Args... args) {
        lua_getglobal(state_, name.c_str());
        return call_r(0, args...);
    }

    template<typename FuncType>
    void register_function(std::string const &name, FuncType const &func) {
        auto wrapper = detail::make_wrapper(make_function(func));
        register_wrapper(name, wrapper);
        wrappers.push_back(wrapper);
    }
};

} // lua
