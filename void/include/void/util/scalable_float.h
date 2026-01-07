#pragma once
#include <void/accessors/util.h>
#include <void/util/def.h>


void_begin_

class sfloat {
private:
    bool even_;
    float value_;

public:
    constexpr sfloat(float v, bool even) noexcept
        : value_(v),
          even_(even) { }
    
    constexpr explicit sfloat(float v) noexcept
        : value_(v),
          even_(false) { }

public:
    v_always_inline [[nodiscard]] float get(float scale) const noexcept {
        return even_ ?
            util::round_to_even(value_ * scale) :
            std::round(value_ * scale);
    }

    v_always_inline [[nodiscard]] float& raw() noexcept {
        return value_;
    }
    v_always_inline [[nodiscard]] float raw() const noexcept {
        return value_;
    }
};

void_end_