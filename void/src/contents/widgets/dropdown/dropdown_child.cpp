#include "dropdown_child.h"
#include <void/void.h>
#include <contents/overlays/dropdown/dropdown.h>
#include <contents/widgets/dropdown/dropdown.h>


void_begin_

dropdown_child::dropdown_child(void_* instance, input_owner* input_owner, 
	                           input_owner_overlay* overlay_owner, std::int32_t overlay_id)
	: widget_child(instance,
		input_owner, 1,
		overlay_owner, overlay_id)
{
}

float dropdown_child::update(const r2::rectf& full_pos, float right_x, float y, float h, const render_input& input)
{
    widget_child::update(full_pos, right_x, y, h, input);

    auto& style = instance()->style();

    const float spacing = style.spacing->get(instance()->scale());

    const float dropdown_width = std::max(std::min(
        std::round(full_pos.h * 3.5f), right_x - full_pos.x),
        full_pos.h
    );
    const float spacing_y = std::round(spacing * 0.25f);

    last_pos_.x = right_x - dropdown_width;
    last_pos_.y = full_pos.y + spacing_y;
    last_pos_.w = dropdown_width;
    last_pos_.h = full_pos.h - spacing_y * 2.f;

    return last_pos_.w;
}

void dropdown_child::render(const r2::rectf& full_pos, float alpha, float animation_disabled)
{
    auto& renderer = instance()->renderer();

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<dropdown_overlay>(overlay_id_);

    const float shadow_size = 0.7f;
    bool push_clip_rect = last_pos_.x - shadow_size < full_pos.x;
    if (push_clip_rect) {
        renderer.push_clip_rect(
            r2::vec2(full_pos.x, full_pos.y),
            r2::vec2(full_pos.x + full_pos.w, full_pos.y + full_pos.h),
            true
        );
    }

    if (*owned_overlay.selected() >= owned_overlay.options()->size())
        *owned_overlay.selected() = 0;

    dropdown::render_dropdown(
        instance(),
        last_pos_,
        animation_hovered_,
        animation_selected_,
        animation_disabled,
        alpha,
        owned_overlay.options()->element_safe(*owned_overlay.selected()),
        false /* is multiselect */
    );

    if (push_clip_rect)
        renderer.pop_clip_rect();
}

input_response dropdown_child::input(const input_base& input)
{
    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_move)) {
        if (input.is_selected(this) ||
            (util::is_in_rect(mouse_x, mouse_y, last_pos_) && input.nothing_selected())) {
            input.set_hovered(this);

            instance()->cursors().set_cursor(cursor::hand);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            input.set_selected(this);

            return input_response::handled();
        }
    }

    else if (input.is_selected(this) &&
             input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            // open window
            parent_overlay_owner_->get_overlay(overlay_id_)->set_pos(
                r2::vec2(last_pos_.x, last_pos_.y + last_pos_.h),
                r2::vec2(last_pos_.w, last_pos_.h)
            );

            parent_overlay_owner_->set_opened(overlay_id_);
        }

        input.clear_selected();

        return input_response::handled();
    }

    return input_response::empty();
}

void_end_