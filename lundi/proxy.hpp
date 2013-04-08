#pragma once

#include <string>

namespace lua {

template<typename Variant, typename State>
class proxy {
    friend State;
    std::string name_;
    State &parent_;

    proxy(std::string name, State &parent)
    : name_(std::move(name))
    , parent_(parent) {
    }

    public:
    void operator=(Variant const &v) {
        parent_.set_global(name_, v);
    }

    template<typename FuncType>
    void operator=(FuncType &func) {
        parent_.register_function(name_, func);
    }

    Variant get() const {
        return parent_.get_global(name_);
    }

    operator Variant() const {
        return get();
    }

    template<typename... Args>
    Variant operator() (Args&&... args) {
        return parent_.call(name_, std::forward<Args>(args)...);
    }
};

}
