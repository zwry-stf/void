#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>

#include "tab.h"
#include "overlay.h"


void_begin_

class menu_builder : protected vobj {
private:
	xstr last_child_{};
	xstr last_group_{};

	friend class base_builder_object;

public:
	using vobj::vobj;

public:
	void label(const xstr& name);
	tab_builder tab(const xstr& name);

	overlay_builder overlay();
};

void_end_