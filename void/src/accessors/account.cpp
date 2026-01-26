#include <void/accessors/account.h>
#include <void/void.h>
#include <contents/overlays/menu_options/menu_options.h>


void_begin_

enum class selected_ids : std::int32_t {
    selected_name,
    selected_max,
};

account::account(void_* instance)
    : vobj(instance),
      input_receiver(instance,
          std::to_underlying(selected_ids::selected_max)
      ),
      display_name_("unknown")
{
    menu_options_ = instance->create_overlay(
        std::make_unique<menu_options>(
            instance, instance, instance
        )
    );
}

account::~account() = default;

void account::set_expiration_time(const time_point& t)
{
    expiration_time_ = t;
}

void account::set_display_name(const xstr& name)
{
    if (name.empty()) {
        display_name_ = "unknown";
    }
    else {
        assert(name.find('\n') == xstr::npos);

        display_name_ = name;
    }

    name_text_width_calculated_ = false;
}

void account::set_pfp(icons::icon_handle icon)
{
    icon_ = icon;
}

void account::render(const render_input& input)
{
    if (!instance()->options().get<options::option_UserAccount>())
        return;

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto& util = instance()->util();

    const auto& menu_pos = instance()->pos();

    if (icon_ == icons::kInvalidHandle) {
        icon_ = instance()->icons().get_or_create_handle(void_resources::default_pfp_png);
    }

    instance()->fonts().bind_font_small();

    constexpr sfloat kDefaultHeight = { 44.f, true };

    const float spacing = style.spacing->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float full_height = kDefaultHeight.get(instance()->scale());
    const float side_spacing = std::round(spacing * 0.7f);
    const float icon_size = full_height - side_spacing * 2.f;

    last_pos_ = r2::rectf{
        menu_pos.x,
        menu_pos.y + menu_pos.h - full_height,
        sidebar_width,
        full_height
    };

    const auto min = r2::vec2(last_pos_.x, last_pos_.y);

    // border
    renderer.add_rect_filled(
        r2::vec2(min.x + border_size, min.y),
        r2::vec2(min.x + last_pos_.w - border_size,
            min.y + border_size),
        style.border()
    );

    // pfp
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
    const float fade_end = min.x + last_pos_.w - border_size;
    const float fade_start = fade_end - fade_width;

    if (!name_text_width_calculated_) {
        if (renderer.get_text_width_strict(display_name_, name_text_width_))
            name_text_width_calculated_ = true;
        else
            name_text_width_ = renderer.get_text_width(display_name_);
        name_text_width_ = std::ceil(name_text_width_);
    }

    const auto& menu_options = instance()->get_overlay<::vo::menu_options>(menu_options_);

    name_animation_hovered_ = util.lerp(
        name_animation_hovered_,
        instance()->input_get_overlay_render_input().is_opened(menu_options) ||
            input.is_hovered(this, selected_ids::selected_name)
    );

    text_pos_ = r2::rectf{
        text_x,
        min.y + side_spacing + text_offset_y,
        name_text_width_,
        text_size_small
    };

    renderer.add_text_faded(
        r2::vec2(text_pos_.x, text_pos_.y),
        style.text(), style.text().transparent(),
        fade_start, fade_end,
        display_name_
    );

    renderer.add_rect_filled_faded(
        r2::vec2(text_pos_.x, text_pos_.y + text_pos_.h - border_size),
        r2::vec2(text_pos_.x + text_pos_.w, text_pos_.y + text_pos_.h),
        style.accent().alpha(name_animation_hovered_),
        style.accent().transparent(),
        fade_start, fade_end
    );

    // expiration date
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
            min.y + last_pos_.h - side_spacing - text_offset_y - text_size_small),
        style.grey(), style.grey().transparent(),
        fade_start, fade_end,
        display_text_
    );
}

input_response account::input(const input_base& input)
{
    if (!input.nothing_selected() &&
        !input.is_selected(this))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_move)) {
        if (util::is_in_rect(mouse_x, mouse_y, text_pos_)) {
            input.set_hovered(this, selected_ids::selected_name);
            instance()->cursors().set_cursor(cursor::hand);

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, text_pos_)) {
            instance()->set_opened(menu_options_);

            return input_response::handled();
        }
    }

    return input_response::empty();
}

void account::on_scale_changed()
{
    name_text_width_calculated_ = false;
}

void_end_