#include "menu_options.h"
#include <void/void.h>
#include <contents/overlays/dropdown/dropdown.h>
#include <contents/widgets/dropdown/dropdown_vertical.h>
#include <contents/widgets/toggle/toggle.h>
#include <contents/overlays/childwindow/childwindow.h>
#include <config/config.h>
#include <contents/overlays/keybind_list/keybind_list.h>
#include <contents/widgets/childwindow/childwindow_child.h>


void_begin_

enum class scale_values : std::uint8_t {
    v_auto,
    v_75,
    v_85,
    v_100,
    v_125,
    v_150,
    v_200
};

menu_options::menu_options(void_* instance, input_owner* input_owner,
                           input_owner_overlay* overlay_owner)
    : overlay(instance, overlay_owner, true, true),
      input_receiver(input_owner, 0)
{
    /// scale
    static constexpr xstr kDropdownOptions[] = {
        "Auto", "75%", "85%", "100%", "125%", "150%", "200%"
    };

    auto overlay_id = create_overlay(
        std::make_unique<dropdown_overlay>(
            instance, input_owner,
            this,
            std::unique_ptr<vo::list_options>(
                vo::list_options::create_constant(kDropdownOptions)
            ),
            &selected_scale_
        )
    );

    instance->config().add_module(
        std::make_unique<default_config_module<std::size_t>>(
            vo::xstr("menu_scale"),
            &selected_scale_
        )
    );

    widgets_.emplace_back(
        std::make_unique<dropdown_vertical>(
            instance, input_owner,
            this,
            overlay_id,
            xstr("Scale")
        )
    );

    /// keybind list
    auto cw_overlay_id = create_overlay(
        std::make_unique<childwindow>(
            instance, instance,
            this,
            xstr("Keybind list")
        )
    );

    auto* child_window = get_overlay<childwindow>(cw_overlay_id);

    child_window->add_widget(
        std::make_unique<toggle>(
            instance, instance,
            xstr("Hide 'always on'"),
            &instance->get_keybind_list()->hide_always_on
        )
    );
    instance->config().add_module(
        std::make_unique<default_config_module<bool>>(
            vo::xstr("hide_always_on"),
            &instance->get_keybind_list()->hide_always_on
        )
    );

    child_window->add_widget(
        std::make_unique<toggle>(
            instance, instance,
            xstr("Hide 'hold'"),
            &instance->get_keybind_list()->hide_hold
        )
    );
    instance->config().add_module(
        std::make_unique<default_config_module<bool>>(
            vo::xstr("hide_hold"),
            &instance->get_keybind_list()->hide_hold
        )
    );

    widgets_.emplace_back(
        std::make_unique<toggle>(
            instance, input_owner,
            xstr("Keybind list"),
            &instance->options().get<options::option_KeybindList>()
        )
    );
    instance->config().add_module(
        std::make_unique<default_config_module<bool>>(
            vo::xstr("enable_keybind_list"),
            &instance->options().get<options::option_KeybindList>()
        )
    );

    widgets_.back()->add_child(
        std::make_unique<childwindow_child>(
            instance, instance,
            this,
            cw_overlay_id
        )
    );

    /// alow movement
    widgets_.emplace_back(
        std::make_unique<toggle>(
            instance, input_owner,
            xstr("Allow movement"),
            &instance->options().get<options::option_AllowKeyInput>()
        )
    );

    instance->config().add_module(
        std::make_unique<default_config_module<bool>>(
            vo::xstr("allow_movement"),
            &instance->options().get<options::option_AllowKeyInput>()
        )
    );
}

