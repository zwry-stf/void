#include "textfield.h"
#include <void/void.h>


void_begin_

textfield_widget::textfield_widget(void_* instance, input_owner* input_owner,
                                   const xstr& name, std::function<void(const std::u32string&)>&& callback, 
                                   textfield_type type, textfield_flags flags, const xstr& default_text, std::size_t max_length)
    : widget(instance, input_owner, 0),
      name_(name),
      callback_(std::move(callback))
{
    assert(name.find('\n') == xstr::npos);
    assert((bool)callback_);

    change_textfield(
        std::make_unique<textfield>(
            instance, input_owner,
            type,
            flags,
            default_text,
            xstr(), /* post text */
            max_length
        )
    );
}

void textfield_widget::change_textfield(std::unique_ptr<textfield>&& textfield)
{
    assert((bool)textfield);

    text_field_ = std::move(textfield);

    text_field_->set_stop_callback(
        [this](const std::u32string& s) -> void {
            this->callback_(s);
        }
    );
}

void textfield_widget::update(float x, float y, float w, const render_input& input, bool occluded)
{
    was_occluded_ = occluded;

    last_pos_.x = x;
    last_pos_.y = y;
    last_pos_.w = w;

    auto& util = instance()->util();

    // animations
    animation_hovered_ = util.lerp(animation_hovered_, input.is_hovered(text_field_.get()));
    animation_selected_ = util.lerp(animation_selected_, input.is_selected(text_field_.get()));
    animation_disabled_ = util.lerp(animation_disabled_, is_disabled());

    constexpr sfloat kTextfieldHeight = sfloat(26.f, true);
    last_pos_.h = kTextfieldHeight.get(instance()->scale());

    auto& style = instance()->style();

    const float spacing = style.spacing->get(instance()->scale());

    if (occluded)
        return;

    const float textfield_width = std::round(
        std::min(std::max(w * 0.45f, last_pos_.h * 3.5f),
            last_pos_.w - spacing)
    );
    const float spacing_y = std::round(spacing * 0.25f);

    text_pos_.x = x + w - textfield_width;
    text_pos_.y = y + spacing_y;
    text_pos_.w = textfield_width;
    text_pos_.h = last_pos_.h - spacing_y * 2.f;

    update_child_widgets(text_pos_.x, input);

    const float text_size_small = style.text_size_small.get(instance()->scale());

    const float text_field_spacing = std::round(spacing * 0.5f);
    const float text_spacing = std::round((last_pos_.h - text_size_small) * 0.5f);

    instance()->fonts().bind_font_small();
    text_field_->update(
        r2::rectf{
            text_pos_.x + text_field_spacing,
            last_pos_.y + text_spacing,
            text_pos_.w - text_field_spacing * 2.f,
            text_size_small
        },
        input,
        occluded
    );
}

void textfield_widget::render(float alpha)
{
    if (was_occluded_) {
        return;
    }

    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    instance()->fonts().bind_font_small();

    const r2::vec2 min = r2::vec2(text_pos_.x, text_pos_.y);
    const r2::vec2 max = min + r2::vec2(text_pos_.w, text_pos_.h);

    const float border_size = style.border_size.get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float text_spacing = std::round((last_pos_.h - text_size_small) * 0.5f);

    // text
    const float cliprect_left = static_cast<float>(renderer.cmd_header().clip_rect.left);
    renderer.modify_clip_rect_x(
        cliprect_left, 
        child_widget_left_pos_ - border_size
    );

    renderer.add_text_faded(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        style.text().transparent(),
        style.text().alpha((alpha * (1.f - animation_disabled_)) *
            (0.3f + animation_selected_ * 0.4f)),
        cliprect_left, 
        std::max(cliprect_left, last_pos_.x + border_size),
        name_, 
        true /* blurred */
    );

    renderer.add_text(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        util.disable_color(style.text().alpha(alpha), animation_disabled_),
        name_
    );

    renderer.pop_clip_rect();

    // text field
    renderer.add_shadow_rect_filled(
        min, max,
        util.disable_color(style.accent().alpha(
            alpha * animation_selected_ * 0.5f), animation_disabled_
        ),
        rounding, 
        (max.y - min.y) * 0.7f
    );

    renderer.add_rect_filled(
        min, max,
        util.disable_color(style.group_background().alpha(alpha), animation_disabled_),
        rounding * 0.5f
    );

    const r2::color text_color = util.disable_color(
        style.text().interp(style.text_accent(),
            0.4f + text_field_->animation_selected() * 0.6f), animation_disabled_
    );
    text_field_->render(alpha, text_color);

    // border
    renderer.add_rect_inner(
        min, max,
        util.disable_color(
            style.accent2().interp(
                style.accent(), animation_hovered_).alpha(alpha),
            animation_disabled_
        ),
        border_size,
        rounding * 0.5f
    );

    render_child_widgets(alpha);
}

input_response textfield_widget::input(const input_base& input)
{
    if (!can_receive_input_simple(input))
        return input_response::empty();

    return text_field_->input(input);
}

void textfield_widget::set_pos(const r2::vec2& pos)
{
    const float delta_x = pos.x - last_pos_.x;
    const float delta_y = pos.y - last_pos_.y;

    widget::set_pos(pos);

    text_pos_.x += delta_x;
    text_pos_.y += delta_y;

    text_field_->set_pos(delta_x, delta_y);
    set_child_widget_pos(delta_x, delta_y);
}

void textfield_widget::on_activate()
{
    widget::on_activate();

    text_field_->on_activate();
}

void textfield_widget::on_scale_change()
{
    widget::on_scale_change();

    text_field_->on_scale_changed();
}

bool textfield_widget::matches_search(const xstr& search) noexcept
{
    return name_.find_ignore_case_f(search) != xstr::npos;
}

void_end_