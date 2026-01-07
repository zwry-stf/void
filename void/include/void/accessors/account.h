#pragma once
#include <void/util/vobj.h>
#include <void/util/time_point.h>
#include <void/util/xstr.h>
#include <void/accessors/icons.h>
#include <r2/renderer_definitions.h>
#include <string>


void_begin_

class account : protected vobj {
private:
    time_point expiration_time_;
    xstr display_name_{};
    icons::icon_handle icon_{ icons::kInvalidHandle };

    std::string display_text_;
    bool was_expired_{ false };
    time_point last_expiration_time_;

public:
    using vobj::vobj;

public:
    void set_expiration_time(const time_point& t);
    void set_display_name(const xstr& name);
    void set_pfp(icons::icon_handle icon);

    void render();
};

void_end_