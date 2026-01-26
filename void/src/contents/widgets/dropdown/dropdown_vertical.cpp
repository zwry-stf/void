#include "dropdown_vertical.h"
#include <void/void.h>
#include <contents/overlays/dropdown/dropdown.h>
#include "dropdown.h"


void_begin_

dropdown_vertical::dropdown_vertical(void_* instance, input_owner* input_owner,
                                     input_owner_overlay* overlay_owner, std::int32_t overlay_id, const xstr& name)
    : widget(instance, input_owner, 1),
      parent_overlay_owner_(overlay_owner),
      overlay_id_(overlay_id),
      name_(name)
{
    assert(name.find('\n') == xstr::npos);
}

void dropdown_vertical::update(float x, float y, float w, const render_input& input, bool occluded)
{
    widget::update(x, y, w, input, occluded);

    auto& style = instance()->style();

    const float spacing = style.spacing->get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    constexpr sfloat kDropdownHeight = sfloat(26.f, true);
    last_pos_.h = kDropdownHeight.get(instance()->scale()) + text_size_small;

    if (occluded)
        return;

    const float spacing_y = std::round(spacing * 0.25f);

    dropdown_pos_.x = x;
    dropdown_pos_.y = y + text_size_small + spacing_y;
    dropdown_pos_.w = w;
    dropdown_pos_.h = last_pos_.h - spacing_y * 2.f - text_size_small;
}

void dropdown_vertical::render(float alpha)
{
    if (was_occluded_) {
        return;
    }

    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const float border_size = style.border_size.get(instance()->scale());

    const float text_spacing = 0.f;

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<dropdown_overlay>(overlay_id_);

    // text
    const float cliprect_left = static_cast<float>(
        renderer.cmd_header().clip_rect.left
    );
    renderer.modify_clip_rect_x(
        cliprect_left,
        last_pos_.x + last_pos_.w
    );

    renderer.add_text_faded(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        style.text().transparent(),
        style.text().alpha((alpha * (1.f - animation_disabled_)) *
            (0.3f + animation_selected_ * 0.4f)),
        cliprect_left, std::max(cliprect_left, last_pos_.x + border_size),
        name_,
        true /* blurred */
    );
    
    renderer.add_text(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        util.disable_color(
            style.text(), animation_disabled_).alpha(alpha),
        name_
    );

    renderer.pop_clip_rect();

    dropdown::render_dropdown(
        instance(),
        dropdown_pos_,
        animation_hovered_,
        animation_selected_,
        animation_disabled_,
        alpha,
        owned_overlay.options()->element_safe(*owned_overlay.selected()),
        false /* is multiselect */
    );
}

input_response dropdown_vertical::input(const input_base& input)
{
    if (!can_receive_input_simple(input))
        return input_response::empty();

    if (!can_receive_input_this(input))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_move)) {
        if (input.is_selected(this) ||
            util::is_in_rect(mouse_x, mouse_y, dropdown_pos_)) {
            input.set_hovered(this);

            instance()->cursors().set_cursor(cursor::hand);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, dropdown_pos_)) {
            input.set_selected(this);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left &&
             input.is_selected(this)) {
        if (util::is_in_rect(mouse_x, mouse_y, dropdown_pos_)) {
            parent_overlay_owner_->get_overlay(overlay_id_)->set_pos(
                r2::vec2(dropdown_pos_.x, dropdown_pos_.y + dropdown_pos_.h),
                r2::vec2(dropdown_pos_.w, dropdown_pos_.h)
            );

            parent_overlay_owner_->set_opened(overlay_id_);
        }

        input.clear_selected();

        return input_response::handled();
    }

    return input_response::empty();
}

void dropdown_vertical::set_pos(const r2::vec2& pos)
{
    const float delta_x = pos.x - last_pos_.x;
    const float delta_y = pos.y - last_pos_.y;

    widget::set_pos(pos);

    dropdown_pos_.x += delta_x;
    dropdown_pos_.y += delta_y;
}

bool dropdown_vertical::matches_search(const xstr& search) noexcept
{
    return name_.find_ignore_case_f(search) != xstr::npos;
}

void_end_