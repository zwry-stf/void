#pragma once
#include "events.h"


void_begin_

class overlay_render_input {
private:
    const std::int32_t opened_;

public:
    overlay_render_input(auto opened) noexcept
        : opened_(opened) { }

public:
    [[nodiscard]] bool is_opened(const input_receiver_overlay* r) const noexcept {
        return opened_ == r->id_;
    }
    [[nodiscard]] bool nothing_opened() const noexcept {
        return opened_ == -1;
    }
};

class overlay_input {
private:
    const message_event* event_{};
    std::int32_t* const opened_ptr_;

public:
    overlay_input(std::int32_t& opened)
        : opened_ptr_(&opened) { }

public:
    void set_event(const message_event* event) noexcept {
        event_ = event;
    }

    [[nodiscard]] const auto& event() const noexcept {
        return *event_;
    }

public:
    [[nodiscard]] bool is_opened(const input_receiver_overlay* r) const noexcept {
        return *opened_ptr_ == r->id_;
    }
    [[nodiscard]] bool nothing_opened() const noexcept {
        return *opened_ptr_ == -1;
    }

public:
    void set_opened(const input_receiver_overlay* r) const noexcept {
        *opened_ptr_ = r->id_;
    }
    void clear_opened() const noexcept {
        *opened_ptr_ = -1;
    }
};

void_end_