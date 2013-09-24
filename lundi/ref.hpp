#pragma once

#include "../lundi.hpp"
#include "variant.hpp"

namespace lua {

class ref {
    lua_State *state_;

public:
    /* Constructs a new reference to the element at the given index.
       This create a strong reference, thus preventing collection
       of the object. This does not alter the stack's state. */
    ref(lua_State *state, int index)
        : state_(state) {
            lua_pushlightuserdata(state_, this);
            lua_pushvalue(state_, index);
            lua_rawset(state_, LUA_REGISTRYINDEX);
    }

    /* Constructs a null reference. Yes, we need them. */
    ref() : state_() {}

    /* Copies a reference. Similar to shared_ptr, this will increment
       the reference count, requiring both copies to be destroyed for the
       referenced object to be collectable. */
    ref(ref const &other)
        : state_(other.state_) {
            lua_pushlightuserdata(state_, this);
            other.push();
            lua_rawset(state_, LUA_REGISTRYINDEX);
    }

    /* Assigns a reference. This will decrease the refcount of the current
       object and increase these of the assigned object. */
    ref &operator=(ref /*const*/ &other) {
        if (state_ != other.state_)
        {
        } // throw, or destroy + construct

        lua_pushlightuserdata(state_, this);
        other.push();
        lua_rawset(state_, LUA_REGISTRYINDEX);
        return *this;
    }

    /* Destroys the reference to the object. This may destroy a link
       in such a way that the object will be collected. */
    ~ref() {
        lua_pushlightuserdata(state_, this);
        lua_pushnil(state_);
        lua_rawset(state_, LUA_REGISTRYINDEX);
    }

    int push() const {
        lua_pushlightuserdata(state_, const_cast<void *>(reinterpret_cast<void const *>(this)));
        lua_rawget(state_, LUA_REGISTRYINDEX);
        return lua_gettop(state_);
    }

    lua_State *state() {
        return state_;
    }
};

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, ref const & r) {
    return os << "ref";
}

inline
bool operator==(ref const &, ref const &) { return false; }


template<typename TableT, typename ValueT>
class table_proxy {
    TableT &table_;
    ValueT key_;

public:
    table_proxy(TableT &table, ValueT const &key)
        : table_(table)
        , key_(key) {}

    operator ValueT() {
        return table_.get(key_);
    }

    const ValueT &operator=(ValueT const& val) {
        table_.set(key_, val);
        return val;
    }
};

template<typename ValueT>
class table {
    ref ref_;

public:
    typedef table_proxy<table, ValueT> proxy;

    table(ref const &ref)
        : ref_(ref) {}

    proxy operator [](ValueT const &key) {
        return proxy(*this, key);
    }

    void set(ValueT const &key, ValueT const &value) {
        int table_idx = ref_.push();
        push(ref_.state(), key);
        push(ref_.state(), value);
        lua_settable(ref_.state(), table_idx);
        lua_pop(ref_.state(), 1);
    }

    ValueT get(ValueT const &key) {
        int table_idx = ref_.push();
        push(ref_.state(), key);
        lua_gettable(ref_.state(), table_idx);
        ValueT value = pop(ref_.state());
        lua_pop(ref_.state(), 1);
        return value;
    }
};

}
