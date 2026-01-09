#include <void/builder/builder.h>
#include <void/void.h>
#include <void/contents/overlays/custom_overlay.h>
#include <background/background_overlay.h>
#include <contents/container/tab_normal.h>
#include <contents/container/label.h>
#include <contents/sidebar.h>


void_begin_

void menu_builder::label(const xstr& name)
{
	instance()->sidebar_->add_tab(
		std::make_unique<::vo::label>(
			instance(), 
			name
		)
	);
}

tab_builder menu_builder::tab(const xstr& name)
{
	auto* _tab = instance()->sidebar_->add_tab(
		std::make_unique<tab_normal>(
			instance(), instance(),
			name,
			instance()
		)
	);

	return tab_builder(
		instance(),
		this,
		reinterpret_cast<tab_normal*>(_tab)
	);
}

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
