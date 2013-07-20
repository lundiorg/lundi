#pragma once

namespace lua {

// Nil type used to provide optionality in the variant
struct nil_type{};

namespace {
const nil_type nil;
}

template<typename CharT, typename Traits>
std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, nil_type n) {
    return os << "nil";
}

inline
bool operator==(nil_type, nil_type) { return true; }

// these are unambiguously worse matches than the above
template<typename T>
bool operator==(nil_type, T const&) { return false; }

template<typename T>
bool operator==(T const&, nil_type) { return false; }

}
