#pragma once
#include <void/def.h>
#include <r2/renderer_definitions.h>


void_begin_

namespace color {
    struct hsv {
        float h, s, v, a;
    };

    inline hsv color_convert_rgb_to_hsv(r2::color col) {
        float K = 0.f;
        if (col.g < col.b) {
            const float old = col.b;
            col.b = col.g;
            col.g = old;
            K = -1.f;
        }
        if (col.r < col.g) {
            const float old = col.r;
            col.r = col.g;
            col.g = old;
            K = -2.f / 6.f - K;
        }

        hsv ret;
        ret.a = col.a;
        const float chroma = col.r - (col.g < col.b ? col.g : col.b);
        ret.h = std::abs(K + (col.g - col.b) / (6.f * chroma + 1e-20f));
        ret.s = chroma / (col.r + 1e-20f);
        ret.v = col.r;
        return ret;
    }

    inline r2::color color_convert_hsv_to_rgb(const hsv& hsv) {
        if (hsv.s == 0.0f) {
            return r2::color(hsv.v, hsv.v, hsv.v, hsv.a);
        }

        const float h = std::fmod(hsv.h, 1.0f) / (60.0f / 360.0f);
        const int i = static_cast<int>(std::floor(h));
        const float f = h - static_cast<float>(i);
        const float p = hsv.v * (1.0f - hsv.s);
        const float q = hsv.v * (1.0f - hsv.s * f);
        const float t = hsv.v * (1.0f - hsv.s * (1.0f - f));

        r2::color ret;
        ret.a = hsv.a;
        switch (i) {
        case 0: ret.r = hsv.v; ret.g = t; ret.b = p; break;
        case 1: ret.r = q; ret.g = hsv.v; ret.b = p; break;
        case 2: ret.r = p; ret.g = hsv.v; ret.b = t; break;
        case 3: ret.r = p; ret.g = q; ret.b = hsv.v; break;
        case 4: ret.r = t; ret.g = p; ret.b = hsv.v; break;
        case 5:
        default: ret.r = hsv.v; ret.g = p; ret.b = q; break;
        }

        return ret;
    }
}

void_end_