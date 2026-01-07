#include <void/contents/widgets/widget_child.h>
#include <void/void.h>


void_begin_

widget_child::widget_child(void_* instance, input_owner* input_owner, std::int32_t input_num, 
                           input_owner_overlay* overlay_owner, std::int32_t overlay_id)
    : vobj(instance),
      input_receiver(input_owner, input_num),
      parent_overlay_owner_(overlay_owner),
      overlay_id_(overlay_id)
{
}

float widget_child::update(const r2::rectf&, float, float, float, const render_input& input)
{
    auto& util = instance()->util();

    // animations
    animation_hovered_ = util.lerp(animation_hovered_, input.is_hovered(this));
    animation_selected_ = util.lerp(animation_selected_, input.is_selected(this));

    return last_pos_.w;
}

void widget_child::on_activate()
{
    animation_hovered_ = 0.f;
    animation_selected_ = 0.f;
}

void_end_