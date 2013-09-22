#pragma once

#include "../lundi.hpp"
#include "variant.hpp"

namespace lua {

class ref {
    state &state_;

public:
    /* Constructs a new reference to the element at the given index.
    * This create a strong reference, thus preventing collection
    * of the object. This does not alter the stack's state.
    */
    ref(state &state, int index)
        : state_(state) {
            lua_pushlightuserdata(state_.state_, this);
            lua_pushvalue(state_.state_, index);
            lua_rawset(state_.state_, LUA_REGISTRYINDEX);
    }

    /* Copies a reference. Similar to shared_ptr, this will increment
    * the reference count, requiring both copies to be destroyed for the
    * referenced object to be collectable.
    */
    ref(ref const &other)
        : state_(other.state_) {
            lua_pushlightuserdata(state_.state_, this);
            other.push();
            lua_rawset(state_.state_, LUA_REGISTRYINDEX);
    }

    /* Assigns a reference. This will decrease the refcount of the current
    * object and increase these of the assigned object.
    */
    ref &operator=(ref /*const*/ &other) {
        if (state_ != other.state_)
        {
        } // throw, or destroy + construct

        lua_pushlightuserdata(state_.state_, this);
        other.push();
        lua_rawset(state_.state_, LUA_REGISTRYINDEX);
    }

    /* Destroys the reference to the object. This may destroy a link
    * in such a way that the object will be collected.
    */
    ~ref() {
        lua_pushlightuserdata(state_.state_, this);
        lua_pushnil(state_.state_);
        lua_rawset(state_.state_, LUA_REGISTRYINDEX);
    }

    int push() const {
        lua_pushlightuserdata(state_.state_, const_cast<void *>(reinterpret_cast<void const *>(this)));
        lua_rawget(state_.state_, LUA_REGISTRYINDEX);
        return lua_gettop(state_.state_);
    }

    state &state() {
        return state_;
    }
};

template<typename Table>
class table_proxy {
    Table &table_;
    variant key_;

public:
    table_proxy(Table &table, variant key)
        : table_(table)
        , key_(key) {}

    operator variant() {
        return table_.get(key_);
    }

    variant &operator=(variant const& val) {
        table_.set(key_, val);
        return val;
    }
};

class table {
    ref ref_;

public:
    typedef table_proxy<table> proxy;

    table(ref const &ref)
        : ref_(ref) {}

    proxy operator [](variant const &key) {
        return proxy(*this, key);
    }

    void set(variant const &key, variant const &value) {
        lua::detail::push_variant push(ref_.state());
        int table_idx = ref_.push();
        boost::apply_visitor(push, key);
        boost::apply_visitor(push, value);
        lua_settable(ref_.state(), table_idx);
        lua_pop(ref_.state(), 1);
    }

    variant get(variant const &key) {
        lua::detail::push_variant push(ref_.state());
        int table_idx = ref_.push();
        boost::apply_visitor(push, key);
        lua_gettable(ref_.state(), table_idx);
        variant value = ref_.state().pop();
        lua_pop(ref_.state(), 1);
        return value;
    }
};

}