void menu_options::update(const overlay_render_input& input)
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
    auto& account = instance()->account();
    const auto display_size = renderer.get_render_size();

    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    last_pos_.x = account.last_pos_.x;
    last_pos_.y = account.last_pos_.y;
    last_pos_.w = sidebar_width;

    header_height_ = text_size_small + border_size * 4.f;

    account_offset_ = r2::rectf{
        0.f,
        header_height_,
        last_pos_.w,
        account.last_pos_.h
    };

    const float widget_spacing = std::round(spacing * 0.75f);
    const float top_spacing = spacing;
    const float side_spacing = spacing;

    float full_height = header_height_ + +account_offset_.h + top_spacing * 2.f;
    bool widget_found = false;
    const float pos_x = last_pos_.x + side_spacing;
    float pos_y = last_pos_.y + header_height_ + account_offset_.h + top_spacing;
    auto render_input = input_owner_->input_get_render_input();

    for (auto& widget : widgets_) {
        if (widget->is_visible()) [[likely]] {
            widget->update(
                pos_x,
                pos_y,
                last_pos_.w - side_spacing * 2.f,
                render_input,
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

    // move up
    last_pos_.y -= last_pos_.h;

    pos_y = last_pos_.y + header_height_ + account_offset_.h + top_spacing;
    for (auto& widget : widgets_) {
        if (widget->is_visible()) [[likely]] {
            widget->set_pos(
                r2::vec2{
                    pos_x,
                    pos_y
                }
            );

            const float space = widget->get_height() + widget_spacing;
            pos_y += space;
        }
    }

    // scale
    if (selected_scale_ != last_selected_scale_) {
        last_selected_scale_ = selected_scale_;

        float scale = instance()->scale();
        switch (static_cast<scale_values>(selected_scale_)) {
        case scale_values::v_auto:
            scale = instance()->calculate_scale();
            break;
        case scale_values::v_75:
            scale = 0.75f;
            break;
        case scale_values::v_85:
            scale = 0.85f;
            break;
        case scale_values::v_100:
            scale = 1.f;
            break;
        case scale_values::v_125:
            scale = 1.25f;
            break;
        case scale_values::v_150:
            scale = 1.5f;
            break;
        case scale_values::v_200:
            scale = 2.f;
            break;
        default:
            break;
        }

        instance()->set_scale(scale);

        set_opened(-1);
    }
}

void menu_options::render()
{
    const float animation = instance()->alpha() * alpha_;
    if (animation < util::g_min_alpha)
        return;

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto& account = instance()->account();

    renderer.push_clip_rect(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h),
        false
    );

    const float rounding = style.rounding->get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    instance()->background().add_immediate_overlay(
        last_pos_,
        animation
    );

    // icon
    if (icon_ == icons::kInvalidHandle) {
        icon_ = instance()->icons().get_or_create_handle(void_resources::sparkles_png);
    }

    const float icon_spacing = border_size;
    const float icon_size = header_height_ - icon_spacing * 2.f;
    const auto icon_pos = r2::vec2{
        last_pos_.x + std::max(icon_spacing, std::round(rounding * 0.5f)),
        last_pos_.y + icon_spacing
    };

    auto* tex = instance()->icons().get_or_create(icon_, icon_size);
    renderer.add_image(
        tex->tex,
        icon_pos,
        icon_pos + r2::vec2(icon_size),
        style.icon().alpha(animation),
        tex->uv_min,
        tex->uv_max
    );

    // title
    renderer.add_text(
        r2::vec2(icon_pos.x + icon_size + icon_spacing + std::round(spacing * 0.5f),
            last_pos_.y + std::round((header_height_ - text_size_small) * 0.5f)),
        style.text().alpha(animation),
        xstr("Options:")
    );

    renderer.add_rect_filled(
        r2::vec2(last_pos_.x + border_size, last_pos_.y + header_height_),
        r2::vec2(last_pos_.x + last_pos_.w - border_size, last_pos_.y + header_height_ + border_size),
        style.border().alpha(animation)
    );

    // account
    const float side_spacing = std::round(spacing * 0.7f);
    const float account_icon_size = account.last_pos_.h - side_spacing * 2.f;

    const auto* icon = instance()->icons().get_or_create(account.icon_, account_icon_size);

    // icon
    renderer.add_image_rounded(
        icon->tex,
        r2::vec2(last_pos_.x + account_offset_.x + side_spacing,
            last_pos_.y + account_offset_.y + side_spacing),
        r2::vec2(last_pos_.x + account_offset_.x + side_spacing + account_icon_size,
            last_pos_.y + account_offset_.y + side_spacing + account_icon_size),
        account_icon_size * 0.5f,
        r2::color::white().alpha(animation),
        icon->uv_min, icon->uv_max
    );

    renderer.add_rect(
        r2::vec2(last_pos_.x + account_offset_.x + side_spacing,
            last_pos_.y + account_offset_.y + side_spacing),
        r2::vec2(last_pos_.x + account_offset_.x + side_spacing + account_icon_size,
            last_pos_.y + account_offset_.y + side_spacing + account_icon_size),
        style.accent().alpha(animation),
        border_size,
        account_icon_size * 0.5f
    );

    // display name
    const float text_x = last_pos_.x + account_offset_.x + side_spacing + account_icon_size + std::round(spacing * 0.5f);
    const float text_offset_y = std::round((account_icon_size * 0.5f - text_size_small) * 0.5f);

    const float fade_width = spacing * 2.f;
    const float fade_end = last_pos_.x + account_offset_.x + last_pos_.w - border_size;
    const float fade_start = fade_end - fade_width;

    renderer.add_text_faded(
        r2::vec2(text_x, 
            last_pos_.y + account_offset_.y + side_spacing + text_offset_y),
        style.text().alpha(animation),
        style.text().transparent(),
        fade_start, fade_end,
        account.display_name_
    );

    // expiration date
    renderer.add_text_faded(
        r2::vec2(text_x,
            last_pos_.y + account_offset_.y + account_offset_.h - side_spacing - text_offset_y - text_size_small),
        style.grey().alpha(animation),
        style.grey().transparent(),
        fade_start, fade_end,
        account.display_text_
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

input_response menu_options::input(const overlay_input& input)
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

    // widgets
    input_base _input = input_owner_->input_get_input(input.event());

    for (auto& widget : widgets_) {
        if (widget->is_visible()) {
            auto res = widget->input(_input);
            if (res.is_handled())
                return res;
        }
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

    return should_block_input(input);
}

void menu_options::render_overlays()
{
    auto render_input = input_get_overlay_render_input();

    for (auto& overlay : overlays_) {
        overlay->update(render_input);
        overlay->render();
    }
}

void menu_options::on_scale_changed()
{
    overlay::on_scale_changed();

    for (auto& w : widgets_)
        w->on_scale_change();
    for (auto& o : overlays_)
        o->on_scale_changed();
}

void_end_