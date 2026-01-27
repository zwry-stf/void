#pragma once
#include <void/contents/container/tab.h>


void_begin_

class label : public tab {
private:
    bool text_width_calculated_{ false };
    float text_width_;
    
    r2::rectf last_pos_;

public:
    label(void_* instance, const xstr& name);

public:
    virtual float update(float x, float y, float w,
        const render_input& input, bool selected) override;
    virtual void render(bool selected) override;
    virtual input_response input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id) override;

    virtual void on_scale_change() override;
};

void_end_