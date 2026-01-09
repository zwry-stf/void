#include "button.h"
#include <void/void.h>


void_begin_

button::button(void_* instance, input_owner* input_owner,
	           const xstr& name, const xstr& button_text, std::function<void()>&& callback)
	: widget(instance, input_owner, 1),
	  name_(name),
	  button_text_(button_text),
	  callback_(std::move(callback))
{
	assert(name.find('\n') == xstr::npos);
	assert(button_text.find('\n') == xstr::npos);
	assert((bool)callback_);
}

void button::update(float x, float y, float w, const render_input& input, bool occluded)
{
	widget::update(x, y, w, input, occluded);

	constexpr sfloat kButtonHeight = sfloat(26.f, true);
	last_pos_.h = kButtonHeight.get(instance()->scale());

	auto& style = instance()->style();
    auto& renderer = instance()->renderer();

	const float spacing = style.spacing->get(instance()->scale());

	if (occluded)
		return;

	const float button_width = std::round(
		std::min(std::max(w * 0.45f, last_pos_.h * 3.5f),
			last_pos_.w - spacing)
	);
	const float spacing_y = std::round(spacing * 0.25f);

	button_pos_.x = x + w - button_width;
	button_pos_.y = y + spacing_y;
	button_pos_.w = button_width;
	button_pos_.h = last_pos_.h - spacing_y * 2.f;

	update_child_widgets(button_pos_.x, input);

    if (!text_width_calculated_) {
        instance()->fonts().bind_font_small();
        if (renderer.get_text_width_strict(button_text_, text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(button_text_);
        text_width_ = std::ceil(text_width_);
    }
}

void button::render(float alpha)
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
    
    const auto text_color = util.disable_color(
        style.text(), animation_disabled_).alpha(alpha);

    renderer.add_text(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        text_color,
        name_
    );

    renderer.pop_clip_rect();

    // button
    const float rounding = style.rounding->get(instance()->scale());

    const r2::vec2 button_min = r2::vec2(button_pos_.x, button_pos_.y);
    const r2::vec2 button_max = button_min + r2::vec2(button_pos_.w, button_pos_.h);

    renderer.add_shadow_rect_filled(
        button_min, button_max,
        util.disable_color(style.accent().alpha(animation_selected_ * 0.5f), animation_disabled_),
        rounding * 0.5f,
        (button_max.y - button_min.y) * 0.7f
    );

    renderer.add_rect_filled(
        button_min, button_max,
        util.disable_color(style.accent2(), animation_disabled_).alpha(alpha),
        rounding * 0.5f
    );

    renderer.add_rect_inner(
        button_min, button_max,
        util.disable_color(style.accent(), animation_disabled_).alpha(alpha * (0.5f + animation_hovered_ * 0.5f)),
        border_size,
        rounding * 0.5f
    );

    // button text
    renderer.push_clip_rect(
        button_min, button_max,
        true
    );

    const float text_spacing_x = std::round((button_pos_.w - text_width_) * 0.5f);

    renderer.add_text(
        r2::vec2(button_pos_.x + text_spacing_x, last_pos_.y + text_spacing),
        util.disable_color(style.text(), animation_disabled_).alpha(alpha),
        button_text_
    );

    renderer.pop_clip_rect();
}

input_response button::input(const input_base& input)
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
            util::is_in_rect(mouse_x, mouse_y, button_pos_)) {
            input.set_hovered(this);

            instance()->cursors().set_cursor(cursor::hand);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, button_pos_)) {
            input.set_selected(this);

            return input_response::handled();
        }
    }

    else if (input.is_selected(this) &&
             input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, button_pos_)) {
            callback_();
        }

        input.clear_selected();

        return input_response::handled();
    }

    return input_response::empty();
}

void button::on_scale_change()
{
	widget::on_scale_change();

	text_width_calculated_ = false;
}

bool button::matches_search(const xstr& search) noexcept
{
	return name_.find_ignore_case_f(search);
}

void_end_