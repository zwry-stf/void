#pragma once
#include <void/contents/widgets/widget.h>
#include <contents/widgets/dropdown/dropdown.h>


void_begin_

class multiselect : public dropdown {
public:
    using dropdown::dropdown;

public:
    virtual void render(float alpha) override;
};

void_end_