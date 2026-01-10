#include "colorpicker.h"
#include <void/void.h>
#include <array>
#include <util/clipboard.h>


void_begin_

enum class colorpicker_selected_ids : std::int32_t {
    selected_square, // brightness square
    selected_hue,
    selected_alpha,
    selected_copy,
    selected_max
};

colorpicker_overlay::colorpicker_overlay(void_* instance, input_owner* input_owner,
                                         input_owner_overlay* overlay_owner, r2::color* color, bool has_alpha)
    : overlay(instance, overlay_owner, false, false),
      input_receiver(input_owner,
          std::to_underlying(colorpicker_selected_ids::selected_max)),
      color_(color),
      has_alpha_(has_alpha),
      text_field_size_(26.f, true)
{
    text_field_ = std::make_unique<textfield>(
        instance, 
        input_owner,
        textfield_type::color,
        textfield_flags::mouse_in_rect | textfield_flags::stop_on_return, /* flags */
        xstr(), /* default text */
        xstr(), /* post text */
        has_alpha ? 8u : 6u /* max length */
    );
    text_field_->set_stop_callback(
        [this](const std::u32string& s) -> void {
            this->on_stop_typing(s);
        }
    );
}

void colorpicker_overlay::update(const overlay_render_input& input)
{
    const float old_animation = instance()->alpha() * alpha_;
    overlay::update(input);

    const auto& menu_pos = instance()->pos();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();
    const auto display_size = renderer.get_render_size();

    // Animation
    if ((r2::color_u32)last_color_ != *color_) {
        load_hsv();
        load_hue();

        last_color_ = *color_;
    }

    auto render_input = input_owner_->input_get_render_input();

    if (!render_input.is_selected(text_field_.get())) {
        std::u32string text;
        color_to_string(*color_, text, has_alpha_);
        text_field_->set_string(text);
    }

    const float animation = instance()->alpha() * alpha_;

    if (animation < util::g_min_alpha) {
        // occluded update
        text_field_->update(
            r2::rectf(),
            render_input,
            true
        );
        return;
    }

    if (old_animation < util::g_min_alpha) [[unlikely]] { // overlay was invisible
        text_field_->on_activate();
    }

    // position
    last_pos_.x = menu_pos.x + set_pos_.x;
    last_pos_.y = menu_pos.y + set_pos_.y;

    const float spacing = style.spacing->get(instance()->scale());
    const float overlay_width = style.overlay_width->get(instance()->scale());
    const float text_field_size = text_field_size_.get(instance()->scale());

    spacing_ = std::round(spacing * 1.5f);
    const float main_size = std::round(overlay_width * 0.667f);
    last_pos_.h = main_size + text_field_size + spacing_;

    brightness_square_size_ = main_size - spacing_ * 2.f;
    bar_width_ = util::round_to_even(brightness_square_size_ * 0.12f);

    last_pos_.w = spacing_ + brightness_square_size_ +
        spacing_ + bar_width_ + spacing_;
    if (has_alpha_)
        last_pos_.w += bar_width_ + spacing_;

    // clamp position in window
    if (pos_changed_) {
        pos_changed_ = false;

        if (last_pos_.x + last_pos_.w > display_size.x) {
            last_pos_.x -= last_pos_.w;
            set_pos_.x -= last_pos_.w;
        }
        if (last_pos_.y + last_pos_.h > display_size.y) {
            last_pos_.y -= last_pos_.h;
            set_pos_.y -= last_pos_.h;
        }
    }

    update_brightness_square();
    update_hue_bar();

    if (has_alpha_) {
        update_alpha_bar();
    }

    update_text_field();
}

void colorpicker_overlay::render()
{
    const float animation = instance()->alpha() * alpha_;

    if (animation < util::g_min_alpha) {
        return;
    }

    instance()->fonts().bind_font_small();

    instance()->background().add_immediate_overlay(
        last_pos_, alpha_
    );

    render_brightness_square();
    render_hue_bar();

    if (has_alpha_) {
        render_alpha_bar();
    }

    render_text_field();
}

