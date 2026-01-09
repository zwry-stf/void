#include "colorpicker.h"
#include <void/void.h>
#include <contents/overlays/colorpicker/colorpicker.h>



void_begin_

colorpicker::colorpicker(void_* instance, input_owner* input_owner, 
	                     input_owner_overlay* overlay_owner, std::int32_t overlay_id, const xstr& name)
	: widget(instance, input_owner, 1),
	  parent_overlay_owner_(overlay_owner),
	  owned_overlay_id_(overlay_id),
	  name_(name)
{
	assert(name.find('\n') == xstr::npos);

	auto& owning_overlay = *parent_overlay_owner_->get_overlay<colorpicker_overlay>(owned_overlay_id_);

	text_field_ = std::make_unique<textfield>(
		instance, input_owner,
		textfield_type::color,
		textfield_flags::stop_on_return,
		xstr(),
		xstr(),
		owning_overlay.has_alpha() ? 8u : 6u
	);
	text_field_->set_stop_callback(
		[this](const std::u32string& s) -> void {
			this->on_stop_typing(s);
		}
	);
}

void colorpicker::update(float x, float y, float w, const render_input& input, bool occluded)
{
	widget::update(x, y, w, input, occluded);

	constexpr sfloat kColorpickerHeight = sfloat(26.f, true);
	last_pos_.h = kColorpickerHeight.get(instance()->scale());

	auto& renderer = instance()->renderer();
	auto& style = instance()->style();

	instance()->fonts().bind_font_small();

	auto& owning_overlay = *parent_overlay_owner_->get_overlay<colorpicker_overlay>(owned_overlay_id_);

	if (!text_width_calculated_) {
		if (renderer.get_text_width_strict(name_, text_width_))
			text_width_calculated_ = true;
		else
			text_width_ = renderer.get_text_width(name_);
		text_width_ = std::ceil(text_width_);
	}

	const float text_size_small = style.text_size_small.get(instance()->scale());
	const float border_size = style.border_size.get(instance()->scale());
	const float text_spacing = std::round((last_pos_.h - text_size_small) * 0.5f);
	const float color_offset_y = std::round(last_pos_.h * 0.15f);

	color_pos_.h = last_pos_.h - color_offset_y * 2.f;
	color_pos_.w = std::round(last_pos_.h * 1.4f);

	color_pos_.x = x + w - color_pos_.w;
	color_pos_.y = y + color_offset_y;

	// text field
	if (!input.is_selected(text_field_.get())) {
		text_field_->set_string(owning_overlay.get_color_string());
	}

	text_field_->update(
		r2::rectf{
			x + text_width_ +
				std::round(style.spacing->get(instance()->scale()) * 0.5f),
			y + text_spacing,
			std::min(w - text_width_, color_pos_.x - x - text_width_ - border_size),
			text_size_small
		},
		input,
		occluded
	);

	// child widgets
	update_child_widgets(color_pos_.x, input);
}

void colorpicker::render(float alpha)
{
	if (was_occluded_) {
		return;
	}

	instance()->fonts().bind_font_small();

	auto& renderer = instance()->renderer();
	auto& style = instance()->style();
	auto& util = instance()->util();

	auto& owning_overlay = *parent_overlay_owner_->get_overlay<colorpicker_overlay>(owned_overlay_id_);

	const float rounding = style.rounding->get(instance()->scale());
	const float border_size = style.border_size.get(instance()->scale());
	const float text_size_small = style.text_size_small.get(instance()->scale());

	// color
	const r2::vec2 color_min = r2::vec2(color_pos_.x, color_pos_.y);
	const r2::vec2 color_max = color_min + r2::vec2(color_pos_.w, color_pos_.h);

	renderer.add_shadow_rect_filled(
		color_min, color_max,
		util.disable_color(style.accent().alpha(animation_selected_ * 0.5f), animation_disabled_),
		rounding * 0.5f,
		(color_max.y - color_min.y) * 0.7f
	);

	renderer.add_rect_filled(
		color_min, color_max,
		util.disable_color(owning_overlay.get_color()->opague().alpha(alpha), animation_disabled_),
		rounding * 0.5f
	);

	renderer.add_rect_inner(
		color_min, color_max,
		util.disable_color(style.accent().alpha(alpha), animation_disabled_),
		border_size,
		rounding * 0.5f
	);

	// Text
	const float cliprect_left = static_cast<float>(renderer.cmd_header().clip_rect.left);
	const float text_spacing = std::round((last_pos_.h - text_size_small) * 0.5f);
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
		cliprect_left, (std::max)(cliprect_left, last_pos_.x + border_size),
		name_, true
	);

	renderer.add_text(
		r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
		util.disable_color(style.text().alpha(alpha), animation_disabled_),
		name_
	);

	const r2::color text_color = util.disable_color(
		style.accent().interp(style.text_accent(),
		text_field_->animation_selected()), animation_disabled_
	);

	text_field_->render(
		alpha,
		text_color
	);

	renderer.pop_clip_rect();

	// child widgets
	render_child_widgets(alpha);
}

input_response colorpicker::input(const input_base& input)
{
	if (!can_receive_input_simple(input))
		return input_response::empty();

	auto res = input_child_widgets(input);
	if (res.is_handled())
		return res;

	res = text_field_->input(input);
	if (res.is_handled())
		return res;

	if (!can_receive_input_this(input))
		return res;

	float mouse_x, mouse_y;
	input.event().get_cursor_pos(mouse_x, mouse_y);

	// Color
	if (input.event().is_message(message_type::mouse_button_down) &&
		input.event().get_mouse_button() == mouse_button::left) {
		if (util::is_in_rect(mouse_x, mouse_y, color_pos_)) {
			input.set_selected(this);

			return input_response::handled();
		}
	}

	else if (input.event().is_message(message_type::mouse_move)) {
		if (input.is_selected(this) ||
			util::is_in_rect(mouse_x, mouse_y, color_pos_)) {
			input.set_hovered(this);
			instance()->cursors().set_cursor(cursor::hand);

			return input_response::handled();
		}
	}

	else if (input.is_selected(this) &&
		     input.event().is_message(message_type::mouse_button_up) &&
		     input.event().get_mouse_button() == mouse_button::left) {
		if (util::is_in_rect(mouse_x, mouse_y, color_pos_)) {
			parent_overlay_owner_->get_overlay(owned_overlay_id_)->set_pos(
				r2::vec2{
					color_pos_.x + color_pos_.w * 0.5f,
					color_pos_.y + color_pos_.h * 0.5f
				}
			);

			parent_overlay_owner_->set_opened(owned_overlay_id_);
		}

		input.clear_selected();

		return input_response::handled();
	}

	return input_response::empty();
}

void colorpicker::on_activate()
{
	widget::on_activate();
	text_field_->on_activate();
}

void colorpicker::on_scale_change()
{
	text_width_calculated_ = false;
	text_field_->on_scale_changed();
}

bool colorpicker::matches_search(const xstr& search) noexcept
{
	return name_.find_ignore_case_f(search) != xstr::npos;
}

void colorpicker::on_stop_typing(const std::u32string& s)
{
	parent_overlay_owner_->get_overlay<colorpicker_overlay>(owned_overlay_id_)->on_stop_typing(s);
}

void_end_