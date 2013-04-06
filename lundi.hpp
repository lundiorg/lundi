#pragma once

#include <vector>
#include <boost/variant.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/view/reverse_view.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <lua.hpp>

namespace lua {

// Nil type used to provide optionality in the variant
struct nil{};

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, nil n) {
  return os << "nil";
}

typedef boost::variant<signed int, double, bool, char const*, nil> variant;

namespace detail {

class push_variant : public boost::static_visitor<> {
  lua_State *state_;
public:
  push_variant(lua_State *state) : state_(state) {}

  void operator()(signed int i) const {
    lua_pushinteger(state_, i);
  }

  void operator()(double d) const {
    lua_pushnumber(state_, d);
  }

  void operator()(bool b) const {
    lua_pushboolean(state_, b);
  }

  void operator()(char const *szc) const {
    lua_pushstring(state_, szc);
  }

  void operator()(nil n) const {
    lua_pushnil(state_);
  }
};

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

  variant peek(int index) {
    switch(lua_type(state_, index)) {
      case LUA_TNUMBER:
        return lua_tonumber(state_, index);
      case LUA_TBOOLEAN:
        return lua_toboolean(state_, index);
      case LUA_TSTRING:
        return lua_tostring(state_, index);
    }
    return nil();
  }

  variant pop() {
    variant value = peek(-1);
    lua_pop(state_, 1);
    return value;
  }

  variant call_r(int nargs) {
    lua_call(state_, nargs, 1);
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
  state() 
  : state_(luaL_newstate()) {
  }

  void set_global(std::string const &name, variant value) {
    boost::apply_visitor(detail::push_variant(state_), value);
    lua_setglobal(state_, name.c_str());
  }

  variant get_global(std::string const &name) {
    lua_getglobal(state_, name.c_str());
    return pop();
  }

  void eval(std::string const &program) {
    luaL_dostring(state_, program.c_str());
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
