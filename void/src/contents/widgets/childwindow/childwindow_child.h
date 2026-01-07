#pragma once
#include <void/contents/widgets/widget_child.h>
#include <void/accessors/icons.h>


void_begin_

class childwindow_child : public widget_child {
protected:
    icons::icon_handle icon_ = icons::kInvalidHandle;

public:
    childwindow_child(void_* instance, input_owner* input_owner,
                      input_owner_overlay* overlay_owner, std::int32_t overlay_id);

public:
    virtual float update(const r2::rectf& full_pos, float right_x, float y, float h,
                         const render_input& input) override;
    virtual void render(const r2::rectf& full_pos, float alpha, float animation_disabled) override;
    virtual input_response input(const input_base& input) override;
};

void_end_