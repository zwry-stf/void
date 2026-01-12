#pragma once
#include <void/contents/widgets/widget.h>
#include <string>
#include <contents/input/keybind.h>


void_begin_

class keybind_widget : public widget {
private:
    const xstr name_;
    keybind_owner* const keybind_owner_;
    std::string last_text_;
    keybind_key last_key_{};

    bool text_width_calculated_{ false };
    float text_width_;

    r2::rectf keybind_pos_;

public:
    keybind_widget(void_* instance, input_owner* input_owner,
                   const xstr& name, keybind_owner* bind);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;

    virtual void on_scale_change() override;
    virtual bool matches_search(const xstr& search) noexcept override;
};

void_end_