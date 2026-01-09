#include "spacing.h"
#include <void/void.h>


void_begin_

spacing::spacing(void_* instance, input_owner* input_owner)
	: widget(instance, input_owner, 0)
{
}

void spacing::update(float x, float y, float w, const render_input& input, bool occluded)
{
	(void)input;
	was_occluded_ = occluded;
	auto& style = instance()->style();

	last_pos_.x = x;
	last_pos_.y = y;
	last_pos_.w = w;
	last_pos_.h = style.border_size.get(instance()->scale());
}

void spacing::render(float alpha)
{
	if (was_occluded_) {
		return;
	}

	auto& style = instance()->style();
	auto& renderer = instance()->renderer();

	const float spacing = std::round(
		std::min(style.spacing->get(instance()->scale()) * 0.5f,
			last_pos_.w * 0.5f)
	);

	renderer.add_rect_filled(
		r2::vec2(last_pos_.x + spacing, 
			last_pos_.y),
		r2::vec2(last_pos_.x + last_pos_.w - spacing, 
			last_pos_.y + style.border_size.get(instance()->scale())),
		style.border().alpha(alpha)
	);
}

input_response spacing::input(const input_base& input)
{
	(void)input;
	return input_response::empty();
}

void spacing::on_activate()
{
}

void spacing::on_scale_change()
{
}

bool spacing::matches_search(const xstr& search) noexcept
{
	return search.empty();
}

void_end_