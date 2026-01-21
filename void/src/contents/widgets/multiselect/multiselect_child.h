#pragma once
#include <contents/widgets/dropdown/dropdown_child.h>


void_begin_

class multiselect_child : public dropdown_child {
public:
    using dropdown_child::dropdown_child;

public:
    virtual void render(const r2::rectf& full_pos, float alpha, float animation_disabled) override;
};

void_end_