#include "sidebar.h"
#include <void/void.h>


void_begin_

void sidebar::render(const render_input& input)
{
    auto& style    = instance()->style();
    auto& renderer = instance()->renderer();
    const auto& menu_pos = instance()->pos();

    // icon
    const float spacing = style.spacing->get(instance()->scale());
    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float top_offset = spacing * 3.f;
    constexpr sfloat kIconSize = sfloat(64.f);
    const float icon_size_d = kIconSize.get(instance()->scale());
    float icon_size = std::min(icon_size_d, sidebar_width - top_offset * 2.f);
    const float icon_side_offset = std::round((sidebar_width - icon_size) * 0.5f);
    icon_size = sidebar_width - icon_side_offset * 2.f;

    if (icon_handle_ == icons::kInvalidHandle) {
        icon_handle_ = instance()->icons().get_or_create_handle(void_resources::icon_128x128_png);
    }
    const auto* icon = instance()->icons().get_or_create(icon_handle_, icon_size);
    renderer.add_image(
        icon->tex,
        r2::vec2(menu_pos.x + icon_side_offset, 
            menu_pos.y + top_offset),
        r2::vec2(menu_pos.x + icon_side_offset + icon_size,
            menu_pos.y + top_offset + icon_size),
        style.icon(),
        icon->uv_min, icon->uv_max
    );

    // tabs
    const float side_offset = std::round(spacing * 0.7f);
    const float tab_spacing = std::round(spacing * 0.5f);
    const float pos_x = menu_pos.x + border_size + side_offset;

    float pos_y = menu_pos.y + top_offset * 2.f + icon_size;
    for (std::size_t i = 0u; i < tabs_.size(); i++) {
        const float h = tabs_[i]->update(
            pos_x, pos_y,
            sidebar_width - side_offset * 2.f - border_size * 2.f,
            input, 
            selected_tab_ == static_cast<std::int32_t>(i)
        );
        pos_y += h + tab_spacing;
    }

    for (std::size_t i = 0u; i < tabs_.size(); i++) {
        tabs_[i]->render(selected_tab_ == static_cast<std::int32_t>(i));
    }
}

input_response sidebar::input(const input_base& input)
{
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(tabs_.size()); i++) {
        auto res = tabs_[i]->input(input, selected_tab_, i);
        if (res.is_handled())
            return res;
    }

    return input_response::empty();
}

void sidebar::on_activate()
{
    if (selected_tab_ >= 0 &&
        selected_tab_ < static_cast<std::int32_t>(tabs_.size()))
        tabs_[selected_tab_]->on_activate(true);
}

void sidebar::on_scale_change()
{
    for (auto& t : tabs_)
        t->on_scale_change();
}

tab* sidebar::get_active_tab() const
{
    if (selected_tab_ >= 0 &&
        selected_tab_ < static_cast<std::int32_t>(tabs_.size()))
        return tabs_[selected_tab_].get();

    return nullptr;
}

void_end_