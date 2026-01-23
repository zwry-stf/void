#pragma once
#include <void/contents/widgets/widget.h>


void_begin_

class spacing : public widget {
public:
    spacing(void_* instance, input_owner* input_owner);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;

    virtual void on_activate() override;
    virtual void on_scale_change() override;
    virtual bool matches_search(const xstr& search) noexcept override;
};

void_end_