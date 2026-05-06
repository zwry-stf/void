#include "label.h"
#include <void/void.h>


void_begin_

label::label(void_* instance, const xstr& name)
    : tab(instance, 
        nullptr, /* overlay owner */
        name,
        false /* is_container */
      )
{
}

float label::update(float x, float y, float w, const render_input&, bool)
{
    last_pos_.x = x;
    last_pos_.y = y;
    last_pos_.w = w;
    constexpr sfloat kLabelHeight = sfloat(26.f, true);
    last_pos_.h = kLabelHeight.get(instance()->scale());

    auto& renderer = instance()->renderer();

    if (!text_width_calculated_) {
        instance()->fonts().bind_font_small();
        if (renderer.get_text_width_strict(name(), text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(name());
    }

    return last_pos_.h;
}

void label::render(bool)
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    instance()->fonts().bind_font_small();

    const float border_size = style.border_size.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    renderer.push_clip_rect(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h),
        true
    );

    const r2::vec2 text_pos = r2::vec2(
        last_pos_.x + std::round((last_pos_.w - text_width_) * 0.5f),
        last_pos_.y + last_pos_.h - text_size_small - border_size
    );

    renderer.add_text(
        text_pos,
        style.text(),
        name()
    );

    renderer.add_rect_filled(
        r2::vec2(text_pos.x - border_size,
            text_pos.y + text_size_small - border_size),
        r2::vec2(text_pos.x + std::ceil(text_width_) + border_size,
            text_pos.y + text_size_small),
        style.text()
    );

    renderer.pop_clip_rect();
}

input_response label::input(const input_base&, std::int32_t&, std::int32_t)
{
    return input_response::empty();
}

void label::on_scale_change()
{
    text_width_calculated_ = false;
}

void_end_