#pragma once
#include <void/util/vobj.h>
#include <void/contents/input/events.h>
#include <void/contents/input/input_response.h>
#include <r2/renderer_definitions.h>


void_begin_

class input_owner_overlay;

class widget_child : protected vobj,
                     public input_receiver {
protected:
    const std::int32_t overlay_id_;

    r2::rectf last_pos_;
    input_owner_overlay* const parent_overlay_owner_;
    float animation_hovered_{ 0.f };
    float animation_selected_{ 0.f };

public:
    widget_child(void_* instance, input_owner* input_owner, std::int32_t input_num,
                 input_owner_overlay* overlay_owner, std::int32_t overlay_id);

    widget_child(const widget_child&) = delete;
    widget_child& operator=(const widget_child&) = delete;

public:
    virtual float update(const r2::rectf& full_pos, float right_x, float y, float h,
                         const render_input& input);
    virtual void render(const r2::rectf& full_pos, float alpha, float animation_disabled) = 0;
    virtual input_response input(const input_base& input) = 0;

    virtual void on_activate();

public:
    [[nodiscard]] auto overlay_id() const noexcept {
        return overlay_id_;
    }
};

void_end_