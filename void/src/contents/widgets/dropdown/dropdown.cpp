#include "dropdown.h"
#include <void/void.h>
#include <contents/overlays/dropdown/dropdown.h>


void_begin_

dropdown::dropdown(void_* instance, input_owner* input_owner, 
	               input_owner_overlay* overlay_owner, std::int32_t overlay_id, const xstr& name)
	: widget(instance, input_owner, 1),
	  parent_overlay_owner_(overlay_owner),
	  overlay_id_(overlay_id),
	  name_(name)
{
	assert(name.find('\n') == xstr::npos);
}

void dropdown::update(float x, float y, float w, const render_input& input, bool occluded)
{
	widget::update(x, y, w, input, occluded);

	constexpr sfloat kDropdownHeight = sfloat(26.f, true);
	last_pos_.h = kDropdownHeight.get(instance()->scale());

	auto& style = instance()->style();

	const float spacing = style.spacing->get(instance()->scale());

	if (occluded)
		return;

    const float dropdown_width = std::round(
        std::min(std::max(w * 0.45f, last_pos_.h * 3.5f),
            last_pos_.w - spacing)
    );
	const float spacing_y = std::round(spacing * 0.25f);

	dropdown_pos_.x = x + w - dropdown_width;
	dropdown_pos_.y = y + spacing_y;
	dropdown_pos_.w = dropdown_width;
	dropdown_pos_.h = last_pos_.h - spacing_y * 2.f;

	update_child_widgets(dropdown_pos_.x, input);
}

