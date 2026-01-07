#pragma once
#include <contents/widgets/childwindow/childwindow_child.h>


void_begin_

class colorpicker_child : public childwindow_child {
public:
	using childwindow_child::childwindow_child;

public:
	virtual void render(const r2::rectf& full_pos, float alpha, float animation_disabled) override;
};

void_end_