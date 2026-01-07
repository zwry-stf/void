#include "tab_normal.h"
#include <void/void.h>
#include <void/contents/container/child_tab.h>

#include <contents/container/child_tab_normal.h>
#include <config/config_tab.h>
#include <theme/theme_tab.h>


void_begin_

tab_normal::tab_normal(void_* instance, input_owner* input_owner, const xstr& name, input_owner_overlay* overlay_owner)
    : tab(instance, overlay_owner, true),
      input_receiver(input_owner, 1),
      name_(name)
{
    assert(name_.find('\n') == xstr::npos);

    add_child_tab(
        std::make_unique<child_tab_normal>(instance, input_owner, overlay_owner, xstr("Child 1"))
    );
    add_child_tab(
        std::make_unique<child_tab_normal>(instance, input_owner, overlay_owner, xstr("Child 2"))
    );
    add_child_tab(
        std::make_unique<config_tab>(instance, input_owner, overlay_owner, instance->config().get_config_instance(), xstr("Config"))
    );
    add_child_tab(
        std::make_unique<theme_tab>(instance, input_owner, overlay_owner, instance->theme().get_theme_instance(), xstr("Theme"))
    );
}

tab_normal::~tab_normal() = default;

float tab_normal::update(float x, float y, float w, const render_input& input, bool selected)
{
    last_pos_.x = x;
    last_pos_.y = y;
    last_pos_.w = w;
    constexpr sfloat kTabHeight = sfloat(32.f, true);
    last_pos_.h = kTabHeight.get(instance()->scale());

    animation_selected_ = instance()->util().lerp(animation_selected_, selected);
    animation_hovered_ = instance()->util().lerp(animation_hovered_, 
        input.is_hovered(this));

    if (selected)
        update_content(input);

    return last_pos_.h;
}

void tab_normal::render(bool selected)
{
    render_header();

    if (selected)
        render_content();
}

input_response tab_normal::input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id)
{
    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (selected_tab == tab_id) {
        // child tab input
        // selected tab first
        if (selected_child_ >= 0 &&
            selected_child_ < static_cast<std::int32_t>(child_tabs_.size())) {
            auto res = child_tabs_[selected_child_]->input(
                input, 
                selected_child_, /* selected tab */
                selected_child_ /* tab id */
            );
            if (res.is_handled())
                return res;
        }

        for (std::int32_t i = 0; i < static_cast<std::int32_t>(child_tabs_.size()); i++) {
            if (i != selected_child_) /* selected child already handled */ {
                auto res = child_tabs_[i]->input(
                    input,
                    selected_child_,
                    i
                );
                if (res.is_handled())
                    return res;
            }
        }
    }

    // header input
    if (input.event().is_message(message_type::mouse_move) &&
        input.nothing_selected()) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            input.set_hovered(this);
            instance()->cursors().set_cursor(cursor::hand);
            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            if (selected_tab != tab_id) {
                selected_tab = tab_id;
                on_activate();
            }

            return input_response::handled();
        }
    }

    return input_response::empty();
}

void tab_normal::on_activate(bool first)
{
    if (selected_child_ >= 0 &&
        selected_child_ < static_cast<std::int32_t>(child_tabs_.size()))
        child_tabs_[selected_child_]->on_activate(true, first);
}

void tab_normal::on_scale_change()
{
    text_width_calculated_ = false;

    for (auto& child : child_tabs_)
        child->on_scale_change();
}

void tab_normal::add_child_tab(std::unique_ptr<child_tab>&& child_tab)
{
    child_tabs_.push_back(std::move(child_tab));
}

void tab_normal::update_content(const render_input& input)
{
    auto& style = instance()->style();

    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float sidebar_width = style.sidebar_width.get(instance()->scale());

    const auto& menu_pos = instance()->pos();
    float pos_x = menu_pos.x + sidebar_width + spacing * 2.f;

    for (std::size_t i = 0; i < child_tabs_.size(); i++) {
        pos_x += child_tabs_[i]->update(
            pos_x, 
            menu_pos.y,
            selected_child_ == static_cast<std::int32_t>(i),
            input
        );

        pos_x += spacing + border_size + spacing;
    }
}

void tab_normal::render_header()
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    instance()->fonts().bind_font_small();

    if (!text_width_calculated_) {
        if (renderer.get_text_width_strict(name_, text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(name_);
        text_width_ = std::ceil(text_width_);
    }

    const float spacing = style.spacing->get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());

    // background
    const float background_rounding = std::round(rounding * 0.6f);
    renderer.add_rect_filled(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + last_pos_.h),
        style.highlight().alpha(0.08f * animation_hovered_ + 0.1f * animation_selected_),
        background_rounding
    );

    // icon
    const bool has_icon = icon_ != icons::kInvalidHandle;
    if (has_icon) {
        const float icon_offset = std::round(spacing * 0.5f) + border_size;
        const float icon_size = last_pos_.w - icon_offset * 2.f - border_size * 2.f;
        const auto& icon = instance()->icons().get_or_create(icon_, icon_size);
        renderer.add_image(
            icon->tex,
            r2::vec2(last_pos_.x + border_size + icon_offset,
                last_pos_.y + border_size + icon_offset),
            r2::vec2(last_pos_.x + border_size + icon_offset + icon_size,
                last_pos_.y + border_size + icon_offset + icon_size),
            style.icon(),
            icon->uv_min, icon->uv_max
        );
    }

    // text
    renderer.push_clip_rect(
        r2::vec2(last_pos_.x + border_size, last_pos_.y),
        r2::vec2(last_pos_.x + last_pos_.w - border_size, last_pos_.y + last_pos_.h),
        true
    );

    renderer.add_text(
        r2::vec2(last_pos_.x + (has_icon ? last_pos_.h : std::round((last_pos_.w - text_width_) * 0.5f)),
            last_pos_.y + (last_pos_.h - style.text_size_small.get(instance()->scale())) * 0.5f),
        style.text_accent(), 
        name_
    );

    renderer.pop_clip_rect();
}

void tab_normal::render_content()
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    const auto& menu_pos = instance()->pos();

    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float top_bar_height = style.top_bar_height.get(instance()->scale());
    const float text_size_large = style.text_size_large.get(instance()->scale());

    const float fade_width = spacing * 2.f;
    const float fade_end   = instance()->search_pos().x - spacing;
    const float fade_start = fade_end - fade_width;

    for (std::size_t i = 0u; i < child_tabs_.size(); i++) {
        child_tabs_[i]->render(
            animation_selected_, 
            selected_child_ == static_cast<std::int32_t>(i)
        );

        const auto& last_pos = child_tabs_[i]->last_pos();

        const float pos_x = last_pos.x + last_pos.w + spacing;

        // border between child tabs
        if (i < child_tabs_.size() - 1u) {
            const float text_offset = std::floor((top_bar_height - text_size_large) * 0.5f);
            renderer.add_rect_filled_faded(
                r2::vec2(pos_x,
                    menu_pos.y + text_offset),
                r2::vec2(pos_x + border_size,
                    menu_pos.y + top_bar_height - text_offset),
                style.accent().alpha(0.7f), style.accent().transparent(), fade_start, fade_end
            );
        }
    }
}


void_end_