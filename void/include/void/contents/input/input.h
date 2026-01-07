#pragma once
#include <void/util/vobj.h>
#include "input_response.h"
#include "message_event.h"
#include "keybind.h"
#include <bitset>
#include <memory>
#include <vector>
#include <mutex>


void_begin_

class input : protected vobj {
private:
    float last_mouse_x_;
    float last_mouse_y_;

    std::bitset<static_cast<std::size_t>(key::max)> key_states_;
    std::bitset<static_cast<std::size_t>(mouse_button::max)> mouse_states_;

    std::unique_ptr<class keybind_manager> keybind_manager_;
    std::mutex messages_mutex_;
    std::vector<message_event> pending_messages_;

public:
    input(void_* instance) noexcept;
    ~input();

public:
#if defined(R2_PLATFORM_WINDOWS)
    input_response input_win32(std::uint32_t msg, uint64_t wparam, int64_t lparam);
#endif

#if defined(VOID_HAS_GLFW)
    input_response input_glfw_key(int key, int scancode, int action, int mods);
    input_response input_glfw_char(std::uint32_t codepoint);
    input_response input_glfw_mouse_button(int button, int action, int mods);
    input_response input_glfw_scroll(double xoffset, double yoffset);
#endif

    [[nodiscard]] int get_scan_code(key key);

public:
    void input_on_frame();

    [[nodiscard]] bool is_control_down() const noexcept {
        return key_states_[static_cast<std::size_t>(key::lctrl)];
    }
    [[nodiscard]] bool is_shift_down() const noexcept {
        return key_states_[static_cast<std::size_t>(key::lshift)];
    }
    [[nodiscard]] bool is_lmenu_down() const noexcept {
        return key_states_[static_cast<std::size_t>(key::lalt)];
    }

    [[nodiscard]] bool is_key_down(key key) const noexcept {
        return key_states_[static_cast<std::size_t>(key)];
    }
    [[nodiscard]] bool is_mouse_down(mouse_button button) const noexcept { 
        return mouse_states_[static_cast<std::size_t>(button)];
    }

    class keybind_owner* add_keybind(keybind* bind, key default_key = key::none,
                                     keybind_mode mode = keybind_mode::hold);

    void clear_queue();

private:
#if defined(R2_PLATFORM_WINDOWS)
    message_event convert_message_win32(std::uint32_t msg, uint64_t wparam, int64_t lparam);
    void input_on_frame_win32();

    void update_key_flags_win32();

    [[nodiscard]] int get_scan_code_win32(key key);
    [[nodiscard]] int get_virtual_key_win32(key key);
    [[nodiscard]] int get_virtual_key_win32(mouse_button key);
#endif

    input_response process_event(const message_event& event);
    input_response push_event(const message_event& event);

    void process_pending_events();
};

void_end_