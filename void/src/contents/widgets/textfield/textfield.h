#pragma once
#include <void/contents/widgets/widget.h>
#include <void/contents/widgets/textfield.h>


void_begin_

class textfield_widget : public widget {
private:
	const xstr name_;

	std::unique_ptr<textfield> text_field_;
};

void_end_