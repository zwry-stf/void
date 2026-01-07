#include "config_drawable.h"
#include <void/void.h>
#include <config/config.h>


void_begin_

enum class drawable_selected_ids : std::int32_t {
    selected_main,
    selected_load,
    selected_save,
    selected_delete,
    selected_max
};

config_drawable::config_drawable(void_* instance, input_owner* input_owner, _config* config_instance, 
                                 const std::wstring& name, std::string&& modified_time)
    : vobj(instance),
      input_receiver(input_owner,
          std::to_underlying(drawable_selected_ids::selected_max)),
      config_instance_(config_instance),
      name_(name),
      modified_time_(std::move(modified_time))
{
    xstr x_config_extension;
    x_config_extension.append(_config::kConfigExtension.data());

    text_field_ = std::make_unique<textfield>(
        instance,
        input_owner,
        textfield_type::text,
        textfield_flags::stop_on_return | textfield_flags::faded_text,
        xstr(), /* default text */
        x_config_extension,
        60 /* max length */
    );
    text_field_->set_stop_callback(
        [this](const std::u32string& s) -> void {
            this->on_stop_typing(s);
        }
    );
    update_textfield();
    generate_name_lowercase();
}

void config_drawable::update(float x, float y, float w, const render_input& input, bool selected, bool occluded)
{
    was_occluded_ = occluded;

    auto& style    = instance()->style();
    auto& util     = instance()->util();

    last_pos_.x = x;
    last_pos_.y = y;
    last_pos_.w = w;
    last_pos_.h = style.config_height.get(instance()->scale());

    // animations
    animation_selected_ = util.lerp(
        animation_selected_, 
        selected
    );
    animation_hovered_  = util.lerp(
        animation_hovered_, 
        input.is_hovered(this), 
        1.3f
    );

    const float border_size = style.border_size.get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());

    // buttons
    const float button_size = style.drawable_button_size.get(instance()->scale());

    const float outer_spacing = std::round(last_pos_.h * 0.1f);

    const float delete_button_size = std::round(button_size * 0.5f);

    const float border_offset = border_size * 3.f;
    const float offset = std::round(
        rounding - rounding / std::sqrt(2.f)
    );

    update_button(
        void_resources::delete_png,
        r2::vec2{
            x + w - delete_button_size - border_offset - offset,
            y + border_offset + offset
        },
        delete_button_size,
        button_delete_,
        input,
        drawable_selected_ids::selected_delete
    );
    update_button(
        void_resources::save_png,
        r2::vec2{
            button_delete_.pos.x - button_size - border_size,
            y + outer_spacing
        },
        button_size,
        button_save_, 
        input,
        drawable_selected_ids::selected_save
    );
    update_button(
        void_resources::load_png,
        r2::vec2{
            button_delete_.pos.x - button_size - border_size,
            y + last_pos_.h - outer_spacing - button_size
        },
        button_size,
        button_load_,
        input,
        drawable_selected_ids::selected_load
    );

    // text
    const float text_size_large = style.text_size_large.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    const float text_spacing = std::round((last_pos_.h - text_size_large - text_size_small) * (1 / 3.f));

    instance()->fonts().bind_font_large();

    text_field_->update(
        r2::rectf{
            x + spacing,
            y + text_spacing,
            std::max(
            w - button_size - spacing - delete_button_size -
            border_offset - offset - button_size - border_size,
            0.f),
        text_size_large
        },
        input,
        occluded
    );
}

