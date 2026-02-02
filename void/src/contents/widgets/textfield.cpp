#include <void/contents/widgets/textfield.h>
#include <void/void.h>
#include <util/clipboard.h>


void_begin_

constexpr xstr kColorPrefix = "#";

textfield::textfield(void_* instance, input_owner* input_owner, textfield_type type, textfield_flags flags,
                     const xstr& default_text, const xstr& post_text, std::size_t max_length)
    : vobj(instance),
      input_receiver(input_owner, 1),
      type_(type),
      flags_(flags),
      default_text_(default_text),
      post_text_(post_text),
      max_length_(max_length)
{
    assert(default_text.find('\n') == xstr::npos);
    assert(post_text.find('\n') == xstr::npos);
}

bool textfield::is_valid_char(char32_t u) const noexcept
{
    char c = static_cast<char>(u);
    bool is_utf = c < 0x80;
    switch (type_) {
    case textfield_type::text:
        return u >= 0x20u && u != 0x7F;
    case textfield_type::color:
        if (is_utf && 
            (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F'))
            return true;
        break;
    case textfield_type::decimal:
        if (is_utf && c == '.') {
            for (std::uint8_t i = 0; i < buffer_.length(); i++)
                if (buffer_[i] == '.')
                    return false;
            return true;
        }
        [[fallthrough]];
    case textfield_type::integer:
        if (is_utf &&
            c >= '0' && c <= '9')
            return true;
        break;
    }

    return false;
}

float textfield::update(const r2::rectf& pos, const render_input& input, bool occluded)
{
    last_pos_ = pos;

    auto& renderer = instance()->renderer();
    auto& util     = instance()->util();

    // animations
    if (last_blink_ == time_point() ||
        std::chrono::duration<double>(instance()->frame_start() - last_blink_).count() >= 0.5) {
        last_blink_ = instance()->frame_start();
        blink_toggle_ = !blink_toggle_;
    }

    animation_selected_ = util.lerp(animation_selected_, input.is_selected(this), 1.5f);
    animation_blinking_ = util.lerp(animation_blinking_, blink_toggle_, 1.5f);

    if (occluded &&
        !input.is_selected(this)) {
        animated_caret_pos_ = 0.f;
        selected_overlay_animation_ = 0.f;
        text_render_x_animation_ = 0.f;
        return last_pos_.x + text_width_;
    }

    bool successfull_update = true;
    if (text_marked_dirty_) {
        if (buffer_.empty()) {
            if (!renderer.get_text_width_strict(default_text_, buffer_width_)) {
                successfull_update = false;
                buffer_width_ = renderer.get_text_width(default_text_);
            }
        }
        else {
            if (!renderer.get_text_width_strict(buffer_, buffer_width_)) {
                successfull_update = false;
                buffer_width_ = renderer.get_text_width(buffer_);
            }
        }

        buffer_width_ = std::ceil(buffer_width_);

        if (type_ == textfield_type::color) {
            prefix_width_ = renderer.get_text_width(kColorPrefix);
            prefix_width_ = std::ceil(prefix_width_);
        }
    }

    if (text_marked_dirty_ ||
        typing_pos_ != last_typing_pos_) {
        last_typing_pos_ = typing_pos_;
        if (!renderer.get_text_width_strict(
            buffer_,
            typing_pos_width_,
            0,
            std::optional(typing_pos_)
        )) {
            successfull_update = false;
            typing_pos_width_ = renderer.get_text_width(
                buffer_,
                0,
                std::optional(typing_pos_)
            );
        }

        if (successfull_update)
            text_marked_dirty_ = false;
    }

    // caret
    if (!(flags_ & textfield_flags::movable_caret)) {
        text_render_x_ = 0.f;
    }
    else {
        const float spacing = instance()->style().spacing->get(instance()->scale());

        if (typing_pos_ >= 0) {
            float caret_x = typing_pos_width_;
            const float left_offset = typing_pos_ == 0 ?
                0.f : std::round(spacing * 0.5f);

            if (caret_x - text_scroll_x_ > last_pos_.w)
                text_scroll_x_ = caret_x - last_pos_.w;

            else if (caret_x - text_scroll_x_ < left_offset)
                text_scroll_x_ = caret_x - left_offset;
        }

        text_render_x_ = -text_scroll_x_;
    }

    text_render_x_animation_ = util.lerp(text_render_x_animation_, text_render_x_, 2.5f);
    
    float text_width = 0.f;
    float render_caret_pos = 0.f;

    if (type_ == textfield_type::color) {
        text_width += prefix_width_;
    }
    else {
        prefix_width_ = 0.f;
    }

    if (buffer_.empty()) {
        text_width_ = 0.f;
        text_width += buffer_width_;

        if (need_caret_pos_) {
            typing_pos_ = 0;
            need_caret_pos_ = false;
        }
    }

    else {
        text_width += buffer_width_;

        render_caret_pos = typing_pos_width_;

        if (need_caret_pos_ ||
            caret_moving_) {
            auto last_typing_pos = typing_pos_;

            typing_pos_ = renderer.get_char_at_pos<true>(
                buffer_,
                caret_click_pos_
            );

            if (need_caret_pos_)
                selected_start_pos_ = typing_pos_;

            if (need_caret_pos_) {
                selected_start_render_pos_ = std::round(render_caret_pos);
            }
            text_selected_ = typing_pos_ != selected_start_pos_;

            if (last_typing_pos != typing_pos_)
                reset_blinking();

            need_caret_pos_ = false;
        }

        text_width_ = std::max(std::ceil(text_width), 0.f);
    }

    if (!post_text_calculated_) {
        if (renderer.get_text_width_strict(post_text_, post_text_width_))
            post_text_calculated_ = true;
        else
            post_text_width_ = renderer.get_text_width(post_text_);
    }

    render_caret_pos = std::round(render_caret_pos);
    animated_caret_pos_ = util.lerp(animated_caret_pos_, render_caret_pos, 2.5f);

    text_width += post_text_width_;

    if (text_selected_) {
        const float t_width = renderer.get_text_width(
            buffer_,
            0,
            std::optional(selected_start_pos_)
        );

        selected_start_render_pos_ = std::round(t_width);
    }

    selected_overlay_animation_ = util.lerp(selected_overlay_animation_,
        input.is_selected(this) && text_selected_, 2.f
    );

    return last_pos_.x + text_width_;
}

void textfield::render(float alpha, const r2::color& text_color)
{
    auto& style    = instance()->style();
    auto& renderer = instance()->renderer();

    const float spacing = style.spacing->get(instance()->scale());

    // text
    const r2::vec2 text_pos = r2::vec2(last_pos_.x + text_render_x_animation_, last_pos_.y);

    renderer.push_clip_rect(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h),
        true
    );

    // prefix
    if (type_ == textfield_type::color) {
        renderer.add_text(
            text_pos, text_color.alpha(alpha),
            kColorPrefix
        );
    }
    
    const float caret_pos = text_pos.x + prefix_width_ + animated_caret_pos_;

    if (selected_overlay_animation_ > util::g_min_alpha) {
        float start_pos = text_pos.x + prefix_width_ + selected_start_render_pos_;
        renderer.add_rect_filled(
            r2::vec2(std::min(start_pos, caret_pos), last_pos_.y),
            r2::vec2(std::max(start_pos, caret_pos), last_pos_.y + last_pos_.h),
            style.accent2().alpha(alpha * selected_overlay_animation_)
        );
    }

    const float fade_width = spacing * 2.f;
    const float fade_end = last_pos_.x + last_pos_.w;
    const float fade_start = std::max(last_pos_.x, last_pos_.x + last_pos_.w - fade_width);

    const auto faded_alpha = animation_selected_;

    if (buffer_.empty()) {
        const auto& grey_color = style.grey().alpha(alpha * (1.f - animation_selected_));

        (flags_ & textfield_flags::faded_text) ?
            renderer.add_text_faded(
                text_pos + r2::vec2(prefix_width_, 0.f),
                grey_color, grey_color.alpha(faded_alpha),
                fade_start, fade_end,
                default_text_) :
            renderer.add_text(
                text_pos + r2::vec2(prefix_width_, 0.f),
                grey_color,
                default_text_
            );
    }

    else {
        (flags_ & textfield_flags::faded_text) ?
            renderer.add_text_faded(
                text_pos + r2::vec2(prefix_width_, 0.f),
                text_color.alpha(alpha), text_color.alpha(alpha * faded_alpha),
                fade_start, fade_end,
                buffer_) :
            renderer.add_text(
                text_pos + r2::vec2(prefix_width_, 0.f),
                text_color.alpha(alpha),
                buffer_
            );
    }


    if (!post_text_.empty()) {
        const auto post_text_color = text_color.alpha(alpha * (1.f - animation_selected_ * 0.5f));
        (flags_ & textfield_flags::faded_text) ?
            renderer.add_text_faded(
                r2::vec2(text_pos.x + prefix_width_ + buffer_width_, text_pos.y),
                post_text_color, post_text_color.transparent(),
                fade_start, fade_end,
                post_text_) :
            renderer.add_text(
                r2::vec2(text_pos.x + prefix_width_ + buffer_width_, text_pos.y),
                post_text_color,
                post_text_
            );
    }

    renderer.pop_clip_rect();

    // caret
    if (animation_blinking_ * animation_selected_ > util::g_min_alpha) {
        const float border_size = style.border_size.get(instance()->scale());

        renderer.push_clip_rect(
            r2::vec2(last_pos_.x - border_size, last_pos_.y),
            r2::vec2(last_pos_.x + last_pos_.w + border_size, last_pos_.y + last_pos_.h),
            true
        );

        renderer.add_rect_filled(
            r2::vec2(caret_pos - border_size, last_pos_.y),
            r2::vec2(caret_pos + border_size, last_pos_.y + last_pos_.h),
            style.accent().alpha(animation_blinking_ * animation_selected_ * alpha)
        );

        renderer.pop_clip_rect();
    }
}

input_response textfield::input(const input_base& input)
{
    if (!input.nothing_selected() &&
        !input.is_selected(this))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    // Active behaviour
    if (input.is_selected(this) &&
        input.event().is_message(message_type::key_down)) {
        auto key = input.event().get_key();

        // paste
        if (key == key::v &&
            instance()->input().is_control_down()) {
            clear_selected();

            // paste
            std::wstring text;
            if (clipboard::get_clipboard(text)) {
                const std::uint32_t length = static_cast<std::uint32_t>(text.length());
                std::uint32_t s = 0u;
                while (s < length) {
                    if (buffer_.size() >= max_length_)
                        break;

                    r2::unicode::unicode_type cp =
                        r2::unicode::get_char_auto(text, length, s);
                    if (cp == r2::unicode::codepoint_invalid)
                        continue;

                    if (is_valid_char(cp)) {
                        buffer_.insert(typing_pos_, 1, cp);

                        typing_pos_++;
                        text_marked_dirty_ = true;
                    }
                }
                need_caret_pos_ = false;
            }
        }

        // copy / cut
        else if ((key == key::c || key == key::x) &&
                 instance()->input().is_control_down()) {
            // paste
            const std::int32_t start = std::min(
                std::min(typing_pos_, selected_start_pos_),
                static_cast<std::int32_t>(buffer_.length())
            );
            const std::int32_t end = std::min(
                std::max(typing_pos_, selected_start_pos_),
                static_cast<std::int32_t>(buffer_.length())
            );

            if (start == end) {
                if (key == key::x)
                    clear_selected();

                text_selected_ = false;

                return input_response::handled();
            }

            std::wstring text;
            for (std::int32_t i = start; i < end; i++) {
                const auto c = buffer_[i];
                wchar_t buf[4];
                std::uint32_t l = r2::unicode::put_char_to_array(c, buf);
                if (l == 0u)
                    continue;
                text.append(&buf[0], &buf[0] + l);
            }

            clipboard::copy_to_clipboard(text);

            if (key == key::x)
                clear_selected();

            text_selected_ = false;
        }

        // select all
        else if (key == key::a &&
                 instance()->input().is_control_down()) {
            selected_start_pos_ = 0;
            typing_pos_ = static_cast<std::int32_t>(buffer_.length());
            text_selected_ = true;
        }

        else if (key == key::escape ||
                (key == key::enter && 
                    (flags_ & textfield_flags::stop_on_return))) {
            input.clear_selected();

            invoke_stop_callback();
        }

        else if (key == key::left) {
            if (text_selected_ &&
                !instance()->input().is_shift_down()) {
                typing_pos_ = std::max(
                    typing_pos_ == selected_start_pos_ ?
                    typing_pos_ - 1 : std::min(typing_pos_, selected_start_pos_), 0
                );

                text_selected_ = false;
            }
            else if (typing_pos_ > 0) {
                if (!text_selected_ &&
                    instance()->input().is_shift_down()) {
                    selected_start_pos_ = typing_pos_;

                    text_selected_ = true;
                }

                typing_pos_--;
            }

            reset_blinking();
        }

        else if (key == key::right) {
            if (text_selected_ &&
                !instance()->input().is_shift_down()) {
                typing_pos_ = std::min(
                    typing_pos_ == selected_start_pos_ ?
                    typing_pos_ + 1 : std::max(typing_pos_, selected_start_pos_),
                    static_cast<std::int32_t>(buffer_.length())
                );

                text_selected_ = false;
            }
            else if (typing_pos_ < static_cast<std::int32_t>(buffer_.length())) {
                if (!text_selected_ &&
                    instance()->input().is_shift_down()) {
                    selected_start_pos_ = typing_pos_;

                    text_selected_ = true;
                }

                typing_pos_++;
            }

            reset_blinking();
        }

        else if (key == key::backspace ||
                 key == key::del) {
            if (text_selected_)
                clear_selected();
            else if ((key == key::backspace && typing_pos_ > 0) ||
                     (key == key::del && typing_pos_ <= static_cast<std::int32_t>(buffer_.length()) - 1)) {
                auto pos = typing_pos_ - (key == key::backspace ? 1 : 0);
                buffer_.erase(pos, 1);

                if (key == key::backspace ||
                    typing_pos_ > static_cast<std::int32_t>(buffer_.length()))
                    typing_pos_--;
                text_marked_dirty_ = true;
            }

            reset_blinking();
        }

        return input_response::handled();
    }

    else if (input.is_selected(this) &&
             input.event().is_message(message_type::char_down)) {
        auto key = input.event().get_char();
        if (is_valid_char(static_cast<char32_t>(key))) {
            clear_selected();

            if (buffer_.length() < max_length_) {
                buffer_.insert(typing_pos_, 1, static_cast<char32_t>(key));
                typing_pos_++;
            }
            text_marked_dirty_ = true;
        }

        reset_blinking();

        return input_response::handled(); // still block input since we are selected
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y,
                last_pos_.x, last_pos_.y,
                (flags_ & textfield_flags::mouse_in_rect) ?
                    last_pos_.w : std::min(
                        text_width_ + prefix_width_ + post_text_width_ +
                        instance()->style().spacing->get(instance()->scale()), last_pos_.w),
                last_pos_.h)) {
            caret_click_pos_ = mouse_x - last_pos_.x - text_render_x_ -
                prefix_width_ + instance()->style().border_size.get(instance()->scale());

            reset_blinking();

            input.set_selected(this);

            need_caret_pos_ = true;
            caret_moving_ = true;

            return input_response::handled();
        }
        else if (input.is_selected(this)) {
            input.clear_selected();

            invoke_stop_callback();

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left &&
             caret_moving_) {
        caret_moving_ = false;

        return input_response::handled();
    }

    if (input.event().is_message(message_type::mouse_move)) {
        if (caret_moving_) {
            caret_click_pos_ = mouse_x - last_pos_.x - text_render_x_ -
                prefix_width_ + instance()->style().border_size.get(instance()->scale());
        }

        if (input.is_selected(this) ||
            util::is_in_rect(mouse_x, mouse_y,
                last_pos_.x, last_pos_.y,
                (flags_ & textfield_flags::mouse_in_rect) ?
                    last_pos_.w : std::min(
                        text_width_ + prefix_width_ + post_text_width_ +
                        instance()->style().spacing->get(instance()->scale()), last_pos_.w),
                last_pos_.h)) {
            input.set_hovered(this);
            instance()->cursors().set_cursor(cursor::text);

            return input_response::handled();
        }
    }

    return input_response::empty();
}

