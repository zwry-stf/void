#include <void/builder/builder.h>
#include <void/void.h>
#include <void/contents/overlays/custom_overlay.h>
#include <background/background_overlay.h>


void_begin_

overlay_builder menu_builder::overlay()
{
	auto* background_overlay = instance()->background().get_background_overlay_instance();

	auto* overlay = background_overlay->add_overlay(
		std::make_unique<custom_overlay>(
			instance(),
			instance(),
			custom_overlay_cfg(),
			custom_overlay_data()
		)
	);

	return overlay_builder(instance(), overlay);
}

void_end_