void dropdown::render(float alpha)
{
    if (was_occluded_) {
        return;
    }

    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const float border_size = style.border_size.get(instance()->scale());

    const float text_spacing = std::round(
        (last_pos_.h - style.text_size_small.get(instance()->scale())) * 0.5f
    );

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<dropdown_overlay>(overlay_id_);

    // text
    const float cliprect_left = static_cast<float>(
        renderer.cmd_header().clip_rect.left
    );
    renderer.push_clip_rect(
        r2::vec2(cliprect_left, last_pos_.y),
        r2::vec2(child_widget_left_pos_ - border_size, last_pos_.y + last_pos_.h),
        true
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

    render_dropdown(
        instance(),
        dropdown_pos_,
        animation_hovered_,
        animation_selected_,
        animation_disabled_,
        alpha,
        owned_overlay.options()->element_safe(*owned_overlay.selected()),
        false /* is multiselect */
    );

    render_child_widgets(alpha);
}

void dropdown::render_dropdown(void_* instance, const r2::rectf& pos, float animation_hovered, float animation_selected,
                               float animation_disabled, float alpha, const xstr& text, bool is_multiselect)
{
    (void)is_multiselect;

    auto& util = instance->util();
    auto& style = instance->style();
    auto& renderer = instance->renderer();

    const float spacing = style.spacing->get(instance->scale());
    const float rounding = style.rounding->get(instance->scale());
    const float border_size = style.border_size.get(instance->scale());

    // background
    const r2::vec2 dropdown_min = r2::vec2(pos.x, pos.y);
    const r2::vec2 dropdown_max = dropdown_min + r2::vec2(pos.w, pos.h);

    renderer.add_shadow_rect_filled(
        dropdown_min, dropdown_max,
        util.disable_color(style.accent().alpha(animation_selected * 0.5f), animation_disabled),
        rounding,
        pos.h * 0.7f /* shadow size */
    );

    renderer.add_rect_filled(
        dropdown_min, dropdown_max,
        util.disable_color(style.accent2(), animation_disabled).alpha(alpha),
        rounding * 0.5f
    );

    // border
    renderer.add_rect_inner(
        dropdown_min, dropdown_max,
        util.disable_color(style.accent(), animation_disabled).alpha(
            alpha * (0.5f + animation_hovered * 0.5f)),
        border_size,
        rounding * 0.5f
    );

    const float icon_spacing = std::round(pos.h * 0.15f);
    const float icon_size = pos.h - icon_spacing * 2.f;

    const r2::vec2 icon_min = r2::vec2(pos.x + pos.w - icon_spacing - icon_size,
        pos.y + icon_spacing);
    const r2::vec2 icon_max = icon_min + r2::vec2(icon_size, icon_size);

    const auto icon_color = util.disable_color(style.accent().interp(
        style.text_accent(), 0.5f + animation_hovered * 0.5f),
        animation_disabled).alpha(alpha);

    // dropdown icon
    if (is_multiselect) {
        const float icon_line_width = border_size * 2.f;
        const float line_spacing = icon_line_width;
        const float space_per_line = icon_line_width + line_spacing;

        const float full_icon_height = icon_size;
        const int num_in_height = static_cast<int>(
            std::floor(
                (full_icon_height + line_spacing) / space_per_line
            )
        );
        const float remaining_space = full_icon_height -
            (static_cast<float>(num_in_height) * space_per_line - line_spacing);

        const float f = std::round(remaining_space * 0.5f);

        const float side_spacing = std::round(spacing * 0.1f);

        float pos_y = pos.y + icon_spacing + f;
        for (int i = 0; i < num_in_height; i++) {
            renderer.prim_rect(
                r2::vec2(icon_min.x + side_spacing, pos_y),
                r2::vec2(icon_max.x - side_spacing, pos_y + icon_line_width),
                icon_color
            );
            pos_y += space_per_line;
        }
    }
    else {
        const float icon_offset = std::round(icon_size * 0.25f);

        const r2::vec2 left = r2::vec2{
                icon_min.x,
                icon_min.y + icon_size * 0.5f - icon_offset
        };
        const r2::vec2 mid = r2::vec2{
                icon_min.x + icon_size * 0.5f,
                icon_min.y + icon_size * 0.5f + icon_offset
        };
        const r2::vec2 right = r2::vec2{
                icon_max.x,
                icon_min.y + icon_size * 0.5f - icon_offset
        };
        const float line_width = std::round(border_size * 1.45f);

        const r2::vec2 right_offset = (right - mid).normalize().perp() * r2::vec2(line_width);
        const r2::vec2 left_offset = (mid - left).normalize().perp() * r2::vec2(line_width);
        const r2::vec2 bottom_right = right + right_offset;
        const r2::vec2 bottom_left = left + left_offset;

        const r2::vec2 d1 = (left - mid);
        const r2::vec2 d2 = (right - mid);

        const float denom = d1.cross(d2);

        r2::vec2 bottom_mid;
        if (std::abs(denom) < 1e-6f) {
            bottom_mid = mid + (left_offset + right_offset) * r2::vec2(0.5f);
        }
        else {
            const r2::vec2 A = mid + left_offset;
            const r2::vec2 B = mid + right_offset;

            const float t = (B - A).cross(d2) / denom;
            bottom_mid = A + d1 * r2::vec2(t);
        }

        renderer.add_quad_filled(
            mid,
            bottom_mid,
            bottom_right,
            right,
            icon_color
        );

        renderer.add_quad_filled(
            bottom_mid,
            mid,
            left,
            bottom_left,
            icon_color
        );
    }

    // text
    renderer.push_clip_rect(
        dropdown_min, r2::vec2(icon_min.x - border_size, dropdown_max.y),
        true
    );

    const float spacing_x = std::round(spacing * 0.375f);

    const float fade_width = spacing * 2.f;
    const float fade_end = icon_min.x - border_size * 2.f;
    const float fade_start = fade_end - fade_width;

    const auto text_color = util.disable_color(
        style.text(), animation_disabled).alpha(alpha);

    const float text_spacing = std::round(
        (pos.h - style.text_size_small.get(instance->scale())) * 0.5f
    );

    renderer.add_text_faded(
        r2::vec2(pos.x + spacing_x, pos.y + text_spacing),
        text_color, text_color.transparent(),
        fade_start, fade_end,
        text
    );

    renderer.pop_clip_rect();
}


input_response dropdown::input(const input_base& input)
{
    if (!can_receive_input_simple(input))
        return input_response::empty();

    auto res = input_child_widgets(input);
    if (res.is_handled())
        return res;

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

bool dropdown::matches_search(const xstr& search) noexcept
{
	return name_.find_ignore_case_f(search) != xstr::npos;
}

void_end_