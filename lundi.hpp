#pragma once

#include <vector>
#include <functional>
#include <exception>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/view/reverse_view.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <lua.hpp>

#include "lundi/variant.hpp"
#include "lundi/proxy.hpp"

namespace lua {

class exception : public std::runtime_error { public: exception(const std::string& s) : runtime_error(s) { } };

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
    typedef Ret FuncType(Args...);

    function_wrapper_impl(FuncType &f) : func(f) {}

    int operator()(lua_State *state) {
        boost::fusion::vector<Args...> params;
        boost::fusion::reverse_view<decltype(params)> r_params(params);
        for_each(r_params, fetch_parameter(state));
        variant result = invoke(func, params);
        boost::apply_visitor(detail::push_variant(state), result);
        return 1;
    }

private:
    FuncType *func;
};

template<typename Ret, typename... Args>
function_wrapper  *make_wrapper(Ret (&function)(Args...)) {
    return new function_wrapper_impl<Ret, Args...>(function);
}

int dispatch_to_wrapper(lua_State *state) {
    void *light_ud = lua_touserdata(state, lua_upvalueindex(1));
    function_wrapper *wrapper = reinterpret_cast<function_wrapper *>(light_ud);
    return (*wrapper)(state);
}

} // detail

class state {
    lua_State *state_;
    std::function<void(std::string const&)> error_func_;

    variant peek(int index) {
        switch(lua_type(state_, index)) {
            case LUA_TNUMBER:
                return lua_tonumber(state_, index);
            case LUA_TBOOLEAN:
                return lua_toboolean(state_, index);
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
            std::string error_msg (lua_tostring(state_, -1));
            lua_pop(state_, -1); // remove error message
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
    }

    void set_global(std::string const &name, variant value) {
        boost::apply_visitor(detail::push_variant(state_), value);
        lua_setglobal(state_, name.c_str());
    }

    variant get_global(std::string const &name) {
        lua_getglobal(state_, name.c_str());
        return pop();
    }

    proxy<variant, state> operator[](std::string name) {
        return proxy<variant, state>(std::move(name), *this);
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
    void register_function(std::string const &name, FuncType &func) {
        auto wrapper = detail::make_wrapper(func);
        register_wrapper(name, wrapper);
        wrappers.push_back(wrapper);
    }
};

} // lua
