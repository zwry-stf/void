#include "multiselect.h"
#include <void/void.h>
#include <contents/overlays/multiselect/multiselect.h>


void_begin_

void multiselect::render(float alpha)
{
    if (was_occluded_) {
        return;
    }

    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const float border_size = style.border_size.get(instance()->scale());

    const float text_spacing = std::round(
        (last_pos_.h - style.text_size_small.get(instance()->scale())) * 0.5f
    );

    auto& owned_overlay = *parent_overlay_owner_->get_overlay<multiselect_overlay>(overlay_id_);

    // text
    const float cliprect_left = static_cast<float>(
        renderer.cmd_header().clip_rect.left
    );
    renderer.push_clip_rect(
        r2::vec2(cliprect_left, last_pos_.y),
        r2::vec2(child_widget_left_pos_ - border_size, last_pos_.y + last_pos_.h),
        true
    );

    renderer.add_text_faded(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        style.text().transparent(),
        style.text().alpha((alpha * (1.f - animation_disabled_)) *
            (0.3f + animation_selected_ * 0.4f)),
        cliprect_left, std::max(cliprect_left, last_pos_.x + border_size),
        name_,
        true /* blurred */
    );
    
    renderer.add_text(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        util.disable_color(
            style.text(), animation_disabled_).alpha(alpha),
        name_
    );

    renderer.pop_clip_rect();

    dropdown::render_dropdown(
        instance(),
        dropdown_pos_,
        animation_hovered_,
        animation_selected_,
        animation_disabled_,
        alpha,
        owned_overlay.get_text(),
        true /* is multiselect */
    );

    render_child_widgets(alpha);
}

void_end_