void config_drawable::render(float alpha)
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    const float border_size = style.border_size.get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());

    // background
    renderer.add_rect_filled(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h),
        style.group_background().alpha(0.8f * alpha),
        rounding
    );

    // border
    renderer.add_rect(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h),
        style.border().interp(style.accent2(), animation_selected_ * 0.7f).alpha(
            0.7f * alpha + animation_selected_ * 0.3f),
        border_size,
        rounding
    );

    // buttons
    render_button(button_load_, alpha);
    render_button(button_save_, alpha);
    render_button(button_delete_, alpha);

    // text
    const float text_size_large = style.text_size_large.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    const float text_spacing = std::round((last_pos_.h - text_size_large - text_size_small) * (1 / 3.f));

    const auto text_color = style.text().interp(
        style.text_accent(),
        0.4f + text_field_->animation_selected() * 0.6f
    );

    instance()->fonts().bind_font_large();

    text_field_->render(alpha, text_color);

    instance()->fonts().bind_font_small();

    // date
    constexpr xstr kModifiedTitle = xstr("Modified: ");

    if (!text_width_calculated_) {
        if (renderer.get_text_width_strict(kModifiedTitle, text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(kModifiedTitle);
        text_width_ = std::ceil(text_width_);
    }

    renderer.add_text(
        r2::vec2(
            last_pos_.x + spacing,
            last_pos_.y + last_pos_.h - text_spacing - text_size_small
        ),
        style.text().alpha(0.7f * alpha), 
        kModifiedTitle
    );

    const float text_end = last_pos_.x + spacing + text_width_;

    if (modified_time_.empty()) {
        renderer.add_text(
            r2::vec2(
                text_end,
                last_pos_.y + last_pos_.h - text_spacing - text_size_small
            ),
            style.text().alpha(0.7f * alpha), 
            xstr("<unknown>")
        );
    }
    else {
        renderer.add_text(
            r2::vec2(
                text_end,
                last_pos_.y + last_pos_.h - text_spacing - text_size_small
            ),
            style.text().alpha(0.7f * alpha), 
            modified_time_
        );
    }
}

input_response config_drawable::input(const input_base& input, std::int32_t& selected_config, std::int32_t config_id)
{
    auto res = text_field_->input(input);
    if (res.is_handled())
        return res;

    if (!input.nothing_selected() &&
        !input.is_selected(this))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_move)) {
        if (input.is_selected(this, drawable_selected_ids::selected_load) ||
            (util::is_in_rect(mouse_x, mouse_y, button_load_.pos) &&
                input.nothing_selected())) {
            input.set_hovered(this, drawable_selected_ids::selected_load);
            instance()->cursors().set_cursor(cursor::hand);

            return input_response::handled();
        }

        else if (input.is_selected(this, drawable_selected_ids::selected_save) ||
            (util::is_in_rect(mouse_x, mouse_y, button_save_.pos) && input.nothing_selected())) {
            input.set_hovered(this, drawable_selected_ids::selected_save);
            instance()->cursors().set_cursor(cursor::hand);

            return input_response::handled();
        }

        else if (input.is_selected(this, drawable_selected_ids::selected_delete) ||
            (util::is_in_rect(mouse_x, mouse_y, button_delete_.pos) && input.nothing_selected())) {
            input.set_hovered(this, drawable_selected_ids::selected_delete);
            instance()->cursors().set_cursor(cursor::hand);

            return input_response::handled();
        }

        else if (util::is_in_rect(mouse_x, mouse_y, last_pos_) &&
            input.nothing_selected()) {
            input.set_hovered(this, drawable_selected_ids::selected_main);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, button_load_.pos)) {
            input.set_selected(this, drawable_selected_ids::selected_load);
            selected_config = config_id;

            return input_response::handled();
        }

        else if (util::is_in_rect(mouse_x, mouse_y, button_save_.pos)) {
            input.set_selected(this, drawable_selected_ids::selected_save);
            selected_config = config_id;

            return input_response::handled();
        }

        else if (util::is_in_rect(mouse_x, mouse_y, button_delete_.pos)) {
            input.set_selected(this, drawable_selected_ids::selected_delete);

            return input_response::handled();
        }

        else if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            selected_config = config_id;

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (input.is_selected(this, drawable_selected_ids::selected_load)) {
            if (util::is_in_rect(mouse_x, mouse_y, button_load_.pos)) {
                config_instance_->load(name_);
            }

            input.clear_selected();

            return input_response::handled();
        }

        else if (input.is_selected(this, drawable_selected_ids::selected_save)) {
            if (util::is_in_rect(mouse_x, mouse_y, button_save_.pos)) {
                config_instance_->save(name_, this);
            }

            input.clear_selected();

            return input_response::handled();
        }

        else if (input.is_selected(this, drawable_selected_ids::selected_delete)) {
            if (util::is_in_rect(mouse_x, mouse_y, button_delete_.pos)) {
                config_instance_->remove(name_, this);
            }

            input.clear_selected();

            return input_response::handled();
        }
    }

    return input_response::empty();
}