void textfield::set_pos(float delta_x, float delta_y)
{
    last_pos_.x += delta_x;
    last_pos_.y += delta_y;
}

void textfield::on_activate()
{
    animation_blinking_ = 0.f;
    animation_selected_ = 0.f;
}

void textfield::on_scale_changed()
{
    text_marked_dirty_ = true;
    post_text_calculated_ = false;
}

void textfield::set_string(const std::u32string& s)
{
    buffer_.clear();
    buffer_.reserve(s.size());
    for (auto c : s) {
        if (is_valid_char(c)) {
            buffer_ += c;
        }

        if (buffer_.length() == max_length_)
            break;
    }
    text_marked_dirty_ = true;
}

void textfield::reset_blinking() noexcept
{
    blink_toggle_ = false; // will be toggled to true in next frame
    last_blink_ = {};
}

void textfield::clear_selected()
{
    if (!text_selected_)
        return;

    const std::int32_t start = std::min(
        std::min(typing_pos_, selected_start_pos_),
        static_cast<std::int32_t>(buffer_.length())
    );
    const std::int32_t end = std::min(
        std::max(typing_pos_, selected_start_pos_),
        static_cast<std::int32_t>(buffer_.length())
    );

    if (start == end)
        return;

    buffer_.erase(start, end - start);

    typing_pos_ = start;
    text_selected_ = false;
    text_marked_dirty_ = true;
}

void textfield::start_typing(bool reset_scroll)
{
    typing_pos_ = 0;

    if (reset_scroll) [[likely]]
        text_scroll_x_ = 0.f;

    reset_blinking();
}

void textfield::invoke_stop_callback()
{
    if (callback_)
        callback_(buffer_);
}

void_end_