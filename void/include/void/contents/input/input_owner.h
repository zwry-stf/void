#pragma once
#include "events.h"


void_begin_

class input_owner {
protected:
    inline static constexpr std::int64_t kInvalidState = -1;

    std::int64_t hovered_{ kInvalidState };
    std::int64_t selected_{ kInvalidState };
    std::int64_t id_counter_{ 0 };

public:
    input_owner() = default;

public:
    [[nodiscard]] render_input input_get_render_input() const noexcept {
        render_input ret(hovered_, selected_);
        return ret; 
    }

    [[nodiscard]] input_base input_get_input(const message_event& event) noexcept {
        input_base ret(hovered_, selected_);
        ret.set_event(event); 
        return ret; 
    }

    void input_reset_hovered_state() noexcept { 
        hovered_ = kInvalidState;
    }
    void input_reset_selected_state() noexcept { 
        selected_ = kInvalidState;
    }
    [[nodiscard]] bool input_nothing_selected() const noexcept {
        return selected_ == kInvalidState;
    }
    [[nodiscard]] std::int64_t create_ids(std::int32_t num) noexcept {
        const auto ret = id_counter_;
        id_counter_ += num;
        return ret;
    }
    [[nodiscard]] std::int64_t resize(std::int64_t old_min, std::int64_t old_max, std::int32_t num) noexcept {
        if (old_max == id_counter_) {
            id_counter_ = old_min + num;
            return old_min;
        }
        else {
            return create_ids(num);
        }
    }
};

void_end_