#pragma once
#include <void/util/vobj.h>
#include <void/util/string_token.h>
#include <functional>


void_begin_

class overlay_builder : protected vobj {
private:
    class custom_overlay* const overlay_instance_;
    string_token config_path_{};

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

    // enable/disable moving
    // default: true
    overlay_builder& make_movable(bool value = true);

    // clamp overlay inside screen rect
    // default: true
    overlay_builder& clamp_in_window(bool value = true);

    // enable/disable liquid glass effect
    // default: false
    overlay_builder& liquid_glass(bool value = true);

    // render callback
    overlay_builder& on_render(std::function<void(void_*, class custom_overlay&)>&& callback);

    // update callback
    overlay_builder& on_update(std::function<void(void_*, class custom_overlay&)>&& callback);

    // input callback
    overlay_builder& on_input(std::function<class input_response(void_*, class custom_overlay&, const class input_base&)>&& callback);

    // set the config path
    overlay_builder& config(const string_token& path);
};

void_end_