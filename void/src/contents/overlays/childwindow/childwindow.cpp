#include "childwindow.h"
#include <void/void.h>


void_begin_

childwindow::childwindow(void_* instance, input_owner_overlay* input_owner, const xstr& name)
    : overlay(instance, input_owner, true, true),
      name_(name)
{
}

void childwindow::update(const overlay_render_input& input)
{
    const float old_animation = instance()->alpha() * alpha_;
    overlay::update(input);

    const float animation = instance()->alpha() * alpha_;
    if (animation < util::g_min_alpha) [[unlikely]]
        return;

    if (old_animation < util::g_min_alpha) [[unlikely]] { // overlay was invisible
        for (auto& w : widgets_)
            w->on_activate();
    }

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    const auto& menu_pos = instance()->pos();
    const auto display_size = renderer.get_render_size();

    const float overlay_width = style.overlay_width->get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    last_pos_.x = menu_pos.x + set_pos_.x;
    last_pos_.y = menu_pos.y + set_pos_.y;
    last_pos_.w = overlay_width;

    const float top_height = text_size_small + border_size * 4.f;
    const float widget_spacing = std::round(spacing * 0.75f);
    const float top_spacing = spacing;
    const float side_spacing = spacing;

    float full_height = top_height + top_spacing * 2.f;
    bool widget_found = false;
    const float pos_x = last_pos_.x + side_spacing;
    float pos_y = last_pos_.y + top_height + top_spacing;
    for (auto& widget : widgets_) {
        if (widget->is_visible()) [[likely]] {
            widget->update(
                pos_x,
                pos_y,
                last_pos_.w - side_spacing * 2.f,
                input_get_render_input(),
                false /* occluded */
            );

            widget_found = true;

            const float space = widget->get_height() + widget_spacing;
            full_height += space;
            pos_y += space;
        }
    }

    if (widget_found) [[likely]]
        full_height -= widget_spacing;

    last_pos_.h = full_height;

    // clamp position in window
    if (pos_changed_) {
        pos_changed_ = false;

        bool pos_changed = false;
        if (last_pos_.x + last_pos_.w > display_size.x) {
            last_pos_.x -= last_pos_.w;
            set_pos_.x -= last_pos_.w;
            pos_changed = true;
        }
        if (last_pos_.y + last_pos_.h > display_size.y) {
            last_pos_.y -= last_pos_.h;
            set_pos_.y -= last_pos_.h;
            pos_changed = true;
        }

        if (pos_changed) {
            // fix widget positions

            const float pos_x2 = last_pos_.x + side_spacing;
            pos_y = last_pos_.y + top_height + top_spacing;
            for (auto& widget : widgets_) {
                if (widget->is_visible()) [[likely]] {
                    widget->set_pos(
                        r2::vec2{
                            pos_x2,
                            pos_y
                        }
                    );

                    const float space = widget->get_height() + widget_spacing;
                    pos_y += space;
                }
            }
        }
    }
}

void childwindow::render()
{
    const float animation = instance()->alpha() * alpha_;
    if (animation < util::g_min_alpha)
        return;

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    renderer.push_clip_rect(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h),
        false
    );

    const float rounding = style.rounding->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    const float top_height = text_size_small + border_size * 4.f;

    instance()->background().add_immediate_overlay(
        last_pos_,
        animation
    );

    // title
    renderer.add_text(
        r2::vec2(last_pos_.x + std::max(border_size * 2.f, std::round(rounding * 0.5f)),
            last_pos_.y + border_size * 2.f),
        style.text().alpha(animation),
        name_
    );

    renderer.add_rect_filled(
        r2::vec2(last_pos_.x + border_size, last_pos_.y + top_height),
        r2::vec2(last_pos_.x + last_pos_.w - border_size, last_pos_.y + top_height + border_size),
        style.border().alpha(animation)
    );

    // items
    for (auto& widget : widgets_) {
        if (widget->is_visible()) [[likely]] {
            widget->render(
                animation
            );
        }
    }

    renderer.pop_clip_rect();
}

input_response childwindow::input(const overlay_input& input)
{
    if (!input.is_opened(this))
        return input_response::empty();

    // overlays
    if (input_overlay_opened_id() >= 0 &&
        input_overlay_opened_id() < static_cast<std::int32_t>(overlays_.size())) {
        auto res = overlays_[input_overlay_opened_id()]->input(
            input_get_overlay_input(input.event())
        );
        if (res.is_handled())
            return res;
    }

    // clicked outside of overlay rect
    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_button_down) &&
        (input.event().get_mouse_button() == mouse_button::left ||
            input.event().get_mouse_button() == mouse_button::right)) {
        if (!util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            input.clear_opened();

            return input_response::handled();
        }
    }

    // widgets
    input_base _input = input_get_input(input.event());

    for (auto& widget : widgets_) {
        if (widget->is_visible()) {
            auto res = widget->input(_input);
            if (res.is_handled())
                return res;
        }
    }

    return should_block_input(input);
}

void childwindow::render_overlays()
{
    auto render_input = input_get_overlay_render_input();

    for (auto& overlay : overlays_) {
        assert(overlay->has_overlays() == false);

        overlay->update(render_input);
        overlay->render();
    }
}

void childwindow::on_scale_changed()
{
    overlay::on_scale_changed();

    for (auto& w : widgets_)
        w->on_scale_change();
    for (auto& o : overlays_)
        o->on_scale_changed();
}

void_end_