#pragma once

#include <void/def.h>
#include <assert.h>
#include <cstdint>


void_begin_

enum class message_type : std::uint8_t {
    none = static_cast<std::uint8_t>(-1),
    ignore = 0,
    mouse_button_down,
    mouse_button_up,
    mouse_move,
    key_down,
    key_up,
    char_down,
    scroll,
};

enum class mouse_button : std::uint8_t {
    left,
    right,
    middle,
    xbutton1,
    xbutton2,

    max,
};

enum class key : std::uint8_t {
    none,
    // chars a-z
    a, b, c, d, e, f,
    g, h, i, j, k, l,
    m, n, o, p, q, r,
    s, t, u, v, w, x,
    y, z,
    // numbers 0-10
    n0, n1, n2, n3, n4, 
    n5, n6, n7, n8, n9,
    // function keys
    f1, f2, f3, f4, f5,
    f6, f7, f8, f9, f10,
    f11, f12,
    // navigation
    up, down, left, right,
    // modifiers
    lshift, rshift,
    lctrl,  rctrl,
    lalt,   ralt,
    // confirm/cancel
    enter,
    backspace,
    escape,
    tab,
    space,
    // system keys
    insert,
    del,
    home,
    end,

    max,
};

class message_event {
private:
    message_type msg_;
    float       mouse_x_;
    float       mouse_y_;
    union {
        float scroll_value_;
        key key_value_;
        mouse_button mouse_button_value_;
        std::uint32_t char_value_;
        bool mouse_in_window_;
    };

private:
    message_event() = default;

public:
    static [[nodiscard]] message_event from_mousemove(float mouse_x, float mouse_y, bool in_window) noexcept {
        message_event ret;
        ret.msg_ = message_type::mouse_move;
        ret.mouse_x_ = mouse_x;
        ret.mouse_y_ = mouse_y;
        ret.mouse_in_window_ = in_window;
        return ret;
    }

    static [[nodiscard]] message_event from_mouse_button(bool down, float mouse_x, float mouse_y, mouse_button key) noexcept {
        message_event ret;
        ret.msg_ = down ? message_type::mouse_button_down : message_type::mouse_button_up;
        ret.mouse_button_value_ = key;
        ret.mouse_x_ = mouse_x;
        ret.mouse_y_ = mouse_y;
        return ret;
    }

    static [[nodiscard]] message_event from_scroll(float mouse_x, float mouse_y, float scroll) noexcept {
        message_event ret;
        ret.msg_ = message_type::scroll;
        ret.mouse_x_ = mouse_x;
        ret.mouse_y_ = mouse_y;
        ret.scroll_value_ = scroll;
        return ret;
    }

    static [[nodiscard]] message_event from_key(bool down, key key) noexcept {
        message_event ret;
        ret.msg_ = down ? message_type::key_down : message_type::key_up;
        ret.key_value_ = key;
        if (key == key::none)
            ret.msg_ = message_type::ignore;
        return ret;
    }

    static [[nodiscard]] message_event from_char(std::uint32_t c) noexcept {
        message_event ret;
        ret.msg_ = message_type::char_down;
        ret.char_value_ = c;
        return ret;
    }

    static [[nodiscard]] message_event ignore() noexcept {
        message_event ret;
        ret.msg_ = message_type::ignore;
        return ret;
    }

    static [[nodiscard]] message_event none() noexcept {
        message_event ret;
        ret.msg_ = message_type::none;
        return ret;
    }

public:
    [[nodiscard]] bool is_message(message_type msg) const noexcept {
        return msg_ == msg; 
    }
    void get_cursor_pos(float& x, float& y) const noexcept {
        x = mouse_x_; y = mouse_y_;
    }

    [[nodiscard]] float get_scroll() const noexcept {
        assert(msg_ == message_type::scroll);
        return scroll_value_;
    }
    [[nodiscard]] key get_key() const noexcept {
        assert(msg_ == message_type::key_down || msg_ == message_type::key_up);
        return key_value_;
    }
    [[nodiscard]] mouse_button get_mouse_button() const noexcept {
        assert(msg_ == message_type::mouse_button_down || msg_ == message_type::mouse_button_up);
        return mouse_button_value_;
    }
    [[nodiscard]] bool mouse_in_window() const noexcept {
        assert(msg_ == message_type::mouse_move);
        return mouse_in_window_;
    }
    [[nodiscard]] std::uint32_t get_char() const noexcept {
        assert(msg_ == message_type::char_down);
        return char_value_;
    }
    [[nodiscard]] bool has_cursor_pos() const noexcept {
        return
            msg_ == message_type::mouse_move ||
            msg_ == message_type::mouse_button_down ||
            msg_ == message_type::mouse_button_up ||
            msg_ == message_type::scroll;
    }
};

void_end_