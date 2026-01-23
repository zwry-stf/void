#include "toggle.h"
#include <void/void.h>


void_begin_

toggle::toggle(void_* instance, input_owner* input_owner,
               const xstr& name, bool* value)
    : widget(instance, input_owner, 1),
      name_(name),
      value_(value)
{
    assert(value != nullptr);
    assert(name_.find('\n') == xstr::npos);
}

void toggle::update(float x, float y, float w, const render_input& input, bool occluded)
{
    widget::update(x, y, w, input, occluded);

    constexpr sfloat kToggleHeight = sfloat(26.f, true);
    last_pos_.h = kToggleHeight.get(instance()->scale());

    auto& util = instance()->util();
    auto& style = instance()->style();

    // animations
    animation_ = util.lerp2(animation_, *value_);

    if (occluded)
        return;

    // toggle
    const float spacing = style.spacing->get(instance()->scale());
    const float scaled_height = last_pos_.h * 0.55f;
    const float toggle_width = std::round(scaled_height * 2.f);

    const float toggle_spacing_y = std::round(scaled_height * 0.42f);
    const float toggle_spacing_x = std::round(spacing * 0.4f);

    toggle_pos_ = r2::rectf(
        x + w - toggle_spacing_x - toggle_width,
        y + toggle_spacing_y,
        toggle_width,
        last_pos_.h - toggle_spacing_y * 2.f
    );

    // child widgets
    update_child_widgets(
        toggle_pos_.x,
        input
    );
}

void toggle::render(float alpha)
{
    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    // toggle
    const r2::vec2 toggle_min = r2::vec2(toggle_pos_.x, toggle_pos_.y);
    const r2::vec2 toggle_max = r2::vec2(toggle_pos_.x + toggle_pos_.w, toggle_pos_.y + toggle_pos_.h);
    const float rounding = style.rounding->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());

    renderer.add_shadow_rect_filled(
        toggle_min, toggle_max,
        util.disable_color(style.accent().alpha(alpha * animation_selected_ * 0.5f), animation_disabled_),
        rounding, 
        (toggle_max.y - toggle_min.y) * 0.7f
    );

    renderer.add_rect_filled(
        toggle_min, toggle_max,
        util.disable_color(style.accent2().alpha(alpha), animation_disabled_),
        rounding
    );

    renderer.add_rect_inner(
        toggle_min, toggle_max,
        util.disable_color(style.accent().alpha(alpha * (0.5f + animation_hovered_ * 0.5f)), animation_disabled_),
        border_size,
        rounding
    );

    // thumb
    const float thumb_animation = border_size * (0.5f - std::abs(0.5f - animation_));
    const float thumb_size = toggle_max.y - toggle_min.y - border_size * 2.f;
    const float animation_space = (toggle_max.x - toggle_min.x) - thumb_size - border_size * 2.f;

    renderer.add_rect_filled(
        r2::vec2(toggle_min.x + border_size + animation_space * animation_ + thumb_animation,
            toggle_min.y + border_size + thumb_animation),
        r2::vec2(toggle_min.x + border_size + animation_space * animation_ - thumb_animation + thumb_size,
            toggle_min.y + border_size - thumb_animation + thumb_size),
        util.disable_color(style.highlight().alpha(alpha), animation_disabled_),
        rounding
    );

    // child widgets
    render_child_widgets(alpha);

    // text
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float text_spacing = std::round((last_pos_.h - text_size_small) * 0.5f);
    const float cliprect_left = static_cast<float>(renderer.cmd_header().clip_rect.left);
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
        util.disable_color(style.text().alpha(alpha), animation_disabled_),
        name_
    );

    renderer.pop_clip_rect();
}

input_response toggle::input(const input_base& input)
{
    if (!can_receive_input_simple(input))
        return input_response::empty();

    // child widgets
    auto res = input_child_widgets(input);
    if (res.is_handled())
        return res;

    if (!can_receive_input_this(input))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    // toggle
    if (input.event().is_message(message_type::mouse_button_down) &&
        input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, toggle_pos_)) {
            input.set_selected(this);
            return input_response::handled();
        }
    }

    else if (input.is_selected(this) &&
             input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        input.clear_selected();

        if (util::is_in_rect(mouse_x, mouse_y, toggle_pos_)) {
            toggle_value();
        }

        return input_response::handled();
    }

    else if (input.event().is_message(message_type::mouse_move)) {
        if (input.is_selected(this)) {
            input.set_hovered(this);
            instance()->cursors().set_cursor(cursor::hand);
            return input_response::handled();
        }

        if (util::is_in_rect(mouse_x, mouse_y, toggle_pos_)) {
            input.set_hovered(this);
            instance()->cursors().set_cursor(cursor::hand);
            return input_response::handled();
        }
    }

    return input_response::empty();
}

void toggle::set_pos(const r2::vec2& pos)
{
    const float delta_x = pos.x - last_pos_.x;
    const float delta_y = pos.y - last_pos_.y;

    widget::set_pos(pos);

    toggle_pos_.x += delta_x;
    toggle_pos_.y += delta_y;

    set_child_widget_pos(delta_x, delta_y);
}

void toggle::on_activate()
{
    widget::on_activate();

    animation_ = *value_ ? 1.f : 0.f;
}

bool toggle::matches_search(const xstr& search) noexcept
{
    return name_.find_ignore_case_f(search) != xstr::npos;
}

void toggle::toggle_value() noexcept
{
    *value_ = !*value_;
}

void_end_