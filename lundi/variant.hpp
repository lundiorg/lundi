#pragma once

#include <boost/variant.hpp>
#include <lua.hpp>
#include "nil.hpp"
#include "ref.hpp"

namespace lua {
namespace detail {

template<typename T>
struct variant_friendly {
    typedef T type;
};

template<>
struct variant_friendly<unsigned int> {
    typedef double type;
};

template<typename State>
class variant_pusher : public boost::static_visitor<> {
    State state_;
public:
    variant_pusher(State const &state) : state_(state) {}

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

    void operator()(nil_type n) const {
        lua_pushnil(state_);
    }

    void operator()(ref r) const {
        r.push();
    }
};

} // detail

typedef boost::variant<signed int, double, std::string, bool, ref, nil_type> variant;

template<typename State>
void push(State const &state, variant const &val) {
    boost::apply_visitor(detail::variant_pusher<State>(state), val);
}

template<typename State>
variant pop(State const &state) {
    variant value;
    
    switch (lua_type(state, -1)) {
    case LUA_TNUMBER:
        value = lua_tonumber(state, -1);
    case LUA_TBOOLEAN:
        value = (lua_toboolean(state, -1) != 0); // Silent boolean conversion
    case LUA_TSTRING:
        value = lua_tostring(state, -1);
        // TODO : function?
    }

    lua_pop(state, 1);
    return value;
}

} // lua
