#pragma once
#include <void/contents/widgets/widget.h>
#include <void/contents/widgets/textfield.h>


void_begin_

class colorpicker : public widget {
private:
    const xstr name_;
    const std::int32_t owned_overlay_id_;
    input_owner_overlay* const parent_overlay_owner_;

    bool text_width_calculated_{ false };
    float text_width_;

    r2::rectf color_pos_;

    std::unique_ptr<textfield> text_field_;

public:
    colorpicker(void_* instance, input_owner* input_owner, 
                input_owner_overlay* overlay_owner, std::int32_t overlay_id, const xstr& name);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;
    virtual void on_activate() override;
    virtual void on_scale_change() override;
    virtual bool matches_search(const xstr& search) noexcept override;

private:
    void on_stop_typing(const std::u32string& s);
};

void_end_