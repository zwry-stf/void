#pragma once
#include <contents/widgets/childwindow/childwindow_child.h>


void_begin_

class optional_colorpicker_child : public childwindow_child {
private:
    bool* const value_enabled_;

public:
    optional_colorpicker_child(void_* instance, input_owner* input_owner,
                               input_owner_overlay* overlay_owner, std::int32_t overlay_id,
                               bool* value);

public:
    virtual void render(const r2::rectf& full_pos, float alpha, float animation_disabled) override;
    virtual input_response input(const input_base& input) override;
};

void_end_