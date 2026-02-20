#include <void/contents/widgets/scrollbar.h>
#include <void/void.h>


void_begin_

scrollbar::scrollbar(void_* instance, input_owner* input_owner)
    : vobj(instance),
      input_receiver(input_owner, 1)
{
}

void scrollbar::update(const r2::rectf& pos, float highest)
{
    assert(pos.w > 0.f);
    assert(pos.h >= 0.f);

    highest_pos_ = highest;

    last_pos_ = pos;
    
    auto& util  = instance()->util();
    auto& style = instance()->style();

    // scrolling update
    des_scroll_ -= scroll_speed_ * instance()->delta_time() * style.animation_speed() * 2.2f;
    scroll_speed_ = util.lerp(scroll_speed_, 0.f, 1.75f);

    if (des_scroll_ < 0.f)
        des_scroll_ = util.lerp(des_scroll_, 0.f, 2.1f);

    if (des_scroll_ > highest)
        des_scroll_ = util.lerp(des_scroll_, highest, 2.1f);

    if (std::abs(scroll_speed_) < 0.5f)
        des_scroll_ = util.lerp(des_scroll_, std::round(des_scroll_));

    scroll_ = util.lerp(scroll_, des_scroll_, 1.75f);

    if (pos.h <= 0.f) {
        return;
    }

    // render
    bar_cache_.width = pos.w - style.border_size.get(instance()->scale());
    bar_cache_.offset = 0.f;

    const float min_scroll_bar_height = bar_cache_.width * 4.f;
    const float scroll_bar_start_y = last_pos_.y + bar_cache_.offset;

    const float content_height = highest + pos.h;

    const float scroll_bar_height = (std::max)(
        min_scroll_bar_height, pos.h * (pos.h / content_height)
    );
    bar_cache_.last_height = scroll_bar_height;

    const float scroll_offset = highest > 0.f ?
        ((scroll_ / highest) * (pos.h - scroll_bar_height)) : scroll_;

    float scroll_top = scroll_bar_start_y + scroll_offset;
    const bool is_on_top = scroll_offset < 0.f;
    if (is_on_top)
        scroll_top = scroll_bar_start_y;

    float scroll_bottom = scroll_bar_start_y + scroll_offset + scroll_bar_height;
    if (scroll_offset + scroll_bar_height > pos.h) {
        scroll_bottom = scroll_bar_start_y + pos.h;
        if (scroll_bottom - scroll_top < bar_cache_.width) {
            // scroll bar should be atleast a circle big
            scroll_top = scroll_bottom - bar_cache_.width;
        }
    }

    if (is_on_top &&
        scroll_bottom - scroll_top < bar_cache_.width)
        scroll_bottom = scroll_top + bar_cache_.width;

    bar_cache_.last_pos.x = last_pos_.x -
        style.border_size.get(instance()->scale());
    bar_cache_.last_pos.y = scroll_top;
    bar_cache_.last_pos.w = bar_cache_.width;
    bar_cache_.last_pos.h = scroll_bottom - scroll_top;
}

void scrollbar::render(float alpha)
{
    if (last_pos_.h <= 0.f) {
        return;
    }

    auto& renderer = instance()->renderer();
    auto& style    = instance()->style();

    renderer.add_rect_filled(
        r2::vec2(last_pos_.x, bar_cache_.last_pos.y),
        r2::vec2(last_pos_.x + bar_cache_.width, 
            bar_cache_.last_pos.y + bar_cache_.last_pos.h),
        style.accent().alpha(alpha), 
        bar_cache_.width * 0.5f
    );
}

input_response scrollbar::input(const input_base& input)
{
    if (!input.nothing_selected() &&
        !input.is_selected(this))
        return input_response::empty();

    if (last_pos_.h <= 0.f) {
        return input_response::empty();
    }

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_button_down) &&
        input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, bar_cache_.last_pos)) {
            moving_off_ = mouse_y - bar_cache_.last_pos.y;
            input.set_selected(this);
            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (input.is_selected(this)) {
            input.clear_selected();
            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_move)) {
        if (input.is_selected(this)) {
            float pos = mouse_y - moving_off_ - last_pos_.y;
            if (pos + bar_cache_.last_height > last_pos_.h) {
                pos = last_pos_.h - bar_cache_.last_height;
            }
            else if (pos < 0.f) {
                pos = 0.f;
            }

            scroll_speed_ = 0.f;

            float d = last_pos_.h - bar_cache_.last_height;
            if (d != 0.f)
                des_scroll_ = (pos / d) * highest_pos_;
            else
                des_scroll_ = 0.f;
            instance()->cursors().set_cursor(cursor::hand);
            return input_response::handled();
        }

        if (util::is_in_rect(mouse_x, mouse_y, bar_cache_.last_pos)) {
            input.set_hovered(this);
            instance()->cursors().set_cursor(cursor::hand);
            return input_response::handled();
        }
    }

    return input_response::empty();
}

void scrollbar::on_scroll(float scroll)
{
    const float viewport_h = (std::max)(last_pos_.h, 1.f);

    const float step_px = viewport_h / instance()->style().scroll_speed;

    scroll_speed_ += scroll * step_px * instance()->style().scroll_speed;

    const float max_scroll_speed = instance()->style().max_scroll_speed;
    scroll_speed_ = std::clamp(scroll_speed_, -max_scroll_speed, max_scroll_speed);
}

void_end_