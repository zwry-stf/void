#include "group.h"
#include <void/void.h>


void_begin_

group::group(void_* instance, const xstr& name, group_area area)
    : vobj(instance),
      area_(area),
      name_(name)
{
}

float group::update(float x, float y, float w, const render_input& input, float& substract_last_widget)
{
    last_pos_.x = x;
    last_pos_.y = y;
    last_pos_.w = w;

    occluded_ = false;
    bool dont_render = true;
    for (auto& widget : widgets_)
        if (!widget->is_skipped() &&
            widget->is_visible()) {
            dont_render = false;
            break;
        }
    if (dont_render) {
        occluded_ = true;
        return 0.f;
    }

    instance()->fonts().bind_font_small();

    auto& style = instance()->style();
    const auto& menu_pos = instance()->pos();

    occluded_ = false;
    const float lowest_pos = menu_pos.y + menu_pos.w;
    const float highest_pos = menu_pos.y + style.top_bar_height.get(instance()->scale());
    // out of screen bottom
    if (y > lowest_pos)
        occluded_ = true;

    const float spacing = style.spacing->get(instance()->scale());
    const float widget_spacing = std::round(spacing * 0.75f);
    constexpr sfloat kTopSpacing = sfloat(10.f);
    const float top_spacing = kTopSpacing.get(instance()->scale());
    float pos_y = y + top_spacing;
    for (auto& widget : widgets_) {
        if (!widget->is_skipped() &&
            widget->is_visible()) {
            bool occluded = (pos_y > lowest_pos) ? true : false;

            widget->update(
                x + spacing,
                pos_y,
                w - spacing * 2.f,
                input,
                occluded_ || occluded
            );

            pos_y += widget->get_height() + widget_spacing;
            substract_last_widget = widget->get_height() + widget_spacing * 2.f;
        }
    }

    const float full_height = pos_y - y;

    if (y + full_height < highest_pos)
        occluded_ = true;

    last_pos_.h = full_height;

    return full_height;
}

void group::render(float alpha)
{
    if (occluded_) {
        return;
    }

    instance()->fonts().bind_font_small();

    render_outline(alpha);

    for (auto& widget : widgets_) {
        if (!widget->is_skipped() &&
            widget->is_visible()) {
            widget->render(
                alpha
            );
        }
    }
}

input_response group::input(const input_base& input, bool selected_only)
{
    for (auto& widget : widgets_) {
        if (!widget->is_skipped() &&
            widget->is_visible()) {
            if (!selected_only ||
                widget->is_selected(input)) {
                auto res = widget->input(input);
                if (res.is_handled())
                    return res;
            }
        }
    }

    return input_response::empty();
}

void group::on_activate()
{
    for (auto& widget : widgets_)
        widget->on_activate();
}

void group::on_scale_change()
{
    text_width_calculated_ = false;

    for (auto& widget : widgets_)
        widget->on_scale_change();
}

void group::search(const xstr& text)
{
    if (text.empty())
        for (auto& widget : widgets_)
            widget->set_skipped(false);
    else
        for (auto& widget : widgets_)
            widget->set_skipped(!widget->matches_search(text));
}

void group::render_outline(float alpha)
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    if (!text_width_calculated_) {
        if (renderer.get_text_width_strict(name_, text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(name_);
        text_width_ = std::ceil(text_width_);
    }

    const float rounding = style.rounding->get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float top_bar_height = style.top_bar_height.get(instance()->scale());
    const float outline_rounding = std::round(rounding * 0.5f);
    const r2::vec2 min = r2::vec2(last_pos_.x, last_pos_.y);
    const r2::vec2 max = r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h);

    const float gradient_offset = spacing * 3.f;
    const float text_spacing = border_size;
    const float text_width = std::min(text_width_,
        last_pos_.w - rounding * 2.f - gradient_offset * 2.f - text_spacing * 2.f);

    // background
    renderer.add_rect_filled(
        min, max,
        style.group_background().alpha(alpha),
        outline_rounding
    );

    // outline
    const r2::vec2 const_offset = r2::vec2(border_size * 0.5f);

    renderer.path_add_point( // left point
        r2::vec2(max.x - outline_rounding - gradient_offset - text_spacing * 2.f - text_width,
            min.y) + const_offset
    );

    const float corner_size = outline_rounding * r2::math::g_pi_div_2;
    const float corner_step = 2.f;
    const float step = corner_step / corner_size * 2.f;
    renderer.path_arc_to<6,  9>(r2::vec2{ min.x + outline_rounding + const_offset.x,
        min.y + outline_rounding + const_offset.y }, outline_rounding, step);
    renderer.path_arc_to<9, 12>(r2::vec2{ min.x + outline_rounding + const_offset.x, 
        max.y - outline_rounding - const_offset.y }, outline_rounding, step);
    renderer.path_arc_to<0,  3>(r2::vec2{ max.x - outline_rounding - const_offset.x,
        max.y - outline_rounding - const_offset.y }, outline_rounding, step);
    renderer.path_arc_to<3,  6>(r2::vec2{ max.x - outline_rounding - const_offset.x,
        min.y + outline_rounding + const_offset.y }, outline_rounding, step);

    renderer.path_add_point( // right point
        r2::vec2(max.x - outline_rounding - gradient_offset,
            min.y) + const_offset
    );

    renderer.path_stroke(
        style.border().alpha(0.7f * alpha), 
        border_size,
        false /* closed */
    );

    const float text_offset = std::round(text_size_small * 0.5f);

    if (min.y + text_offset < 
        instance()->pos().y + top_bar_height)
        return;

    // gradients
    const r2::color transparent = style.accent().transparent();
    const r2::color color = style.accent().alpha(alpha);

    renderer.add_rect_filled_multicolor( // left line
        r2::vec2(max.x - outline_rounding - gradient_offset * 2.f - text_spacing * 2.f - text_width,
            min.y),
        r2::vec2(max.x - outline_rounding - gradient_offset - text_spacing * 2.f - text_width,
            min.y + border_size),
        transparent, color, color, transparent
    );

    renderer.add_rect_filled_multicolor( // right line
        r2::vec2(max.x - outline_rounding - gradient_offset,
            min.y),
        r2::vec2(max.x - outline_rounding,
            min.y + border_size),
        color, transparent, transparent, color
    );

    // text
    bool push_clip_rect = text_width != text_width_;
    const float text_pos = max.x - outline_rounding - gradient_offset - text_spacing;

    if (push_clip_rect) {
        renderer.push_clip_rect( // only right side clipping needed
            r2::vec2(min.x, min.y - text_size_small),
            r2::vec2(text_pos, min.y + text_size_small),
            true
        );
    }

    renderer.add_text(
        r2::vec2(text_pos - text_width,
            min.y - text_offset),
        style.text_accent().alpha(alpha), 
        name_
    );

    if (push_clip_rect) {
        renderer.pop_clip_rect();
    }
}

void_end_