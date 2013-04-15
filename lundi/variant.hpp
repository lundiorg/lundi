#pragma once

#include <boost/variant.hpp>
#include <lua.hpp>
#include "nil.hpp"

namespace lua {

typedef boost::variant<signed int, double, std::string, bool, nil> variant;

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

    void operator()(std::string const& szc) const {
        lua_pushstring(state_, szc.c_str());
    }

    void operator()(nil n) const {
        lua_pushnil(state_);
    }
};

} // detail
} // lua
