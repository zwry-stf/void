#include "theme_drawable.h"
#include <void/void.h>
#include <theme/theme.h>


void_begin_

enum class drawable_selected_ids : std::int32_t {
    selected_main,
    selected_load,
    selected_save,
    selected_delete,
    selected_max
};

theme_drawable::theme_drawable(void_* instance, input_owner* input_owner, _theme* theme_instance, 
	                           const std::wstring& name)
    : vobj(instance),
      input_receiver(input_owner,
          std::to_underlying(drawable_selected_ids::selected_max)),
      theme_instance_(theme_instance),
      name_(name)
{
    xstr x_theme_extension;
    x_theme_extension.append(_theme::kThemeExtension.data());

    text_field_ = std::make_unique<textfield>(
        instance,
        input_owner,
        textfield_type::text,
        textfield_flags::stop_on_return | textfield_flags::faded_text,
        xstr(), /* default text */
        x_theme_extension,
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

void theme_drawable::update(float x, float y, float w, const render_input& input, bool selected, bool occluded)
{
    was_occluded_ = occluded;

    auto& style    = instance()->style();
    auto& util     = instance()->util();

    last_pos_.x = x;
    last_pos_.y = y;
    last_pos_.w = w;
    last_pos_.h = style.theme_height.get(instance()->scale());

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
    const float spacing = style.spacing->get(instance()->scale());

    // buttons
    const float button_size = style.drawable_button_size.get(instance()->scale());

    const float delete_button_size = std::round(button_size * 0.5f);

    const float border_offset = border_size * 3.f;
    const float offset = std::round(
        rounding - rounding / std::sqrt(2.f)
    );

    const float side_spacing = spacing * 2.f;
    const float button_spacing = spacing;

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
            x + w - side_spacing - button_size,
            y + last_pos_.h - side_spacing - button_size
        },
        button_size,
        button_save_,
        input,
        drawable_selected_ids::selected_save
    );
    update_button(
        void_resources::load_png,
        r2::vec2{
            button_save_.pos.x - button_spacing - button_size,
            button_save_.pos.y
        },
        button_size,
        button_load_,
        input,
        drawable_selected_ids::selected_load
    );

    // text
    const float text_size_large = style.text_size_large.get(instance()->scale());

    instance()->fonts().bind_font_large();

    const float text_x = x + side_spacing;
    text_field_->update(
        r2::rectf{
            text_x,
            y + last_pos_.h - side_spacing -
                std::round((button_size + text_size_large) * 0.5f),
            std::max(
                button_load_.pos.x - text_x - border_size,
                0.f
            ),
            text_size_large
        },
        input,
        occluded
    );
}

void theme_drawable::render(float alpha)
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    const float border_size = style.border_size.get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());

    const float outer_offset = std::round(spacing * 0.125f) * animation_selected_;
    const float inner_offset = std::round(spacing * 1.5f);

    // background
    renderer.add_rect_filled(
        r2::vec2(last_pos_.x - outer_offset,
            last_pos_.y - outer_offset),
        r2::vec2(last_pos_.x + last_pos_.w + outer_offset, 
            last_pos_.y + last_pos_.h + outer_offset),
        style.group_background().alpha(0.8f * alpha),
        rounding
    );

    // border
    renderer.add_rect(
        r2::vec2(last_pos_.x - outer_offset,
            last_pos_.y - outer_offset),
        r2::vec2(last_pos_.x + last_pos_.w + outer_offset,
            last_pos_.y + last_pos_.h + outer_offset),
        style.border().interp(style.accent2(), animation_selected_ * 0.7f).alpha(
            0.7f * alpha + animation_selected_ * 0.3f),
        border_size,
        rounding
    );

    // colors
    const r2::vec2 colors_min = r2::vec2(
        last_pos_.x + inner_offset,
        last_pos_.y + inner_offset
    );
    const r2::vec2 colors_max = r2::vec2(
        last_pos_.x + last_pos_.w - inner_offset,
        last_pos_.y + last_pos_.h - inner_offset
    );

    renderer.add_rect_filled(
        colors_min, colors_max,
        background_color_.alpha(alpha)
    );

    renderer.path_add_point(colors_min);
    renderer.path_add_point(
        r2::vec2(colors_min.x, colors_max.y)
    );
    renderer.path_add_point(
        r2::vec2(colors_max.x, colors_min.y)
    );

    const auto vtx_index = renderer.vertex_ptr();
    renderer.path_fill_convex(r2::color::white());

    const r2::color accent2 = accent2_color_.alpha(alpha);
    renderer.shade_vertices_col(
        vtx_index,
        renderer.vertex_ptr(),
        colors_min,
        colors_max,
        accent_color_.alpha(alpha), // top left
        accent2, // top right
        accent2, // bottom right
        accent2 // bottom left
    );

    // buttons
    render_button(button_load_, alpha);
    render_button(button_save_, alpha);
    render_button(button_delete_, alpha);

    // text
    const auto text_color = style.text().interp(
        style.text_accent(),
        0.4f + text_field_->animation_selected() * 0.6f
    );

    instance()->fonts().bind_font_large();

    text_field_->render(alpha, text_color);
}

input_response theme_drawable::input(const input_base& input, std::int32_t& selected_theme, std::int32_t theme_id)
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
            selected_theme = theme_id;

            return input_response::handled();
        }

        else if (util::is_in_rect(mouse_x, mouse_y, button_save_.pos)) {
            input.set_selected(this, drawable_selected_ids::selected_save);
            selected_theme = theme_id;

            return input_response::handled();
        }

        else if (util::is_in_rect(mouse_x, mouse_y, button_delete_.pos)) {
            input.set_selected(this, drawable_selected_ids::selected_delete);

            return input_response::handled();
        }

        else if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            selected_theme = theme_id;

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (input.is_selected(this, drawable_selected_ids::selected_load)) {
            if (util::is_in_rect(mouse_x, mouse_y, button_load_.pos)) {
                theme_instance_->load(name_);
            }

            input.clear_selected();

            return input_response::handled();
        }

        else if (input.is_selected(this, drawable_selected_ids::selected_save)) {
            if (util::is_in_rect(mouse_x, mouse_y, button_save_.pos)) {
                theme_instance_->save(name_, this);
            }

            input.clear_selected();

            return input_response::handled();
        }

        else if (input.is_selected(this, drawable_selected_ids::selected_delete)) {
            if (util::is_in_rect(mouse_x, mouse_y, button_delete_.pos)) {
                theme_instance_->remove(name_, this);
            }

            input.clear_selected();

            return input_response::handled();
        }
    }

    return input_response::empty();
}

void theme_drawable::on_scale_changed()
{
    text_field_->on_scale_changed();
}

void theme_drawable::on_activate()
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

void theme_drawable::search(const std::wstring& text)
{
    if (name_lowercase_.find(text) != std::wstring::npos)
        skipped_ = false;
    else
        skipped_ = true;
}

void theme_drawable::on_stop_typing(const std::u32string& text)
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
        if (theme_instance_->rename(name_, converted)) {
            name_ = converted;
            generate_name_lowercase();
        }
        else {
            update_textfield(); // go back to old name
        }
    }
}

void theme_drawable::update_button(int icon_id, const r2::vec2& pos, float button_size, theme_button& button,
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

void theme_drawable::render_button(const theme_button& button, float alpha)
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

void theme_drawable::generate_name_lowercase()
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

void theme_drawable::update_textfield()
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