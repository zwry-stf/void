#pragma once
#include <void/def.h>
#include <cstdint>
#include <algorithm>


void_begin_

enum class e_resizing_type : std::uint8_t {
    none         = 0u,
    invalid      = static_cast<std::uint8_t>(-1),
    top          = (1u << 0u),
    bottom       = (1u << 1u),
    left         = (1u << 2u),
    right        = (1u << 3u),
    top_left     = top | left,
    top_right    = top | right,
    bottom_left  = bottom | left,
    bottom_right = bottom | right,
};

inline constexpr e_resizing_type operator|(const e_resizing_type& a, const e_resizing_type& b) noexcept {
    return static_cast<e_resizing_type>(
        std::to_underlying(a) | std::to_underlying(b)
    );
}
inline constexpr e_resizing_type& operator|=(e_resizing_type& a, const e_resizing_type& b) noexcept {
    a = a | b;
    return a;
}
inline constexpr bool operator&(const e_resizing_type& a, const e_resizing_type& b) noexcept {
    return static_cast<e_resizing_type>(std::to_underlying(a) & std::to_underlying(b)) == b;
}

void_end_