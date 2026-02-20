#pragma once
#include <void/contents/input/keybind.h>
#include <void/contents/input/input.h>
#include <void/util/xstr.h>
#include <functional>


void_begin_

struct keybind_key {
    union key_type {
        key key;
        mouse_button mouse_button;
    } storage;
    bool has_key{};
};

class keybind_owner {
private:
    keybind* const key_bind_;
    keybind_key key_;
    std::size_t mode_;
    bool last_key_down_;
    xstr name_{};
    
    std::function<bool()> disabled_callback_;

public:
    keybind_owner(keybind* key_bind, key key = key::none,
                  keybind_mode default_mode = keybind_mode::hold) noexcept
        : key_bind_(key_bind),
          mode_(static_cast<std::size_t>(default_mode)),
          last_key_down_(false), 
          disabled_callback_() {
        set_key(key);
    }
    
    keybind_owner(keybind* key_bind, mouse_button key, 
                  keybind_mode default_mode = keybind_mode::hold) noexcept
        : key_bind_(key_bind),
          mode_(static_cast<std::size_t>(default_mode)), 
          last_key_down_(false), 
          disabled_callback_() {
        set_key(key);
    }

public:
    void set_key(key key) noexcept {
        key_.has_key = true;
        key_.storage.key = key;
    }
    void set_key(mouse_button key) noexcept {
        key_.has_key = false;
        key_.storage.mouse_button = key;
    }
    void set_mode(keybind_mode mode) noexcept {
        mode_ = static_cast<std::size_t>(mode);
    }
    void set_name(const xstr& name) noexcept {
        name_ = name;
    }
    [[nodiscard]] bool has_key() const noexcept {
        return key_.has_key;
    }
    [[nodiscard]] auto key() const noexcept {
        assert(has_key());
        return key_.storage.key; 
    }
    [[nodiscard]] auto mouse_button() const noexcept {
        assert(!has_key());
        return key_.storage.mouse_button; 
    }
    [[nodiscard]] const auto& name() const noexcept {
        return name_;
    }
    [[nodiscard]] auto mode() const noexcept {
        return static_cast<keybind_mode>(mode_);
    }

    [[nodiscard]] auto& mode_ref() noexcept {
        return mode_;
    }
    [[nodiscard]] auto& key_ref() noexcept {
        return key_;
    }

    [[nodiscard]] bool value() const noexcept {
        return (bool)(*key_bind_);
    }

    void update(void_* instance);
    
    void set_disabled_callback(std::function<bool()>&& callback) {
        disabled_callback_ = std::move(callback);
    }

private:
    [[nodiscard]] bool disabled() const {
        return disabled_callback_ && disabled_callback_();
    }
};

void_end_