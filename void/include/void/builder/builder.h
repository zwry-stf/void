#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>

#include "base.h"
#include "group.h"
#include "childwindow.h"
#include "overlay.h"


void_begin_

class child_tab_base_builder : protected base_builder_object {
private:
	class child_tab* tab_instance_;

public:
	child_tab_base_builder(void_* instance, menu_builder* builder, class child_tab_normal* tab_instance);

public:
	child_tab_base_builder& set_icon(int resource_id);
};

class tab_normal_builder : protected base_builder_object {
private:
	class tab_normal* tab_instance_;

public:
	tab_normal_builder(void_* instance, menu_builder* builder, class tab_normal* tab_instance);

public:
	tab_normal_builder& set_icon(int resource_id);
};

class menu_builder : protected vobj {
private:

public:
	using vobj::vobj;

public:
	void label(const xstr& name);
	tab_normal_builder tab(const xstr& name);

	overlay_builder overlay();
};

void_end_