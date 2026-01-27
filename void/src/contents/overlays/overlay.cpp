#include <void/contents/overlays/overlay.h>
#include <void/void.h>


void_begin_

overlay::overlay(void_* instance, input_owner_overlay* input_owner,
                 bool has_overlays, bool is_childwindow)
    : vobj(instance),
      input_receiver_overlay(input_owner),
      has_overlays_(has_overlays),
      is_childwindow_(is_childwindow)
{
}

void overlay::update(const overlay_render_input& input)
{
    des_animation_ = instance()->util().lerp2(
        des_animation_,
        input.is_opened(this),
        0.8f
    );
    animation_ = instance()->util().lerp(
        animation_,
        des_animation_,
        3.f
    );

    alpha_ = animation_ * animation_;
    alpha_ = alpha_ < util::g_min_alpha ? 0.f :
        (alpha_ > 1.f - util::g_min_alpha ? 1.f : alpha_);
}

void overlay::set_pos(const r2::vec2& pos, const r2::vec2& parent_size)
{
    set_pos_ = r2::vec2(std::round(pos.x - instance()->pos().x),
        std::round(pos.y - instance()->pos().y));
    parent_size_ = parent_size;
    pos_changed_ = true;
}

input_response overlay::should_block_input(const overlay_input& input)
{
    assert(input.is_opened(this));

    if (input.event().is_message(message_type::key_down) &&
        input.event().get_key() == key::escape) {
        input.clear_opened();
        return input_response::handled();
    }

    return input_response::handled();
}

void_end_