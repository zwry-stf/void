#pragma once
#include <void/def.h>
#include <void/util/def.h>


void_begin_

template <typename T>
class default_value {
private:
    T default_;
    T value_;

public:
    v_always_inline constexpr default_value(const T& value) 
        : default_(value),
          value_(value) { }
    v_always_inline default_value(const default_value& v)   
        : default_(v.default_),
          value_(v.value_) { }

    v_always_inline default_value& operator=(const T& value) {
        value_ = value;
        return *this;
    }

public:
    [[nodiscard]] v_always_inline T& operator()() noexcept { 
        return value_; 
    }
    [[nodiscard]] v_always_inline const T& operator()() const noexcept { 
        return value_; 
    }
    [[nodiscard]] v_always_inline T& get_default() noexcept {
        return default_; 
    }
    [[nodiscard]] v_always_inline const T& get_default() const noexcept { 
        return default_;
    }
    [[nodiscard]] v_always_inline T* operator->() noexcept {
        return &value_; 
    }
};

void_end_