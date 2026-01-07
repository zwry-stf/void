#pragma once
#include <void/util/vobj.h>
#include <void/util/default_value.h>
#include <void/util/scalable_float.h>
#include <r2/renderer_definitions.h>


void_begin_

class style : protected vobj {
public:
    using vobj::vobj;

public:
    // configurable values
    default_value<r2::color> background         = r2::color(0x1A, 0x19, 0x1F, 0xDC);
    default_value<r2::color> overlay_background = r2::color(0x1A, 0x19, 0x1F, 0xC1);
    default_value<r2::color> group_background   = r2::color(0xCA, 0xCA, 0xE2, 0x0D);
    default_value<r2::color> accent             = r2::color(0xB3, 0xC7, 0xFF, 0x59);
    default_value<r2::color> accent2            = r2::color(0xB1, 0xAB, 0xC5, 0x57);
    default_value<r2::color> border             = r2::color(0xB9, 0xC5, 0xE4, 0x33);
    default_value<r2::color> highlight          = r2::color(0xD6, 0xD3, 0xF0, 0xE6);
    default_value<r2::color> text               = r2::color(0xE0, 0xE8, 0xFF, 0xA7);
    default_value<r2::color> text_accent        = r2::color(0xC7, 0xCC, 0xFF, 0xCC);
    default_value<r2::color> icon               = r2::color(0xE0, 0xE8, 0xFF, 0xB3);
    default_value<r2::color> disabled           = r2::color(0.3f, 0.3f, 0.3f);
    default_value<r2::color> grey               = r2::color(0x36, 0x3C, 0x4D, 0xD9);

    default_value<float> animation_speed = 11.f;
    default_value<sfloat> spacing        = sfloat(8.f);
    default_value<sfloat> rounding       = sfloat(8.f);
    default_value<sfloat> overlay_width  = sfloat(220.f, true);

    default_value<float> notification_time = 4.f;
    
    // constant values
    const float  scroll_speed         = 85.f;
    const sfloat border_size          = sfloat(1.f);
    const sfloat text_size_small      = sfloat(18.f, true);
    const sfloat text_size_large      = sfloat(22.f, true);
    const sfloat sidebar_width        = sfloat(136.f, true);
    const sfloat top_bar_height       = sfloat(48.f, true);
    const sfloat drawable_button_size = sfloat(22.f, true);
    const sfloat config_height        = sfloat(64.f, true);
    const sfloat theme_width          = sfloat(160.f, true);
    const sfloat theme_height         = sfloat(144.f, true);
    const sfloat childwidget_size     = sfloat(16.f, true);

    const sfloat min_x = sfloat(400.f, true);
    const sfloat min_y = sfloat(380.f, true);

    const float max_scroll_speed = 140.f;
};

void_end_