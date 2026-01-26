#include "keybind_child.h"
#include <void/void.h>
#include <contents/overlays/childwindow/childwindow.h>
#include "keybind_widget.h"
#include <contents/widgets/dropdown/dropdown.h>
#include <contents/overlays/dropdown/dropdown.h>


void_begin_

std::unique_ptr<keybind_child> keybind_child::create_keybind(void_* instance, input_owner* input_owner,
                                                             input_owner_overlay* overlay_owner, keybind_owner* bind)
{
    static constexpr xstr mode_names_eng[] = {
        xstr("Hold"),
        xstr("Toggle"),
        xstr("Always"),
    };

    const auto overlay_id = overlay_owner->create_overlay(
        std::make_unique<childwindow>(
            instance,
            input_owner,
            overlay_owner,
            xstr("Keybind")
        )
    );
    auto& overlay = *overlay_owner->get_overlay<childwindow>(overlay_id);

    auto* widget = overlay.add_widget(
        std::make_unique<keybind_widget>(
            instance,
            input_owner,
            xstr("Key"),
            bind
        )
    );
    widget->set_disabled_callback(
        [bind]() -> bool {
            return bind->mode_ref() == static_cast<std::size_t>(keybind_mode::always);
        }
    );

    const auto dropdown_overlay_id = overlay.create_overlay(
        std::make_unique<dropdown_overlay>(
            instance, input_owner,
            &overlay,
            std::unique_ptr<list_options>(list_options::create_constant(mode_names_eng)),
            &bind->mode_ref()
        )
    );

    overlay.add_widget(
        std::make_unique<dropdown>(
            instance,
            input_owner,
            &overlay,
            dropdown_overlay_id,
            xstr("Mode")
        )
    );

    return std::make_unique<keybind_child>(
        instance,
        input_owner,
        overlay_owner,
        overlay_id
    );
}

void keybind_child::render(const r2::rectf& full_pos, float alpha, float animation_disabled)
{
    if (last_pos_.x + last_pos_.w < full_pos.x)
        return; // occluded to left

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto& util = instance()->util();

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<childwindow>(overlay_id_);

    // render
    const bool push_clip_rect = last_pos_.x < full_pos.x;
    if (push_clip_rect)
        renderer.modify_clip_rect_x(full_pos.x, full_pos.x + full_pos.w);

    const float border_size = style.border_size.get(instance()->scale());
    const float selected_animation = std::max(animation_selected_, owned_overlay.alpha());
    const float offset = border_size * selected_animation;

    if (icon_ == icons::kInvalidHandle) {
        icon_ = instance()->icons().get_or_create_handle(void_resources::keybind_png);
    }
    const auto* icon = instance()->icons().get_or_create(icon_, last_pos_.w);
    renderer.add_image(
        icon->tex,
        r2::vec2(last_pos_.x - offset, last_pos_.y - offset),
        r2::vec2(last_pos_.x + last_pos_.w + offset, last_pos_.y + last_pos_.h + offset),
        util.disable_color(style.icon().interp(
            style.accent(), animation_hovered_ * 0.6f).alpha(
                alpha * (0.7f + selected_animation * 0.3f)
            ), animation_disabled),
        icon->uv_min, icon->uv_max
    );

    if (push_clip_rect)
        renderer.pop_clip_rect();
}

void_end_