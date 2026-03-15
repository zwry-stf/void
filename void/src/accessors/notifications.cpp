#include <void/accessors/notifications.h>
#include <void/void.h>


void_begin_

notification_buider::notification_buider(void_* instance)
    : vobj(instance),
      current_color_(instance->style().text())
{
}

notification_buider::~notification_buider()
{
    if (data_.empty())
        return;

    instance()->notifications().add_notification(std::move(data_));
}

notification_buider notifications::create_notification()
{
    return notification_buider(instance());
}

notification_buider notifications::create_note()
{
    auto& style = instance()->style();
    return std::move(create_notification()
        << style.accent() << xstr("[")
        << style.accent2() << instance()->options().get<options::option_ProjectName>()
        << style.accent() << xstr("]")
        << style.text() << xstr(" : "));
}

notification_buider notifications::create_error()
{
    auto& style = instance()->style();
    return std::move(create_notification()
        << style.accent() << xstr("[")
        << r2::color::red().interp(style.accent2(), 0.5f) << instance()->options().get<options::option_ProjectName>()
        << style.accent() << xstr("]")
        << style.text() << xstr(" : ")
        << style.text_accent() << xstr("Error - "));
}

void notifications::add_notification(std::vector<std::unique_ptr<notification_data>>&& data)
{
    if (!instance()->options().get<options::option_Notifications>())
        return;

    std::lock_guard<std::mutex> lock(mutex_);
    notifications_.emplace_back(std::chrono::steady_clock::now(), std::move(data));
}

void notifications::render()
{
    if (!instance()->options().get<options::option_Notifications>())
        return;

    auto time_now = instance()->frame_start();

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto& util = instance()->util();

    const float visible_time = style.notification_time();
    const float animation_time_in = style.notification_time() / style.animation_speed() * 0.5f;
    const float animation_time_out = style.notification_time() / style.animation_speed() * 2.f;

    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());

    // render
    const float screen_spacing_x = spacing * 2.f;
    const float screen_spacing_y = std::round(screen_spacing_x * 0.8f);
    constexpr sfloat kTextSpacingY = sfloat(3.f);
    const float text_spacing_y = kTextSpacingY.get(instance()->scale());
    const float noti_height      = text_size_small + text_spacing_y * 2.f;
    const float noti_spacing     = std::round(spacing * 0.6f);

    const float side_spacing = spacing;

    std::lock_guard<std::mutex> lock(mutex_);

    float pos_y = 0.f;
    for (auto it = notifications_.begin(); it != notifications_.end();) {
        auto& notification = *it;

        const float remaining_time = std::chrono::duration<float>(time_now - notification.creation_time).count();

        const float progress_in = std::clamp(remaining_time / animation_time_in, 0.f, 1.f);
        const float progress_out = std::clamp(
            std::max(remaining_time - visible_time - animation_time_in, 0.f) / animation_time_out,
            0.f, 1.f
        );

        const float alpha_in = progress_in * progress_in * progress_in; // ease in
        const float alpha_out = std::pow(1.f - progress_out, 3.f); // ease out

        float alpha = std::min(alpha_in, alpha_out);
        if (alpha < util::g_min_alpha) {
            if (alpha_out < util::g_min_alpha)
                it = notifications_.erase(it);
            else
                it++;
            continue;
        }

        const float animation_size = noti_height * 0.7f;
        const float notification_pos_x = screen_spacing_x + std::pow(1.f - progress_in, 2.f) * animation_size - (1.f - alpha_out) * animation_size;

        // animate, if not set yet, force to position
        if (notification.pos_y < 0.f)
            notification.pos_y = pos_y;
        else
            notification.pos_y = util.lerp(notification.pos_y, pos_y, 2.f);

        float full_width = side_spacing * 2.f;

        // calculate width
        for (auto& field : notification.data) {
            if (!field->text_width_calculated) {
                field->cache_text_size(renderer);
            }
            full_width += field->text_width;
        }

        // background
        const float max_rounding = (noti_height - spacing) * 0.5f;
        const float background_rounding = std::round(std::min(rounding * 0.5f, max_rounding));
        const auto min = r2::vec2(notification_pos_x, screen_spacing_y + notification.pos_y);
        const auto max = min + r2::vec2(full_width, noti_height);

        renderer.add_shadow_rect_filled(
            min, max,
            style.background().opague().alpha(alpha * 0.6f),
            background_rounding, noti_height
        );

        renderer.add_rect_filled(
            min, max,
            style.background().opague().alpha(alpha),
            background_rounding
        );

        // render items
        float pos_x = min.x + side_spacing;
        for (auto& field : notification.data) {
            const auto pos = r2::vec2(pos_x, min.y + text_spacing_y);

            field->render(renderer, pos, field->color.alpha(alpha));
            pos_x += field->text_width;
        }

        // border
        const auto border_color = style.accent2().alpha(alpha);

        if (background_rounding < 0.5f) {
            renderer.add_rect_filled(
                min, r2::vec2(max.x, min.y + border_size),
                border_color
            );
        }
        else {
            const bool odd = (static_cast<int>(std::round(border_size)) & 1) != 0;
            const float snap = odd ? 0.5f : 0.0f;

            const float border_offset = border_size * 0.5f;

            constexpr float kCornerStep = 2.f;
            const float corner_size = background_rounding * r2::math::g_pi_div_2;
            const float step = kCornerStep / corner_size * 2.f;

            renderer.path_arc_to<3, 6>(
                r2::vec2(
                    max.x - background_rounding - border_offset,
                    min.y + background_rounding + border_offset
                ),
                background_rounding,
                step
            );
            renderer.path_arc_to<6, 9>(
                r2::vec2(
                    min.x + background_rounding + border_offset, 
                    min.y + background_rounding + border_offset
                ), 
                background_rounding, 
                step
            );
            renderer.path_stroke(
                border_color,
                border_size,
                false /* closed */
            );

            const float max_border_length = noti_height - background_rounding * 2.f;
            const float border_length = std::round(std::min(background_rounding * 1.2f, max_border_length));
            const auto border_transparent = border_color.transparent();
            renderer.add_rect_filled_multicolor(
                r2::vec2(min.x, min.y + background_rounding + border_offset + snap),
                r2::vec2(min.x + border_size, min.y + background_rounding + border_length),
                border_color, border_color,
                border_transparent, border_transparent
            );

            renderer.add_rect_filled_multicolor(
                r2::vec2(max.x - border_size, min.y + background_rounding + border_offset),
                r2::vec2(max.x, min.y + background_rounding + border_length),
                border_color, border_color,
                border_transparent, border_transparent
            );
        }

        if (alpha > 0.5f)
            pos_y += noti_height + noti_spacing;

        it++;
    }
}

void notifications::on_scale_changed()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& n : notifications_) {
        for (auto& d : n.data) {
            d->text_width_calculated = false;
        }
    }
}

void_end_