input_response colorpicker_overlay::input(const overlay_input& input)
{
    if (!input.is_opened(this))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    auto _input = input_owner_->input_get_input(input.event());

    if (input.event().is_message(message_type::key_down) &&
        _input.is_selected(text_field_.get())) {
        if (input.event().get_key() == key::v &&
            instance()->input().is_control_down()) {
            // paste
            std::wstring text;
            std::u32string buffer;
            bool failed = false;
            if (clipboard::get_clipboard(text)) {
                const std::uint32_t length = static_cast<std::uint32_t>(text.length());
                std::uint32_t s = 0u;
                while (s < length) {
                    r2::unicode::unicode_type cp =
                        r2::unicode::get_char_auto(text, length, s);
                    if (cp == r2::unicode::codepoint_invalid) {
                        failed = true;
                        break;
                    }

                    buffer += cp;
                }
            }

            // try parse
            r2::color new_color;
            if (string_to_color(buffer, new_color, has_alpha_,
                false /* allow_no_alpha: dont allow no alpha if has_alpha is present */)) {
                *color_ = new_color;
                text_field_->set_string(buffer);

                _input.clear_selected();
                return input_response::handled();
            }
        }
    }

    auto res = text_field_->input(_input);
    if (res.is_handled())
        return res;

    if (input.event().is_message(message_type::mouse_button_down) &&
        input.event().get_mouse_button() == mouse_button::left) {
        if (!util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            input.clear_opened();

            return input_response::handled();
        }

        else if (util::is_in_quad(mouse_x, mouse_y, copy_pos_)) {
            _input.set_selected(this, colorpicker_selected_ids::selected_copy);

            return input_response::handled();
        }

        // Brightness square
        else if (util::is_in_rect(mouse_x, mouse_y, square_pos_)) {
            _input.set_selected(this, colorpicker_selected_ids::selected_square);

            return input_response::handled();
        }

        // Hue bar
        else if (util::is_in_rect(mouse_x, mouse_y, hue_pos_)) {
            _input.set_selected(this, colorpicker_selected_ids::selected_hue);

            return input_response::handled();
        }

        // Alpha bar
        else if (has_alpha_ &&
                 util::is_in_rect(mouse_x, mouse_y, alpha_pos_)) {
            _input.set_selected(this, colorpicker_selected_ids::selected_alpha);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_move)) {
        if (_input.is_selected(this, colorpicker_selected_ids::selected_copy) ||
            (_input.nothing_selected() && 
                util::is_in_quad(mouse_x, mouse_y, copy_pos_))) {
            instance()->cursors().set_cursor(cursor::hand);

            _input.set_hovered(this, colorpicker_selected_ids::selected_copy);

            return input_response::handled();
        }

        // Selected hovered states
        else if (_input.is_selected(this, colorpicker_selected_ids::selected_square)) {
            instance()->cursors().set_cursor(cursor::size_all);

            hsv_.s = std::clamp(
                (mouse_x - square_pos_.x) / square_pos_.w,
                0.f, 1.f
            );
            hsv_.v = 1.f - std::clamp(
                (mouse_y - square_pos_.y) / square_pos_.h,
                0.f, 1.f
            );

            load_color();

            _input.set_hovered(this, colorpicker_selected_ids::selected_square);

            return input_response::handled();
        }

        else if (_input.is_selected(this, colorpicker_selected_ids::selected_hue)) {
            instance()->cursors().set_cursor(cursor::size_ns);

            hsv_.h = std::clamp(
                (mouse_y - hue_pos_.y) / hue_pos_.h,
                0.f, 1.f
            );

            load_color();

            _input.set_hovered(this, colorpicker_selected_ids::selected_hue);

            return input_response::handled();
        }

        else if (_input.is_selected(this, colorpicker_selected_ids::selected_alpha)) {
            instance()->cursors().set_cursor(cursor::size_ns);

            hsv_.a = 1.f - std::clamp(
                (mouse_y - alpha_pos_.y) / alpha_pos_.h,
                0.f, 1.f
            );
            load_color();

            _input.set_hovered(this, colorpicker_selected_ids::selected_alpha);

            return input_response::handled();
        }

        // Normal hovered states
        // Brightness square
        else if (_input.nothing_selected() &&
                 util::is_in_rect(mouse_x, mouse_y, square_pos_)) {
            instance()->cursors().set_cursor(cursor::size_all);

            _input.set_hovered(this, colorpicker_selected_ids::selected_square);

            return input_response::handled();
        }

        // Hue bar
        else if (_input.nothing_selected() && 
                 util::is_in_rect(mouse_x, mouse_y, hue_pos_)) {
            instance()->cursors().set_cursor(cursor::size_ns);

            _input.set_hovered(this, colorpicker_selected_ids::selected_hue);

            return input_response::handled();
        }

        // Alpha bar
        else if (has_alpha_ &&
                 _input.nothing_selected() &&
                 util::is_in_rect(mouse_x, mouse_y, alpha_pos_)) {
            instance()->cursors().set_cursor(cursor::size_ns);

            _input.set_hovered(this, colorpicker_selected_ids::selected_alpha);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (_input.is_range_selected(this, 
            colorpicker_selected_ids::selected_square, 
            colorpicker_selected_ids::selected_alpha)) {
            _input.clear_selected();

            return input_response::handled();
        }
        else if (_input.is_selected(this, colorpicker_selected_ids::selected_copy)) {
            _input.clear_selected();

            if (util::is_in_quad(mouse_x, mouse_y, copy_pos_)) {
                // copy text
                std::wstring text;
                std::u32string buffer;
                buffer = text_field_->get_string();
                for (const auto c : buffer) {
                    wchar_t buf[4];
                    std::uint32_t l = r2::unicode::put_char_to_array(c, buf);
                    if (l == 0u)
                        continue;
                    text.append(&buf[0], &buf[0] + l);
                }

                if (!clipboard::copy_to_clipboard(text)) {
                    instance()->notifications().create_error()
                        << xstr("failed to copy color");
                }
            }

            return input_response::handled();
        }
    }

    return should_block_input(input);
}

void colorpicker_overlay::on_scale_changed()
{
    overlay::on_scale_changed();

    text_field_->on_scale_changed();
}

void colorpicker_overlay::update_brightness_square()
{
    square_pos_ = r2::rectf{
        last_pos_.x + spacing_,
        last_pos_.y + spacing_,
        brightness_square_size_, brightness_square_size_
    };
}

void colorpicker_overlay::render_brightness_square()
{
    const float animation = instance()->alpha() * alpha_;

    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const r2::vec2 square_min = r2::vec2(
        square_pos_.x, square_pos_.y
    );
    const r2::vec2 square_max = square_min + r2::vec2(
        square_pos_.w, square_pos_.h
    );

    renderer.add_rect_filled_multicolor(
        square_min, square_max,
        r2::color::white().alpha(animation), hue_.opague().alpha(animation),
        hue_.opague().alpha(animation), r2::color::white().alpha(animation)
    );

    renderer.add_rect_filled_multicolor(
        square_min, square_max,
        r2::color::black().transparent(), r2::color::black().transparent(),
        r2::color::black().alpha(animation), r2::color::black().alpha(animation)
    );

    const r2::vec2 circle_pos = r2::vec2(
        square_min.x + square_pos_.w * hsv_.s,
        square_min.y + square_pos_.h * (1.f - hsv_.v)
    );

    const float radius = style.border_size.get(instance()->scale()) * 3.f;

    renderer.add_rect_filled(
        circle_pos - r2::vec2(radius),
        circle_pos + r2::vec2(radius),
        style.highlight().alpha(animation),
        radius /* rounding */
    );
}

void colorpicker_overlay::update_hue_bar()
{
    hue_pos_ = r2::rectf(
        last_pos_.x + spacing_ + brightness_square_size_ + spacing_,
        last_pos_.y + spacing_,
        bar_width_,
        brightness_square_size_
    );
}

void colorpicker_overlay::render_hue_bar()
{
    static constexpr const r2::color colors[] = {
        r2::color::red(),
        r2::color::yellow(),
        r2::color::green(),
        r2::color::cyan(),
        r2::color::blue(),
        r2::color::purple(),
        r2::color::red()
    };

    const float animation = instance()->alpha() * alpha_;

    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    constexpr int kColorCount = static_cast<int>((sizeof(colors) / sizeof(colors[0]))) - 1;
    const float step = hue_pos_.h / static_cast<float>(kColorCount);
    for (int i = 0; i < kColorCount; ++i) {
        renderer.add_rect_filled_multicolor(
            r2::vec2(hue_pos_.x,
                hue_pos_.y + i * step),
            r2::vec2(hue_pos_.x + bar_width_,
                hue_pos_.y + (i + 1) * step),
            colors[i].alpha(animation), colors[i].alpha(animation),
            colors[i + 1].alpha(animation), colors[i + 1].alpha(animation)
        );
    }

    const float border_size = style.border_size.get(instance()->scale()) * 2.f;

    renderer.add_rect_filled(
        r2::vec2(hue_pos_.x,
            hue_pos_.y + (hue_pos_.h - border_size) * hsv_.h),
        r2::vec2(hue_pos_.x + bar_width_,
            hue_pos_.y + (hue_pos_.h - border_size) * hsv_.h + border_size),
        style.highlight().alpha(animation)
    );
}

void colorpicker_overlay::update_alpha_bar()
{
    alpha_pos_ = r2::rectf(
        last_pos_.x + spacing_ + brightness_square_size_ + spacing_ + bar_width_ + spacing_,
        last_pos_.y + spacing_,
        bar_width_,
        brightness_square_size_
    );
}

void colorpicker_overlay::render_alpha_bar()
{
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();
    const float animation = instance()->alpha() * alpha_;

    const float step = std::round(alpha_pos_.w * 0.5f);

    bool color_toggle = false;
    const r2::color dark_col = r2::color(0.4f, 0.4f, 0.4f).alpha(animation);
    const r2::color bright_col = r2::color(0.9f, 0.9f, 0.9f).alpha(animation);
    for (float y = 0.f; y < brightness_square_size_; y += step) {
        float height = step;
        if (y + step > brightness_square_size_) // Clamp, so we dont need clip rect
            height = brightness_square_size_ - y;

        renderer.prim_rect(
            r2::vec2(alpha_pos_.x, alpha_pos_.y + y),
            r2::vec2(alpha_pos_.x + step, alpha_pos_.y + y + height),
            color_toggle ? bright_col : dark_col
        );

        renderer.prim_rect(
            r2::vec2(alpha_pos_.x + step, alpha_pos_.y + y),
            r2::vec2(alpha_pos_.x + step * 2.f, alpha_pos_.y + y + height),
            !color_toggle ? bright_col : dark_col
        );

        color_toggle = !color_toggle;
    }

    renderer.add_rect_filled_multicolor(
        r2::vec2(alpha_pos_.x, alpha_pos_.y),
        r2::vec2(alpha_pos_.x + alpha_pos_.w, alpha_pos_.y + alpha_pos_.h),
        color_->opague().alpha(animation),
        color_->opague().alpha(animation),
        r2::color::black().transparent(),
        r2::color::black().transparent()
    );

    const float border_size = style.border_size.get(instance()->scale()) * 2.f;

    renderer.add_rect_filled(
        r2::vec2(alpha_pos_.x,
            alpha_pos_.y + (alpha_pos_.h - border_size) * (1.f - hsv_.a)),
        r2::vec2(alpha_pos_.x + bar_width_,
            alpha_pos_.y + (alpha_pos_.h - border_size) * (1.f - hsv_.a) + border_size),
        style.highlight().alpha(animation)
    );
}

void colorpicker_overlay::update_text_field()
{
    auto render_input = input_owner_->input_get_render_input();
    auto& style = instance()->style();
    auto& util = instance()->util();

    const float text_field_size = text_field_size_.get(instance()->scale());

    field_pos_ = r2::vec4{
        last_pos_.x + spacing_,
        last_pos_.y + spacing_ + brightness_square_size_ + spacing_,
        last_pos_.x + last_pos_.w - spacing_,
        field_pos_.y + text_field_size
    };
    field_pos_.w = field_pos_.y + text_field_size;

    const float spacing = style.spacing->get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float off_spacing = std::round(spacing * 0.5f);

    // copy button
    copy_pos_ = r2::vec4(
        field_pos_.z - text_field_size, field_pos_.y,
        field_pos_.z, field_pos_.w
    );

    copy_animation_selected_ = util.lerp(copy_animation_selected_,
        render_input.is_selected(this, colorpicker_selected_ids::selected_copy)
    );
    copy_animation_hovered_ = util.lerp(copy_animation_hovered_,
        render_input.is_hovered(this, colorpicker_selected_ids::selected_copy)
    );

    const float text_field_spacing = std::round(spacing * 0.5f);
    const float text_spacing = std::round((text_field_size - text_size_small) * 0.5f);

    text_field_->update(
        r2::rectf{
            field_pos_.x + text_field_spacing,
            field_pos_.y + text_spacing,
            copy_pos_.x - off_spacing - field_pos_.x - text_field_spacing * 2.f,
            text_size_small
        },
        render_input,
        false
    );
}

void colorpicker_overlay::render_text_field()
{
    const float animation = instance()->alpha() * alpha_;

    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const float rounding = style.rounding->get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    const float off_spacing = std::round(spacing * 0.5f);

    renderer.add_rect_filled(
        r2::vec2(copy_pos_.x, copy_pos_.y),
        r2::vec2(copy_pos_.z, copy_pos_.w),
        style.group_background().alpha(animation),
        rounding * 0.5f
    );

    const float text_field_size = text_field_size_.get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());

    const float icon_spacing = std::round(spacing * 0.4f);
    const float icon_size = text_field_size - icon_spacing * 2.f;

    if (copy_icon_ == icons::kInvalidHandle) {
        copy_icon_ = instance()->icons().get_or_create_handle(void_resources::copy_png);
    }
    
    const auto* icon = instance()->icons().get_or_create(copy_icon_, icon_size);
    renderer.add_image(
        icon->tex,
        r2::vec2(copy_pos_.x + icon_spacing,
            copy_pos_.y + icon_spacing),
        r2::vec2(copy_pos_.z - icon_spacing,
            copy_pos_.w - icon_spacing),
        style.icon().interp(style.accent(), copy_animation_hovered_ * 0.6f).alpha(
            animation * (0.7f + copy_animation_selected_ * 0.3f)),
        icon->uv_min, icon->uv_max
    );

    renderer.add_rect(
        r2::vec2(copy_pos_.x, copy_pos_.y),
        r2::vec2(copy_pos_.z, copy_pos_.w),
        style.accent2().interp(style.accent(), copy_animation_selected_).alpha(animation),
        border_size,
        rounding * 0.5f
    );

    // text field
    renderer.add_rect_filled(
        r2::vec2(field_pos_.x, field_pos_.y),
        r2::vec2(copy_pos_.x - off_spacing, field_pos_.w),
        style.group_background().alpha(animation),
        rounding * 0.5f
    );

    const r2::color text_color = style.accent().interp(style.text_accent(),
        text_field_->animation_selected()
    );

    text_field_->render(
        animation,
        text_color
    );

    renderer.add_rect(
        r2::vec2(field_pos_.x, field_pos_.y),
        r2::vec2(copy_pos_.x - off_spacing, field_pos_.w),
        style.accent2().interp(style.accent(), text_field_->animation_selected()).alpha(animation),
        border_size,
        rounding * 0.5f
    );
}