void config_drawable::on_scale_changed()
{
    text_width_calculated_ = false;
    text_field_->on_scale_changed();
}

void config_drawable::on_activate()
{
    animation_hovered_ = 0.f;
    animation_selected_ = 0.f;

    button_load_.animation_hovered = 0.f;
    button_load_.animation_selected = 0.f;
    button_save_.animation_hovered = 0.f;
    button_save_.animation_selected = 0.f;
    button_delete_.animation_hovered = 0.f;
    button_delete_.animation_selected = 0.f;

    text_field_->on_activate();
}

void config_drawable::search(const std::wstring& text)
{
    if (name_lowercase_.find(text) != std::wstring::npos)
        skipped_ = false;
    else
        skipped_ = true;
}

void config_drawable::on_stop_typing(const std::u32string& text)
{
    // convert to wstring
    std::wstring converted;
    converted.reserve(text.size());
    for (const auto c : text) {
        wchar_t buf[4];
        std::uint32_t l = r2::unicode::put_char_to_array<wchar_t>(c, buf);

        for (std::uint32_t i = 0u; i < l; i++) {
            converted += buf[i];
        }
    }

    if (converted != name_) {
        if (config_instance_->rename(name_, converted)) {
            name_ = converted;
            generate_name_lowercase();
        }
        else {
            update_textfield(); // go back to old name
        }
    }
}

void config_drawable::update_button(int icon_id, const r2::vec2& pos, float button_size, config_button& button,
                                    const render_input& input, drawable_selected_ids id)
{
    auto& util = instance()->util();

    // icon
    if (button.icon_handle == icons::kInvalidHandle) {
        button.icon_handle = instance()->icons().get_or_create_handle(icon_id);
    }

    // animations
    button.animation_hovered = util.lerp(
        button.animation_hovered,
        input.is_hovered(this, id)
    );
    button.animation_selected = util.lerp(
        button.animation_selected,
        input.is_selected(this, id)
    );

    button.pos.x = pos.x;
    button.pos.y = pos.y;
    button.pos.w = button_size;
    button.pos.h = button_size;
}

void config_drawable::render_button(const config_button& button, float alpha)
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    const r2::vec2 min = r2::vec2(button.pos.x, button.pos.y);
    const r2::vec2 max = min + r2::vec2(button.pos.w, button.pos.h);

    // hovered
    renderer.add_shadow_rect_filled(
        min, max,
        style.accent2().alpha(alpha * (button.animation_hovered * 1.f) * 0.5f),
        style.rounding->get(instance()->scale()), 
        button.pos.h * 0.7f
    );

    // icon
    auto icon_color = style.icon().interp(style.accent(),
        button.animation_hovered * 0.3f + button.animation_selected * 0.4f
    );

    if (&button == &button_delete_)
        icon_color = icon_color.alpha(0.4f);

    const auto* icon = instance()->icons().get_or_create(
        button.icon_handle,
        button.pos.h
    );
    renderer.add_image(
        icon->tex,
        min, max,
        icon_color.alpha(alpha),
        icon->uv_min, icon->uv_max
    );
}

void config_drawable::generate_name_lowercase()
{
    name_lowercase_.resize(name_.size());
    std::transform(
        name_.begin(), name_.end(),
        name_lowercase_.begin(),
        [](wchar_t c) -> wchar_t {
            return static_cast<wchar_t>(
                std::tolower(static_cast<int>(c))
            );
        }
    );
}

void config_drawable::update_textfield()
{
    std::u32string buffer;
    const std::uint32_t length = static_cast<std::uint32_t>(name_.length());
    for (std::uint32_t i = 0u; i < length;) {
        auto cp = r2::unicode::get_char_auto(name_, length, i);
        if (cp != r2::unicode::codepoint_invalid) {
            buffer += cp;
        }
    }

    text_field_->set_string(buffer);
}

void_end_