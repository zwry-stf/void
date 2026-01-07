#pragma once
#include <void/def.h>


void_begin_

class input_response {
private:
    bool handled_ = false;

private:
    explicit input_response(bool handled) noexcept
        : handled_(handled) { }

public:
    static [[nodiscard]] input_response handled() noexcept {
        return input_response(true);
    }

    static [[nodiscard]] input_response empty() noexcept {
        return input_response(false);
    }

public:
    [[nodiscard]] bool is_handled() const noexcept { 
        return handled_; 
    }
};

void_end_