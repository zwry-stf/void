#pragma once
#include <void/contents/widgets/widget.h>


void_begin_

class button : public widget {
private:
    const xstr name_;
    const xstr button_text_;
    std::function<void()> callback_;

    bool text_width_calculated_{ false };
    float text_width_;

    r2::rectf button_pos_;

public:
    button(void_* instance, input_owner* input_owner, 
           const xstr& name, const xstr& button_text, std::function<void()>&& callback);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;

    virtual void on_scale_change() override;
    virtual bool matches_search(const xstr& search) noexcept override;
};

void_end_