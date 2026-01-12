#pragma once
#include <contents/widgets/childwindow/childwindow_child.h>


void_begin_

class keybind_child : public childwindow_child {
public:
	using childwindow_child::childwindow_child;

	static std::unique_ptr<keybind_child> create_keybind(void_* instance, input_owner* input_owner,
													     input_owner_overlay* overlay_owner, class keybind_owner* bind);

public:
	virtual void render(const r2::rectf& full_pos, float alpha, float animation_disabled) override;
};

void_end_