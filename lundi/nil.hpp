#pragma once

namespace lua {

// Nil type used to provide optionality in the variant
struct nil{};

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, nil n) {
    return os << "nil";
}

}