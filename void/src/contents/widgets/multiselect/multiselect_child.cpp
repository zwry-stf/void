#include "multiselect_child.h"
#include <void/void.h>
#include <contents/overlays/multiselect/multiselect.h>
#include <contents/widgets/dropdown/dropdown.h>


void_begin_

void multiselect_child::render(const r2::rectf& full_pos, float alpha, float animation_disabled)
{
    auto& renderer = instance()->renderer();

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<multiselect_overlay>(overlay_id_);

    const float shadow_size = 0.7f;
    bool push_clip_rect = last_pos_.x - shadow_size < full_pos.x;
    if (push_clip_rect) {
        renderer.push_clip_rect(
            r2::vec2(full_pos.x, full_pos.y),
            r2::vec2(full_pos.x + full_pos.w, full_pos.y + full_pos.h),
            true
        );
    }

    dropdown::render_dropdown(
        instance(),
        last_pos_,
        animation_hovered_,
        animation_selected_,
        animation_disabled,
        alpha,
        owned_overlay.get_text(),
        true /* is multiselect */
    );

    if (push_clip_rect)
        renderer.pop_clip_rect();
}

void_end_