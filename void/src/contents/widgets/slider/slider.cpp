#include "slider.h"
#include <void/void.h>
#include <format>


void_begin_

slider::slider(void_* instance, input_owner* input_owner,
               const xstr& name, float* value, float min, float max, const std::format_string<float>& format)
    : widget(instance, input_owner, 1),
      name_(name),
      value_(value),
      min_(min), max_(max),
      format_(format)
{
    assert(value != nullptr);
    assert(min < max);
    assert(name.find('\n') == xstr::npos);
}

void slider::update(float x, float y, float w, const render_input& input, bool occluded)
{
    widget::update(x, y, w, input, occluded);

    auto& util = instance()->util();

    constexpr sfloat kSliderHeight = sfloat(32.f, true);
    last_pos_.h = kSliderHeight.get(instance()->scale());

    const float target_animation = std::clamp((*value_ - min_) / (max_ - min_), 0.f, 1.f);
    animation_ = util.lerp(animation_, target_animation, 1.5f);

    if (occluded)
        return;
}

void slider::render(float alpha)
{
    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    // text
    const float cliprect_left = static_cast<float>(renderer.cmd_header().clip_rect.left);
    renderer.push_clip_rect(
        r2::vec2(cliprect_left, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.w),
        true
    );

    const float border_size = style.border_size.get(instance()->scale());

    const xstr text = name_ + basic_xstr<char, 3>(": ");

    renderer.add_text_faded(
        r2::vec2(last_pos_.x, last_pos_.y),
        style.text().transparent(),
        style.text().alpha((alpha * (1.f - animation_disabled_)) *
            (0.3f + animation_selected_ * 0.4f)),
        cliprect_left, std::max(cliprect_left, last_pos_.x + border_size),
        text, 
        true /* blurred */
    );

    if (!text_width_calculated_) {
        if (renderer.get_text_width_strict(text, text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(text);
        text_width_ = std::ceil(text_width_);
    }

    renderer.add_text(
        r2::vec2(last_pos_.x, last_pos_.y),
        util.disable_color(style.text().alpha(alpha), animation_disabled_),
        text
    );

    if (formatted_text_.empty() ||
        last_value_ != *value_) {
        last_value_ = *value_;
        formatted_text_ = std::format<float>(format_, (float)*value_);
    }

    renderer.add_text(
        r2::vec2(last_pos_.x + text_width_, last_pos_.y),
        style.accent().alpha((alpha * (1.f - animation_disabled_)) *
            (0.3f + animation_selected_ * 1.f)),
        formatted_text_,
        true /* blurred */
    );

    renderer.add_text(
        r2::vec2(last_pos_.x + text_width_, last_pos_.y),
        util.disable_color(style.accent().alpha(alpha), animation_disabled_),
        formatted_text_
    );

    renderer.pop_clip_rect();

    // slider bar
    const float spacing = style.spacing->get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float slider_text_spacing = std::round(spacing * 0.4f);
    const float available_height = last_pos_.h - text_size_small - slider_text_spacing;
    half_slider_height_ = std::round(available_height * 0.5f);

    const float line_height_half = std::round(available_height * 0.18f);

    slider_pos_ = r2::vec4(
        last_pos_.x, last_pos_.y + text_size_small,
        last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h
    );

    renderer.add_rect_filled(
        r2::vec2(last_pos_.x + half_slider_height_,
            last_pos_.y + text_size_small + half_slider_height_ - line_height_half),
        r2::vec2(last_pos_.x + last_pos_.w - half_slider_height_,
            last_pos_.y + text_size_small + half_slider_height_ + line_height_half),
        util.disable_color(style.accent2().alpha(alpha), animation_disabled_),
        rounding * 0.5f
    );

    renderer.add_rect_inner(
        r2::vec2(last_pos_.x + half_slider_height_,
            last_pos_.y + text_size_small + half_slider_height_ - line_height_half),
        r2::vec2(last_pos_.x + last_pos_.w - half_slider_height_,
            last_pos_.y + text_size_small + half_slider_height_ + line_height_half),
        util.disable_color(style.accent().alpha(
            alpha * (0.5f + animation_hovered_ * 0.5f)), animation_disabled_),
        border_size,
        rounding * 0.5f
    );

    // thumb
    const float thumb_offset = (last_pos_.w - half_slider_height_ * 2.f) * animation_;

    const r2::vec2 thumb_min = r2::vec2(
        last_pos_.x + thumb_offset,
        last_pos_.y + text_size_small
    );

    const r2::vec2 thumb_max = thumb_min + r2::vec2(half_slider_height_ * 2.f);

    renderer.path_rect(
        thumb_min, 
        thumb_max,
        half_slider_height_
    );

    auto& path = renderer.path();

    renderer.add_shadow_convex_filled(
        path.data(), static_cast<std::uint32_t>(path.size()),
        util.disable_color(style.accent().alpha(animation_selected_ * 0.5f), animation_disabled_),
        (thumb_max.y - thumb_min.y) * 0.7f, 
        true /* filled */
    );

    renderer.add_convex_filled(
        path.data(), static_cast<std::uint32_t>(path.size()),
        util.disable_color(style.highlight().alpha(alpha), animation_disabled_)
    );

    renderer.add_lines(
        path.data(), static_cast<std::uint32_t>(path.size()),
        util.disable_color(style.accent().alpha(
            alpha * (0.2f + animation_selected_ * 0.6f)), animation_disabled_),
        border_size,
        true
    );

    renderer.path_clear();
}

input_response slider::input(const input_base& input)
{
    if (!can_receive_input_simple(input) ||
        !can_receive_input_this(input))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_button_down) &&
        input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_quad(mouse_x, mouse_y, slider_pos_)) {
            input.set_selected(this);
        
            return input_response::handled();
        }
    }

    else if (input.is_selected(this) &&
             input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        input.clear_selected();

        update_value(mouse_x);

        return input_response::handled();
    }

    else if (input.event().is_message(message_type::mouse_move)) {
        if (input.is_selected(this)) {
            update_value(mouse_x);

            instance()->cursors().set_cursor(cursor::size_ew);

            return input_response::handled();
        }

        if (util::is_in_quad(mouse_x, mouse_y, slider_pos_)) {
            input.set_hovered(this);
            instance()->cursors().set_cursor(cursor::size_ew);
        
            return input_response::handled();
        }
    }

    return input_response::empty();
}

void slider::on_activate()
{
    widget::on_activate();

    animation_ = std::clamp((*value_ - min_) / (max_ - min_), 0.f, 1.f);
}

void slider::on_scale_change()
{
    text_width_calculated_ = false;
}

bool slider::matches_search(const xstr& search) noexcept
{
    return name_.find_ignore_case_f(search) != xstr::npos;
}

void slider::update_value(float mouse_x)
{
    float scale = (mouse_x - last_pos_.x - 
        half_slider_height_) / (last_pos_.w - half_slider_height_ * 2.f);

    set_value(scale);
}

void slider::set_value(float scale)
{
    scale = std::clamp(scale, 0.f, 1.f);

    const float new_value = min_ + (max_ - min_) * scale;

    constexpr float kPow10[] = {
        1.f, 10.f, 100.f, 1000.f, 10000.f, 100000.f, 1000000.f, 10000000.f
    };
    constexpr int kMaxDecimals = static_cast<int>(sizeof(kPow10) / sizeof(kPow10[0]));

    // clamp to decimals
    const float m = kPow10[std::clamp(num_decimals_, 0, kMaxDecimals)];
    const float temp = new_value * m;

    *value_ = std::round(temp) / m;
}

void_end_