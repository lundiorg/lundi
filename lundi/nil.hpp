#pragma once

namespace lua {

// Nil type used to provide optionality in the variant
struct nil{};

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, nil n) {
    return os << "nil";
}

bool operator==(nil, nil) { return true; }

// these are unambiguously worse matches than the above
template<typename T>
bool operator==(nil, T const&) { return false; }

template<typename T>
bool operator==(T const&, nil) { return false; }

}