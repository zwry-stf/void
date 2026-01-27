#pragma once
#include <void/util/vobj.h>
#include <vector>
#include <void/contents/overlays/custom_overlay.h>
#include <void/accessors/icons.h>


void_begin_

class keybind_owner;
class keybind_list : public vobj {
private:
    struct shown_bind {
        bool seen_in_refresh{ false };
        float animation_alpha{ 0.f };
        float animation_y{ -1.f };
        keybind_owner* bind;
    };
    std::vector<shown_bind> shown_binds_;
    float animation_{ 0.f };
    float animation_alpha_{ 0.f };

    icons::icon_handle keybind_icon_{ icons::kInvalidHandle };
    icons::icon_handle hold_icon_{ icons::kInvalidHandle };
    icons::icon_handle toggle_icon_{ icons::kInvalidHandle };
    icons::icon_handle always_on_icon_{ icons::kInvalidHandle };

public:
    bool hide_always_on{ true };
    bool hide_hold{ false };

public:
    keybind_list(void_* instance);

public:
    void on_overlay_update(void_* instance, custom_overlay& overlay);
    void on_overlay_render(void_* instance, custom_overlay& overlay);

private:
    void create_icons();
    void draw_header(void_* instance, custom_overlay& overlay, float height);
};

void_end_