void colorpicker_overlay::load_color()
{
    *color_ = color::color_convert_hsv_to_rgb(hsv_);
    if (!has_alpha_) {
        color_->a = 1.f;
    }

    last_color_ = *color_;

    load_hue();
}

void colorpicker_overlay::load_hue()
{
    hue_ = color::color_convert_hsv_to_rgb(
        color::hsv(hsv_.h, 1.f, 1.f, 1.f)
    );
}

void colorpicker_overlay::load_hsv()
{
    float old_hue = hsv_.h;
    hsv_ = color::color_convert_rgb_to_hsv(*color_);
    if (hsv_.s == 0.f)
        hsv_.h = old_hue;

    hsv_.a = color_->a;
}

void colorpicker_overlay::on_stop_typing(const std::u32string& s)
{
    r2::color t;
    if (string_to_color(s, t, has_alpha_))
        *color_ = t;
    else {
        // dont change color, restore text
        std::u32string text;
        color_to_string(*color_, text, has_alpha_);
        text_field_->set_string(text);
    }
}

void colorpicker_overlay::color_to_string(const r2::color& c, std::u32string& out, bool has_alpha)
{
    static constexpr xstr kHexChars = "0123456789ABCDEF";
    std::uint8_t r = std::min((std::uint8_t)255, (std::uint8_t)std::round(c.r * 255.f));
    std::uint8_t g = std::min((std::uint8_t)255, (std::uint8_t)std::round(c.g * 255.f));
    std::uint8_t b = std::min((std::uint8_t)255, (std::uint8_t)std::round(c.b * 255.f));
    std::uint8_t a = std::min((std::uint8_t)255, (std::uint8_t)std::round(c.a * 255.f));

    auto length = has_alpha ? 8 : 6;
    out.resize(length);

    out[0] = kHexChars[(r >> 4) & 0x0F];
    out[1] = kHexChars[r & 0x0F];

    out[2] = kHexChars[(g >> 4) & 0x0F];
    out[3] = kHexChars[g & 0x0F];

    out[4] = kHexChars[(b >> 4) & 0x0F];
    out[5] = kHexChars[b & 0x0F];

    if (has_alpha) {
        out[6] = kHexChars[(a >> 4) & 0x0F];
        out[7] = kHexChars[a & 0x0F];
    }
}

