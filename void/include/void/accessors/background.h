#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>
#include <void/util/default_value.h>
#include <void/util/scalable_float.h>


void_begin_

class background : protected vobj {
public:
    using vobj::vobj;

public:
    void do_blur_pass(const r2::vec4& area, r2::framebuffer* out_target, float radius,
                      r2::textureview* in_texture = nullptr /* if 0, backbuffer will be sampled (and potentially copied into offscreen tex first */);

    void add_immediate_overlay(const r2::rectf& rect, const float animation);

    [[nodiscard]] class _background* get_background_instance() const noexcept;
    [[nodiscard]] class _background_overlay* get_background_overlay_instance() const noexcept;

public:
    default_value<sfloat> blur_radius = sfloat(60.f);
    default_value<float> noise_scale = 0.55f;
    default_value<float> blend_amount = 0.8f;
    default_value<bool> blur_enabled = true;

    default_value<sfloat> overlay_blur_radius = sfloat(40.f);
    default_value<float> overlay_noise_scale = 0.55f;
    default_value<bool> overlay_blur_enabled = true;
};

void_end_