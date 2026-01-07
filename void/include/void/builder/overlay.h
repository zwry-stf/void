#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <functional>


void_begin_

class overlay_builder : protected vobj {
private:
	class custom_overlay* const overlay_instance_;
	xstr config_path_{};

public:
	overlay_builder(void_* instance, class custom_overlay* overlay_instance);
	~overlay_builder();

public:
	// set the overlays default position, range 0-1
	overlay_builder& pos(float x, float y);

	// set the overlays default unscaled size
	overlay_builder& size(float w, float h);

	// set the overlays unscaled min size
	overlay_builder& min(float w, float h);

	// set the overlays unscaled min size
	overlay_builder& max(float w, float h);

	// enable/disable resizing
	// default: true
	overlay_builder& make_resizable(bool value = true);

	// enable/disable liquid glass effect
	// default: false
	overlay_builder& liquid_glass(bool value = true);

	// render callback
	overlay_builder& on_render(std::function<void(void_* instance, class custom_overlay&)>&& callback);

	// update callback
	overlay_builder& on_update(std::function<void(void_* instance, class custom_overlay&)>&& callback);

	// set the config path
	overlay_builder& config(const xstr& path);
};

void_end_