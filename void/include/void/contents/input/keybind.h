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
    std::atomic<std::uint8_t> pressed_;

public:
    constexpr keybind() = default;

public:
    [[nodiscard]] bool get() const { return (bool)pressed_.load(); }
    [[nodiscard]] operator bool() const { return get(); }

    friend class keybind_owner;
};

void_end_