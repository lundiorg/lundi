#pragma once

#include <boost/variant.hpp>
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
};

} // lua
