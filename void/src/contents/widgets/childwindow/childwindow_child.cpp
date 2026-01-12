#include "childwindow_child.h"
#include <void/void.h>
#include <contents/overlays/childwindow/childwindow.h>


void_begin_

childwindow_child::childwindow_child(void_* instance, input_owner* input_owner,
                                     input_owner_overlay* overlay_owner, std::int32_t overlay_id)
    : widget_child(instance,
        input_owner, 1,
        overlay_owner, overlay_id)
{
}

float childwindow_child::update(const r2::rectf& full_pos, float right_x, float y, float h, const render_input& input)
{
    (void)full_pos;

    const float width = h;
    last_pos_.x = right_x - width;
    last_pos_.y = y;
    last_pos_.w = width;
    last_pos_.h = h;

    const bool overlay_is_opened = parent_overlay_owner_->input_get_overlay_render_input().is_opened(
        parent_overlay_owner_->get_overlay(overlay_id_)
    );

    auto& util = instance()->util();
    animation_hovered_ = util.lerp(animation_hovered_, input.is_hovered(this));
    animation_selected_ = util.lerp(animation_selected_, 
        input.is_selected(this) ||
        overlay_is_opened
    );

    return last_pos_.w;
}

void childwindow_child::render(const r2::rectf& full_pos, float alpha, float animation_disabled)
{
    if (last_pos_.x + last_pos_.w < full_pos.x)
        return; // occluded to left

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto& util = instance()->util();

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<childwindow>(overlay_id_);

    // render
    const bool push_clip_rect = last_pos_.x < full_pos.x;
    if (push_clip_rect)
        renderer.modify_clip_rect_x(full_pos.x, full_pos.x + full_pos.w);

    const float border_size = style.border_size.get(instance()->scale());
    const float selected_animation = std::max(animation_selected_, owned_overlay.alpha());
    const float offset = border_size * selected_animation;

    if (icon_ == icons::kInvalidHandle) {
        icon_ = instance()->icons().get_or_create_handle(void_resources::child_png);
    }
    const auto* icon = instance()->icons().get_or_create(icon_, last_pos_.w);
    renderer.add_image(
        icon->tex,
        r2::vec2(last_pos_.x - offset, last_pos_.y - offset),
        r2::vec2(last_pos_.x + last_pos_.w + offset, last_pos_.y + last_pos_.h + offset),
        util.disable_color(style.icon().interp(
            style.accent(), animation_hovered_ * 0.6f).alpha(
                alpha * (0.7f + selected_animation * 0.3f)
            ), animation_disabled),
        icon->uv_min, icon->uv_max
    );

    if (push_clip_rect)
        renderer.pop_clip_rect();
}

input_response childwindow_child::input(const input_base& input)
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