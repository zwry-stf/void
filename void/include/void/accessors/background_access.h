#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>


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
};

void_end_