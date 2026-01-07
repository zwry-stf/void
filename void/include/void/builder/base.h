#pragma once
#include <void/util/vobj.h>


void_begin_

class base_builder_object : protected vobj {
private:
	class menu_builder* const builder_;

public:
	base_builder_object(void_* instance, class menu_builder* builder) noexcept
		: vobj(instance),
		builder_(builder) {
	}

protected:
	[[nodiscard]] auto* builder() const noexcept {
		return builder_;
	}
};

void_end_