inline static bool get_from_hex(char c, std::uint8_t& out) {
    if (c >= 'A' && c <= 'F')
        out = c - 'A' + 10;
    else if (c >= '0' && c <= '9')
        out = c - '0';
    else
        return false;

    return true;
}

bool colorpicker_overlay::string_to_color(const std::u32string& s, r2::color& out, bool has_alpha, bool allow_no_alpha)
{
    std::array<std::uint8_t, 4u> values = { 0xFFu, 0xFFu, 0xFFu, 0xFFu };

    constexpr std::size_t kDefaultLength = 8u;
    constexpr std::size_t kNoAlphaLength = kDefaultLength - 2u;

    if (s.length() != kNoAlphaLength) {
        if (!has_alpha ||
            s.length() != kDefaultLength)
            return false;
    }
    else if (has_alpha && !allow_no_alpha)
        return false;

    for (std::size_t i = 0; i < s.length() / 2; i ++) {
        const char left = static_cast<char>(std::toupper(s[i * 2]));
        const char right = static_cast<char>(std::toupper(s[i * 2 + 1]));

        std::uint8_t left_num = 0;
        std::uint8_t right_num = 0;

        if (!get_from_hex(left, left_num) ||
            !get_from_hex(right, right_num))
            return false;

        values[i] = left_num * 0x10 + right_num;
    }

    out = r2::color(
        values[0],
        values[1],
        values[2],
        values[3]
    );

    return true;
}

void_end_