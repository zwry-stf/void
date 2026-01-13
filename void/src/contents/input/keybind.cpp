#include "keybind.h"
#include <void/void.h>


void_begin_

void keybind_owner::update(void_* instance)
{
    if (disabled())
        return;

    if (static_cast<keybind_mode>(mode_) == keybind_mode::always) {
        key_bind_->pressed_.store(true, std::memory_order_release);
        return;
    }

    if (key_.has_key &&
        key_.storage.key == key::none)
        return;

    const bool key_down = key_.has_key ?
        instance->input().is_key_down(key_.storage.key) :
        instance->input().is_mouse_down(key_.storage.mouse_button);

    switch (static_cast<keybind_mode>(mode_)) {
    case keybind_mode::hold:
        key_bind_->pressed_.store(key_down, std::memory_order_release);
        break;
    case keybind_mode::toggle:
        if (key_down != last_key_down_) {
            last_key_down_ = key_down;

            if (key_down) {
                key_bind_->pressed_.fetch_xor(1u, std::memory_order_release);
            }
        }
        break;
    }
}

void_end_