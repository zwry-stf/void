#include "keybind_widget.h"
#include <void/void.h>

#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif // R2_PLATFORM_WINDOWS


void_begin_

keybind_widget::keybind_widget(void_* instance, input_owner* input_owner, 
						       const xstr& name, keybind_owner* bind)
	: widget(instance, input_owner, 1),
	  name_(name),
	  keybind_owner_(bind)
{
    assert(name.find('\n') == xstr::npos);
}

void keybind_widget::update(float x, float y, float w, const render_input& input, bool occluded)
{
	widget::update(x, y, w, input, occluded);

	constexpr sfloat kButtonHeight = sfloat(26.f, true);
	last_pos_.h = kButtonHeight.get(instance()->scale());

	auto& style = instance()->style();
    auto& renderer = instance()->renderer();

	const float spacing = style.spacing->get(instance()->scale());

	if (occluded)
		return;

	const float keybind_width = std::round(
		(std::min)((std::max)(w * 0.45f, last_pos_.h * 3.5f),
			last_pos_.w - spacing)
	);
	const float spacing_y = std::round(spacing * 0.25f);

	keybind_pos_.x = x + w - keybind_width;
	keybind_pos_.y = y + spacing_y;
	keybind_pos_.w = keybind_width;
	keybind_pos_.h = last_pos_.h - spacing_y * 2.f;

	update_child_widgets(keybind_pos_.x, input);

    bool recalculate_text = false;
    if (last_text_.empty() ||
        last_key_.has_key != keybind_owner_->has_key()) {
        recalculate_text = true;
    }
    else if (last_key_.has_key) {
        if (last_key_.storage.key != keybind_owner_->key()) {
            recalculate_text = true;
        }
    }
    else {
        if (last_key_.storage.mouse_button != keybind_owner_->mouse_button()) {
            recalculate_text = true;
        }
    }

    if (recalculate_text) {
        text_width_calculated_ = false;
        if (!keybind_owner_->has_key()) {
            switch (keybind_owner_->mouse_button()) {
            case mouse_button::left:     last_text_ = "LButton"; break;
            case mouse_button::right:    last_text_ = "RButton"; break;
            case mouse_button::middle:   last_text_ = "MButton"; break;
            case mouse_button::xbutton1: last_text_ = "XButton 1"; break;
            case mouse_button::xbutton2: last_text_ = "XButton 2"; break;
            }
        }

        else if (keybind_owner_->key() != key::none) {
            int scan_code = instance()->input().get_scan_code(keybind_owner_->key());
#if defined(R2_PLATFORM_WINDOWS)
            if (scan_code != 0) {
                char buffer[128];
                int str_len = GetKeyNameTextA(static_cast<LONG>(scan_code << 16), buffer, sizeof(buffer));
                if (str_len != 0) {
                    last_text_.assign(buffer, buffer + str_len);
                }
            }
#endif // R2_PLATFORM_WINDOWS
        }

        if (last_text_.empty())
            last_text_ = "None";
    }

    if (!text_width_calculated_) {
        instance()->fonts().bind_font_small();
        if (renderer.get_text_width_strict(last_text_, text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(last_text_);
        text_width_ = std::ceil(text_width_);
    }
}

void keybind_widget::render(float alpha)
{
    if (was_occluded_) {
        return;
    }

    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const float border_size = style.border_size.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float text_spacing = std::round(
        (last_pos_.h - text_size_small) * 0.5f
    );

    // text
    const float cliprect_left = static_cast<float>(
        renderer.cmd_header().clip_rect.left
    );
    renderer.modify_clip_rect_x(
        cliprect_left,
        child_widget_left_pos_ - border_size
    );

    renderer.add_text_faded(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        style.text().transparent(),
        style.text().alpha((alpha * (1.f - animation_disabled_)) *
            (0.3f + animation_selected_ * 0.4f)),
        cliprect_left, (std::max)(cliprect_left, last_pos_.x + border_size),
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

    // keybind
    const float rounding = style.rounding->get(instance()->scale());

    const r2::vec2 keybind_min = r2::vec2(keybind_pos_.x, keybind_pos_.y);
    const r2::vec2 keybind_max = keybind_min + r2::vec2(keybind_pos_.w, keybind_pos_.h);

    renderer.add_shadow_rect_filled(
        keybind_min, keybind_max,
        util.disable_color(style.accent().alpha(alpha * animation_selected_ * 0.5f), animation_disabled_),
        rounding * 0.5f,
        (keybind_max.y - keybind_min.y) * 0.7f
    );

    renderer.add_rect_filled(
        keybind_min, keybind_max,
        util.disable_color(style.accent2(), animation_disabled_).alpha(alpha),
        rounding * 0.5f
    );

    renderer.add_rect_inner(
        keybind_min, keybind_max,
        util.disable_color(style.accent(), animation_disabled_).alpha(alpha * (0.5f + animation_hovered_ * 0.5f)),
        border_size,
        rounding * 0.5f
    );

    // keybind text
    renderer.push_clip_rect(
        keybind_min, keybind_max,
        true
    );

    const float text_spacing_x = std::round((keybind_pos_.w - text_width_) * 0.5f);

    const float fade_width = style.spacing->get(instance()->scale()) * 2.f;
    const float fade_end = keybind_pos_.x + keybind_pos_.w - border_size;
    const float fade_start = fade_end - fade_width;

    renderer.add_rect_filled(
        r2::vec2(keybind_pos_.x + text_spacing_x - border_size,
            last_pos_.y + text_spacing + text_size_small - border_size),
        r2::vec2(keybind_pos_.x + text_spacing_x + text_width_ + border_size,
            last_pos_.y + text_spacing + text_size_small + border_size),
        text_color.alpha(animation_selected_)
    );

    renderer.add_text_faded(
        r2::vec2(keybind_pos_.x + text_spacing_x, last_pos_.y + text_spacing),
        util.disable_color(style.text(), animation_disabled_).alpha(alpha),
        util.disable_color(style.text(), animation_disabled_).transparent(),
        fade_start, fade_end,
        last_text_
    );

    renderer.pop_clip_rect();

    render_child_widgets(alpha);
}

input_response keybind_widget::input(const input_base& input)
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
            util::is_in_rect(mouse_x, mouse_y, keybind_pos_)) {
            input.set_hovered(this);

            instance()->cursors().set_cursor(cursor::text);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (input.is_selected(this)) {
            keybind_owner_->set_key(input.event().get_mouse_button());
            input.clear_selected();
        }
        else if (util::is_in_rect(mouse_x, mouse_y, keybind_pos_)) {
            input.set_selected(this);

            return input_response::handled();
        }
    }
    
    else if (input.is_selected(this) &&
             input.event().is_message(message_type::key_down)) {
        if (input.event().get_key() == key::backspace)
            keybind_owner_->set_key(key::none);

        else if (input.event().get_key() != key::escape)
            keybind_owner_->set_key(input.event().get_key());

        input.clear_selected();

        return input_response::handled();
    }

    else if (input.is_selected(this) &&
             input.event().is_message(message_type::mouse_button_down)) {
        keybind_owner_->set_key(input.event().get_mouse_button());

        input.clear_selected();

        return input_response::handled();
    }

    return input_response::empty();
}

void keybind_widget::on_scale_change()
{
	widget::on_scale_change();

	text_width_calculated_ = false;
}

bool keybind_widget::matches_search(const xstr& search) noexcept
{
	return name_.find_ignore_case_f(search) != xstr::npos;
}


void_end_