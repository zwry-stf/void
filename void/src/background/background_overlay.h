#pragma once
#include <r2/renderer_definitions.h>
#include <void/util/vobj.h>
#include <void/util/default_value.h>
#include <background/background.h>
#include <void/contents/input/input_response.h>
#include <void/contents/input/events.h>
#include <void/contents/overlays/custom_overlay.h>


void_begin_

struct overlay_data {
    r2::rectf pos;
    r2::color background;
    r2::color border;
    float rounding_top;
    float rounding_bottom;
    float animation;
    float liquid_curve_value;
};

struct immediate_overlay {
    r2::rectf pos;
    float animation;
};

class _background_overlay : protected vobj {
private:
    std::unique_ptr<r2::shaderprogram> data_shader_composition_;
    std::unique_ptr<r2::shaderprogram> data_shader_liquidglass_;
    std::unique_ptr<r2::buffer> data_constant_buffer_;

    _background::blur_shader_constants blur_shader_constants_{};

    std::vector<immediate_overlay> immediate_overlays_;
    std::vector<std::unique_ptr<custom_overlay>> custom_overlays_;

    struct _shader_constants {
        r2::vec2 resolution;
        float _pad0[2];

        std::uint32_t blur_enabled;
        float noise_scale;
        union {
            float border_size;
            float warp_size;
        };
        float blend_amount;

        // Overlays
        overlay_data overlay;
    };

public:
    using vobj::vobj;

public:
    void init(_background* background);
    void destroy();
    void reset_data();
    void render(_background* background);
    void render_custom_overlays(_background* background);

    input_response input(const input_base& input);

    void add_immediate_overlay(const immediate_overlay& overlay);

    void add_overlay(std::unique_ptr<custom_overlay>&& overlay) {
        custom_overlays_.push_back(std::move(overlay));
    }

public:
    default_value<sfloat> blur_radius = sfloat(40.f);
    default_value<float> noise_scale = 0.55f;
    default_value<bool> blur_enabled = true;
};

void_end_