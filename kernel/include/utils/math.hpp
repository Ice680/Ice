#pragma once

#include <cstddef>
#include <cstdint>
#include <kernel.hpp>
#include <type_traits>

namespace utils {
struct point {
    size_t x = 0;
    size_t y = 0;
};

constexpr auto align_down(auto n, auto a) {
    return (n & ~(a - 1));
}

constexpr auto align_up(auto n, auto a) {
    return align_down(n + a - 1, a);
}

constexpr auto div_roundup(auto n, auto a) {
    return align_down(n + a - 1, a) / a;
}

constexpr auto next_pow2(uint64_t n) {
    return n == 1 ? 1 : 1 << (64 - __builtin_clzl(n - 1));
}

template <typename Enum>
constexpr auto as_int(Enum const value) ->
    typename std::underlying_type<Enum>::type {
    return static_cast<typename std::underlying_type<Enum>::type>(value);
}

template <typename Type>
constexpr Type pow(Type base, Type exp) {
    int result = 1;
    for (; exp > 0; exp--)
        result *= base;
    return result;
}

template <typename Type>
constexpr Type abs(Type num) {
    return num < 0 ? -num : num;
}

template <typename Type>
constexpr Type sign(Type num) {
    return (num > 0) ? 1 : ((num < 0) ? -1 : 0);
}

template <typename Type>
constexpr inline bool ishh(Type a) {
    return uintptr_t(a) >= hhdm_offset;
}

template <typename Type>
constexpr inline Type tohh(Type a) {
    return ishh(a) ? a : Type(uintptr_t(a) + hhdm_offset);
}

template <typename Type>
constexpr inline Type fromhh(Type a) {
    return !ishh(a) ? a : Type(uintptr_t(a) - hhdm_offset);
}
}  // namespace utils