#include "colorpicker_child.h"
#include <void/void.h>
#include <contents/overlays/colorpicker/colorpicker.h>


void_begin_

void colorpicker_child::render(const r2::rectf& full_pos, float alpha, float animation_disabled)
{
    if (last_pos_.x + last_pos_.w < full_pos.x)
        return; // occluded to left

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto& util = instance()->util();

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<colorpicker_overlay>(overlay_id_);

    // render
    const bool push_clip_rect = last_pos_.x < full_pos.x;
    if (push_clip_rect) {
        renderer.push_clip_rect(
            r2::vec2(full_pos.x, full_pos.y),
            r2::vec2(full_pos.x + full_pos.w, full_pos.y + full_pos.h),
            true
        );
    }

    const float border_size = style.border_size.get(instance()->scale());
    const float selected_animation = std::max(animation_selected_, owned_overlay.alpha());
    const float offset = border_size * selected_animation;

    if (icon_ == icons::kInvalidHandle) {
        icon_ = instance()->icons().get_or_create_handle(void_resources::color_png);
    }
    const auto* icon = instance()->icons().get_or_create(icon_, last_pos_.w);
    renderer.add_image(
        icon->tex,
        r2::vec2(last_pos_.x - offset, last_pos_.y - offset),
        r2::vec2(last_pos_.x + last_pos_.w + offset, last_pos_.y + last_pos_.h + offset),
        util.disable_color(owned_overlay.get_color()->opague().interp(
            style.accent(), animation_hovered_ * 0.3f).alpha(
                alpha * (0.7f + selected_animation * 0.3f)), animation_disabled),
        icon->uv_min, icon->uv_max
    );

    if (push_clip_rect)
        renderer.pop_clip_rect();
}

void_end_