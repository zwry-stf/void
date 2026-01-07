#pragma once
#include <void/def.h>
#include <cstdint>


void_begin_

class input_receiver {
private:
    class input_owner* const input_owner_;
    std::int64_t id_min_;
    std::int64_t id_max_;

    friend class input_base;
    friend class render_input;

public:
    input_receiver(class input_owner* owner, std::int32_t num) noexcept;

    void resize_input(std::int32_t num) noexcept;
    [[nodiscard]] std::int64_t input_count() const noexcept {
        return id_max_ - id_min_;
    }
};

class input_receiver_overlay {
private:
    const std::int32_t id_;

    friend class overlay_input;
    friend class overlay_render_input;

public:
    input_receiver_overlay(class input_owner_overlay* owner) noexcept;
};

void_end_