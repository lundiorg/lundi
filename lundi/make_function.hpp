#pragma once

#include <functional>

namespace lua {

struct not_pointer_to_member_function{};

template<typename T>
struct pointer_to_member_function_traits {
    typedef not_pointer_to_member_function type;
};

template<typename C, typename R, typename... P>
struct pointer_to_member_function_traits<R (C::*)(P...) const> {
    typedef std::function<R(P...)> type;
};

struct not_callable{};

template<typename C>
struct callable_traits {
    typedef typename pointer_to_member_function_traits<decltype(&C::operator())>::type type;
};

template<typename R, typename... P>
struct callable_traits<R(P...)> {
    typedef std::function<R(P...)> type;
};

template<typename Func>
typename callable_traits<Func>::type
make_function(const Func &func) {
    return func;
}

} // lua
