#pragma once
#include <void/util/vobj.h>


void_begin_

class theme : protected vobj {
public:
	using vobj::vobj;

public:
	[[nodiscard]] class _theme* get_theme_instance() const noexcept;
};

void_end_