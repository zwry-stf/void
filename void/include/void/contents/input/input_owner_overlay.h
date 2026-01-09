#pragma once
#include "overlay_events.h"
#include <vector>
#include <memory>
#include <void/contents/overlays/overlay.h>
#include <assert.h>


void_begin_

class overlay;
class input_owner_overlay {
private:
    std::int32_t opened_{ -1 };
    std::int32_t counter_{ 0 }; // input counter

protected:
    std::vector<std::unique_ptr<overlay>> overlays_;

public:
    input_owner_overlay() = default;

public:
    [[nodiscard]] overlay_input input_get_overlay_input(const message_event& event) noexcept {
        overlay_input ret(opened_);
        ret.set_event(&event);
        return ret;
    }
    [[nodiscard]] overlay_render_input input_get_overlay_render_input() const noexcept {
        return overlay_render_input(opened_);
    }

    [[nodiscard]] std::int32_t create_id() noexcept {
        return counter_++;
    }
    template <typename T = overlay>
    requires(std::is_base_of_v<overlay, T>)
    [[nodiscard]] T* get_overlay(std::int32_t id) const noexcept {
        return reinterpret_cast<T*>(overlays_[id].get());
    }
    [[nodiscard]] std::int32_t create_overlay(std::unique_ptr<overlay>&& overlay) {
        overlays_.push_back(std::move(overlay));

        return last_overlay_id();
    }
    [[nodiscard]] std::int32_t last_overlay_id() const noexcept {
        assert(overlays_.size() > 0u);
        return static_cast<std::int32_t>(overlays_.size()) - 1;
    }

    void set_opened(std::int32_t id) noexcept {
        opened_ = id;
    }

protected:
    [[nodiscard]] std::int32_t input_overlay_opened_id() const noexcept {
        return opened_;
    }
};

void_end_