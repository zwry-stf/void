#include <void/contents/widgets/widget.h>
#include <void/void.h>


void_begin_

widget::widget(void_* instance, input_owner* input_owner, std::int32_t input_num) noexcept
    : vobj(instance),
      input_receiver(input_owner, input_num)
{
}

widget::~widget() = default;

void widget::update(float x, float y, float w, const render_input& input, bool occluded)
{
    was_occluded_ = occluded;

    last_pos_.x = x;
    last_pos_.y = y;
    last_pos_.w = w;

    auto& util = instance()->util();

    // animations
    animation_hovered_ = util.lerp(animation_hovered_, input.is_hovered(this));
    animation_selected_ = util.lerp(animation_selected_, input.is_selected(this));
    animation_disabled_ = util.lerp(animation_disabled_, is_disabled());
}

void widget::on_activate()
{
    animation_disabled_ = is_disabled() ? 1.f : 0.f;
    animation_hovered_ = 0.f;
    animation_selected_ = 0.f;

    for (auto& c : child_widgets_)
        c->on_activate();
}

bool widget::matches_search(const xstr& search) noexcept
{
    return search.empty();
}

bool widget::can_receive_input_this(const input_base& input) const noexcept
{
    if (!input.nothing_selected() &&
        !input.is_selected(this))
        return false;
    
    return true;
}

bool widget::can_receive_input_simple(const input_base& input) const noexcept
{
    if (was_occluded_ &&
        !is_selected(input))
        return false;

    if (is_disabled() ||
        !is_visible()) {
        if (input.is_selected(this))
            input.clear_selected();
        return false;
    }

    return true;
}

void widget::update_child_widgets(float right_x, const render_input& input)
{
    auto& style = instance()->style();

    const float child_widget_spacing = style.spacing->get(instance()->scale() * 0.5f);

    const float height = style.childwidget_size.get(instance()->scale());
    const float spacing = std::round((last_pos_.h - height) * 0.5f);
    const float pos_y = last_pos_.y + spacing;

    float pos_x = right_x;
    for (auto& child : child_widgets_) {
        pos_x -= child_widget_spacing;

        pos_x -= child->update(
            last_pos_,
            pos_x, pos_y, height,
            input
        );
    }

    child_widget_left_pos_ = pos_x;
}

void widget::render_child_widgets(float alpha)
{
    for (auto& child : child_widgets_) {
        child->render(
            last_pos_,
            alpha, 
            animation_disabled_
        );
    }
}

input_response widget::input_child_widgets(const input_base& input)
{
    // make sure mouse input is inside widget pos
    if (input.event().has_cursor_pos()) {
        float mouse_x, mouse_y;
        input.event().get_cursor_pos(mouse_x, mouse_y);

        if (mouse_x < last_pos_.x ||
            mouse_y < last_pos_.y ||
            mouse_x >= last_pos_.x + last_pos_.w ||
            mouse_y >= last_pos_.y + last_pos_.h) {
            if (!input.nothing_selected()) {
                for (auto& widget : child_widgets_) {
                    if (input.is_selected(widget.get())) {
                        return widget->input(input);
                    }
                }
            }
            return input_response::empty();
        }
    }

    for (auto& widget : child_widgets_) {
        auto res = widget->input(input);
        if (res.is_handled())
            return res;
    }

    return input_response::empty();
}


void_end_