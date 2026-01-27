#include "keybind_list.h"
#include <void/void.h>
#include <contents/input/keybind_manager.h>
#include <void/builder/builder.h>


void_begin_

keybind_list::keybind_list(void_* instance)
    : vobj(instance)
{
    auto mb = instance->get_builder();

    mb.overlay()
        .config("keybinds")
        .make_resizable(false)
        .liquid_glass(false)
        .on_update(
            [this](void_* instance, custom_overlay& overlay) -> void {
                this->on_overlay_update(instance, overlay);
            }
        )
        .on_render(
            [this](void_* instance, custom_overlay& overlay) -> void {
                this->on_overlay_render(instance, overlay);
            }
        )
        .size(130.f, 28.f)
        .pos(0.01f, 0.2f);
}

void keybind_list::on_overlay_update(void_* instance, custom_overlay& overlay)
{
    overlay.toggle_input(instance->options().get<options::option_KeybindList>());
}

void keybind_list::on_overlay_render(void_* instance, custom_overlay& overlay)
{
    auto& renderer = instance->renderer();
    auto& style = instance->style();
    auto& util = instance->util();

    const bool enable_list = instance->options().get<options::option_KeybindList>();
    const float temp_animation = util.lerp2(animation_alpha_, enable_list);
    if (!enable_list &&
        temp_animation < util::g_min_alpha) {
        shown_binds_.clear();
        animation_alpha_ = 0.f;
        animation_ = 0.f;
        return;
    }

    // reset "seen_in_refresh" states
    for (auto& b : shown_binds_) {
        b.seen_in_refresh = false;
    }

    // iterate keybinds
    const auto& binds = instance->input().keybind_manager()->binds();
    for (auto& b : binds) {
        if (!b->value())
            continue;

        if (hide_always_on &&
            b->mode_ref() == static_cast<std::size_t>(keybind_mode::always))
            continue;

        if (hide_hold &&
            b->mode_ref() == static_cast<std::size_t>(keybind_mode::hold))
            continue;

        // place back/set seen true
        shown_bind* sbind = nullptr;
        for (auto& sb : shown_binds_) {
            if (sb.bind == b.get()) {
                sbind = &sb;
                break;
            }
        }

        if (sbind == nullptr) {
            sbind = &shown_binds_.emplace_back();
            sbind->bind = b.get();
        }

        sbind->seen_in_refresh = true;
    }

    // animation
    std::size_t valid_targets = 0u;
    for (auto& sb : shown_binds_) {
        if (sb.seen_in_refresh)
            valid_targets++;
    }

    const bool should_render = enable_list &&
        (valid_targets != 0u || instance->is_open());

    animation_alpha_ = util.lerp2(animation_alpha_, should_render);
    overlay.set_animation(animation_alpha_);

    if (animation_alpha_ < vo::util::g_min_alpha) {
        shown_binds_.clear();
        return;
    }

    // update binds
    for (auto it = shown_binds_.begin(); it != shown_binds_.end();) {
        auto& bind = *it;

        // animation
        bind.animation_alpha = util.lerp2(bind.animation_alpha, bind.seen_in_refresh);

        if (!bind.seen_in_refresh &&
            bind.animation_alpha < util::g_min_alpha) {
            it = shown_binds_.erase(it);
            continue;
        }
        it++;
    }

    create_icons();

    const float text_size_small = style.text_size_small.get(instance->scale());
    const float spacing = style.spacing->get(instance->scale());
    const float rounding = style.rounding->get(instance->scale());
    const float border_size = style.border_size.get(instance->scale());

    const float header_height = std::round(text_size_small * 1.2f);
    draw_header(instance, overlay, header_height);

    const float row_spacing = std::round(spacing * 0.4f);

    const float row_height = std::round(text_size_small * 1.2f);
    const float icon_spacing = std::round(row_height * 0.15f);
    const float icon_size = row_height - icon_spacing * 2.f;
    const float full_size = static_cast<float>(valid_targets) * (row_height + row_spacing) +
        header_height + (valid_targets == 0u ? 0.f : std::round(spacing * 0.5f));
    animation_ = util.lerp(animation_, full_size - header_height);
    overlay.set_size_scaled(
        r2::vec2{
            overlay.get_size().x,
            animation_ + header_height
        }
    );

    const r2::vec2 pos = overlay.get_pos();
    const r2::vec2 size = overlay.get_size();

    overlay.data().background = style.overlay_background();
    overlay.data().border = style.border();

    overlay.data().rounding_top = rounding;
    overlay.data().rounding_bottom = (std::min)(animation_, rounding);

    renderer.push_clip_rect(
        pos + r2::vec2(border_size),
        pos + size - r2::vec2(std::round(row_spacing * 0.5f), border_size)
    );

    const float text_spacing = std::round((row_height - text_size_small) * 0.5f);
    const float start_y = pos.y + header_height + std::round(row_spacing * 0.5f);
    float pos_y = 0.f;

    for (auto& b : shown_binds_) {
        float pos_x = pos.x + std::round(spacing * 0.5f);

        const float animation_fade = (std::min)(
            b.animation_alpha * b.animation_alpha,
            animation_alpha_
        );
        pos_x += (1.f - animation_fade) * row_height * 0.5f;

        if (b.animation_y == -1.f) {
            b.animation_y = pos_y;
        }
        else {
            b.animation_y = util.lerp(b.animation_y, pos_y, 2.5f);
        }

        // icon
        icons::icon_handle icon_handle;
        switch (static_cast<keybind_mode>(b.bind->mode_ref())) {
        case keybind_mode::always:
            icon_handle = always_on_icon_;
            break;
        case keybind_mode::toggle:
            icon_handle = toggle_icon_;
            break;
        case keybind_mode::hold: [[fallthrough]];
        default:
            icon_handle = hold_icon_;
            break;
        }
        auto* tex = instance->icons().get_or_create(icon_handle, icon_size);

        renderer.add_image_rounded(
            tex->tex,
            r2::vec2(pos_x + icon_spacing,
                start_y + b.animation_y + icon_spacing),
            r2::vec2(pos_x + icon_spacing + icon_size,
                start_y + b.animation_y + icon_spacing + icon_size),
            rounding * 0.5f,
            style.icon().alpha(animation_fade),
            tex->uv_min,
            tex->uv_max
        );

        pos_x += icon_size + icon_spacing * 2.f + row_spacing;

        renderer.add_text(
            r2::vec2(
                pos_x, 
                start_y + b.animation_y + text_spacing
            ),
            style.text().alpha(animation_fade),
            b.bind->name()
        );

        if (b.seen_in_refresh) {
            pos_y += row_height + row_spacing;
        }
    }

    renderer.pop_clip_rect();
}

