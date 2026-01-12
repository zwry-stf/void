#pragma once
#include <atomic>
#include <void/def.h>
#include <cstdint>


void_begin_

enum class keybind_mode : std::uint8_t {
    hold,
    toggle,
    always,
};

class keybind {
private:
    std::atomic<std::uint8_t> pressed_{ 0u };

public:
    constexpr keybind() noexcept = default;

public:
    [[nodiscard]] bool get() const noexcept { 
        return pressed_.load(std::memory_order_acquire) != 0u;
    }
    [[nodiscard]] explicit operator bool() const {
        return get();
    }

    friend class keybind_owner;
};

void_end_