#include "optional_colorpicker_child.h"
#include <void/void.h>
#include <contents/overlays/colorpicker/colorpicker.h>


void_begin_

optional_colorpicker_child::optional_colorpicker_child(void_* instance, input_owner* input_owner, 
                                                       input_owner_overlay* overlay_owner, std::int32_t overlay_id,
                                                       bool* value)
    : childwindow_child(instance, input_owner, overlay_owner, overlay_id),
      value_enabled_(value)
{
}

void optional_colorpicker_child::render(const r2::rectf& full_pos, float alpha, float animation_disabled)
{
    if (last_pos_.x + last_pos_.w < full_pos.x)
        return; // occluded to left

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto& util = instance()->util();

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<colorpicker_overlay>(overlay_id_);

    // render
    const bool push_clip_rect = last_pos_.x < full_pos.x;
    if (push_clip_rect)
        renderer.modify_clip_rect_x(full_pos.x, full_pos.x + full_pos.w);

    const float border_size = style.border_size.get(instance()->scale());

    if (*value_enabled_) {
        const float selected_animation = std::max(animation_selected_, owned_overlay.alpha());
        const float offset = border_size * selected_animation;

        if (icon_ == icons::kInvalidHandle) {
            icon_ = instance()->icons().get_or_create_handle(void_resources::color_png);
        }
        const auto* icon = instance()->icons().get_or_create(icon_, last_pos_.w);
        renderer.add_image(
            icon->tex,
            r2::vec2(last_pos_.x - offset, last_pos_.y - offset),
            r2::vec2(last_pos_.x + last_pos_.w + offset, last_pos_.y + last_pos_.h + offset),
            util.disable_color(owned_overlay.get_color()->opague().interp(
                style.accent(), animation_hovered_ * 0.3f).alpha(
                    alpha * (0.7f + selected_animation * 0.3f)), animation_disabled),
            icon->uv_min, icon->uv_max
        );
    }
    else {
        const auto pos_mid = r2::vec2{
            std::round(last_pos_.x + last_pos_.w * 0.5f),
            std::round(last_pos_.y + last_pos_.h * 0.5f)
        };
        const auto icon_color = style.icon().interp(
            style.accent(), animation_hovered_ * 0.6f).alpha(
                alpha * 0.7f * (0.5f + animation_hovered_ * 0.5f)
        );

        const auto icon_color_a = icon_color.alpha(0.8f - animation_hovered_ * 0.5f);

        // center
        renderer.prim_rect(
            pos_mid - r2::vec2(border_size, border_size),
            pos_mid + r2::vec2(border_size, border_size),
            icon_color
        );

        // up
        renderer.add_rect_filled_multicolor(
            r2::vec2(pos_mid.x - border_size, last_pos_.y + border_size),
            r2::vec2(pos_mid.x + border_size, pos_mid.y - border_size),
            icon_color_a, icon_color_a, icon_color, icon_color
        );

        // down
        renderer.add_rect_filled_multicolor(
            r2::vec2(pos_mid.x - border_size, pos_mid.y + border_size),
            r2::vec2(pos_mid.x + border_size, last_pos_.y + last_pos_.h - border_size),
            icon_color, icon_color, icon_color_a, icon_color_a
        );

        // left
        renderer.add_rect_filled_multicolor(
            r2::vec2(last_pos_.x + border_size, pos_mid.y - border_size),
            r2::vec2(pos_mid.x - border_size, pos_mid.y + border_size),
            icon_color_a, icon_color, icon_color, icon_color_a
        );

        // right
        renderer.add_rect_filled_multicolor(
            r2::vec2(pos_mid.x + border_size, pos_mid.y - border_size),
            r2::vec2(last_pos_.x + last_pos_.w - border_size, pos_mid.y + border_size),
            icon_color, icon_color_a, icon_color_a, icon_color
        );
    }

    if (push_clip_rect)
        renderer.pop_clip_rect();
}

input_response optional_colorpicker_child::input(const input_base& input)
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
            if (*value_enabled_) {
                if (instance()->input().is_control_down())
                    *value_enabled_ = false;
                else
                    input.set_selected(this);
            }
            else {
                *value_enabled_ = true;
            }

            return input_response::handled();
        }
    }
    
    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::right) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            *value_enabled_ = false;

            return input_response::handled();
        }
    }

    else if (input.is_selected(this) &&
             input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            // open window
            parent_overlay_owner_->get_overlay(overlay_id_)->set_pos(
                r2::vec2{
                    last_pos_.x + last_pos_.w * 0.5f,
                    last_pos_.y + last_pos_.h * 0.5f
                }
            );

            parent_overlay_owner_->set_opened(overlay_id_);
        }

        input.clear_selected();

        return input_response::handled();
    }

    return input_response::empty();
}

void_end_