void keybind_list::create_icons()
{
    if (keybind_icon_ == icons::kInvalidHandle) {
        keybind_icon_ = instance()->icons().get_or_create_handle(void_resources::keybind_png);
    }
    if (hold_icon_ == icons::kInvalidHandle) {
        hold_icon_ = instance()->icons().get_or_create_handle(void_resources::keybind_hold_png);
    }
    if (toggle_icon_ == icons::kInvalidHandle) {
        toggle_icon_ = instance()->icons().get_or_create_handle(void_resources::keybind_toggle_png);
    }
    if (always_on_icon_ == icons::kInvalidHandle) {
        always_on_icon_ = instance()->icons().get_or_create_handle(void_resources::keybind_always_on_png);
    }
}

void keybind_list::draw_header(void_* instance, custom_overlay& overlay, float height)
{
    auto& style = instance->style();
    auto& renderer = instance->renderer();

    const auto pos = overlay.get_pos();
    const auto size = overlay.get_size();

    const float icon_spacing = style.border_size.get(instance->scale());
    const float icon_size = height - icon_spacing * 2.f;
    const auto icon_pos = r2::vec2{
            pos.x + std::round(style.spacing->get(instance->scale()) * 0.5f),
            pos.y + icon_spacing
    };

    auto* tex = instance->icons().get_or_create(keybind_icon_, icon_size);
    renderer.add_image(
        tex->tex,
        icon_pos,
        icon_pos + r2::vec2(icon_size),
        style.icon().alpha(animation_alpha_),
        tex->uv_min,
        tex->uv_max
    );

    renderer.add_text(
        r2::vec2{
            icon_pos.x + icon_size + icon_spacing,
            pos.y + std::round((height - style.text_size_small.get(instance->scale())) * 0.5f)
        },
        style.text_accent().alpha(animation_alpha_),
        vo::xstr("Keybinds:")
    );
}

void_end_