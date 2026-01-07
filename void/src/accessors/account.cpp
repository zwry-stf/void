#include <void/accessors/account.h>
#include <void/void.h>


void_begin_

void account::set_expiration_time(const time_point& t)
{
    expiration_time_ = t;
}

void account::set_display_name(const xstr& name)
{
    assert(name.find('\n') == xstr::npos);

    display_name_ = name;
}

void account::set_pfp(icons::icon_handle icon)
{
    icon_ = icon;
}

void account::render()
{
    if (!instance()->options().get<options::option_UserAccount>())
        return;

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    const auto& menu_pos = instance()->pos();

    if (icon_ == icons::kInvalidHandle) {
        icon_ = instance()->icons().get_or_create_handle(void_resources::default_pfp_png);
    }

    instance()->fonts().bind_font_small();

    constexpr sfloat kIconSize = { 32.f, true };

    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float side_spacing = std::round(spacing * 0.7f);
    const float full_height = kIconSize.get(instance()->scale()) + side_spacing * 2.f;

    const auto min = r2::vec2(menu_pos.x, menu_pos.y + menu_pos.h - full_height);

    // border
    renderer.add_rect_filled(
        r2::vec2(min.x + border_size, min.y),
        r2::vec2(min.x + sidebar_width - border_size,
            min.y + border_size),
        style.border()
    );

    renderer.push_clip_rect(
        r2::vec2(min.x + border_size,
            min.y + border_size),
        r2::vec2(min.x + sidebar_width - border_size,
            menu_pos.y + menu_pos.h - border_size),
        true
    );

    // pfp
    const float icon_size = kIconSize.get(instance()->scale());
    const auto* icon = instance()->icons().get_or_create(icon_, icon_size);

    renderer.add_image_rounded(
        icon->tex,
        r2::vec2(min.x + side_spacing, min.y + side_spacing),
        r2::vec2(min.x + side_spacing + icon_size, min.y + side_spacing + icon_size),
        icon_size * 0.5f,
        r2::color::white(),
        icon->uv_min, icon->uv_max
    );

    renderer.add_rect(
        r2::vec2(min.x + side_spacing, min.y + side_spacing),
        r2::vec2(min.x + side_spacing + icon_size, min.y + side_spacing + icon_size),
        style.accent(),
        border_size,
        icon_size * 0.5f
    );

    // name
    const float text_x = min.x + side_spacing + icon_size + std::round(spacing * 0.5f);
    const float text_offset_y = std::round((icon_size * 0.5f - text_size_small) * 0.5f);

    const float fade_width = spacing * 2.f;
    const float fade_end = min.x + sidebar_width - border_size;
    const float fade_start = fade_end - fade_width;

    renderer.add_text_faded(
        r2::vec2(text_x, min.y + side_spacing + text_offset_y),
        style.text(), style.text().transparent(),
        fade_start, fade_end,
        display_name_.empty() ? xstr("unknown") : display_name_
    );

    // expiration data
    const auto time_now = instance()->frame_start();

    bool expired = false;
    if (expiration_time_ == time_point() ||
        expiration_time_ < time_now)
        expired = true;

    if (display_text_.empty() ||
        last_expiration_time_ != expiration_time_ ||
        was_expired_ != expired) {
        last_expiration_time_ = expiration_time_;
        was_expired_ = expired;

        if (!expired) {
            const auto now_steady = std::chrono::steady_clock::now();
            const auto remaining = expiration_time_ - now_steady;

            const auto now_sys = std::chrono::system_clock::now();
            const auto exp_sys = now_sys + remaining;

            auto stime = std::chrono::clock_cast<std::chrono::system_clock>(exp_sys);

            auto day_point = std::chrono::floor<std::chrono::days>(stime);
            std::chrono::year_month_day ymd{ day_point };
            std::chrono::hh_mm_ss time_of_day{ stime - day_point };

            display_text_ = std::format("{:%d. %b %Y}", ymd);
        }
        else
            display_text_ = "expired";
    }

    renderer.add_text_faded(
        r2::vec2(text_x,
            min.y + full_height - side_spacing - text_offset_y - text_size_small),
        style.grey(), style.grey().transparent(),
        fade_start, fade_end,
        display_text_
    );

    renderer.pop_clip_rect();
}

void_end_