#pragma once
#include <void/contents/widgets/widget.h>


void_begin_

class toggle : public widget {
private:
    const xstr name_;
    bool* const value_;

    float animation_{ 0.f };

    r2::rectf toggle_pos_;

public:
    toggle(void_* instance, input_owner* input_owner,
           const xstr& name, bool* value);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;
    virtual void set_pos(const r2::vec2& pos) override;

    virtual void on_activate() override;
    virtual bool matches_search(const xstr& search) noexcept override;

private:
    void toggle_value() noexcept;
};

void_end_