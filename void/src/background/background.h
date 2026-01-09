#pragma once
#include <void/util/vobj.h>
#include <void/util/default_value.h>
#include <void/util/scalable_float.h>
#include <r2/renderer_definitions.h>


void_begin_

class _background : protected vobj {
public:
    using vobj::vobj;

    struct blur_shader_constants {
        std::uint32_t samples = static_cast<std::uint32_t>(-1);
        std::uint32_t parts;
        r2::vec2 dir;

        // gaussian weights
        struct blur_sample {
            float weight;
            float offset;
            float pad[2];
        } gaussianp[51];
    };

private:
    std::unique_ptr<r2::texture2d> data_offscreen_tex_;
    std::unique_ptr<r2::textureview> data_offscreen_view_;
    std::unique_ptr<r2::framebuffer> data_offscreen_fbo_;
    std::unique_ptr<r2::texture2d> data_pass_tex_;
    std::unique_ptr<r2::textureview> data_pass_view_;
    std::unique_ptr<r2::framebuffer> data_pass_fbo_;
    std::unique_ptr<r2::texture2d> data_second_pass_tex_;
    std::unique_ptr<r2::textureview> data_second_pass_view_;
    std::unique_ptr<r2::framebuffer> data_second_pass_fbo_;

    std::unique_ptr<r2::shaderprogram> data_shader_downsample_;
    std::unique_ptr<r2::shaderprogram> data_blur_shader_;
    std::unique_ptr<r2::shaderprogram> data_shader_composition_;

    std::unique_ptr<r2::vertexshader> data_blur_vs_;
    std::unique_ptr<r2::inputlayout>  data_inputlayout_;

    std::unique_ptr<r2::buffer> data_quad_vb_;
    std::unique_ptr<r2::buffer> data_quad_vb_full_;
    std::unique_ptr<r2::buffer> data_quad_ib_;
    std::unique_ptr<r2::buffer> data_cb_downsample_;
    std::unique_ptr<r2::buffer> data_constant_buffer_;
    std::unique_ptr<r2::buffer> data_blur_constant_buffer_;

    blur_shader_constants blur_shader_constants_{};
    bool data_use_backbuffer_;
    bool needs_resolve_;
    r2::vec2 scaled_size_;

    friend class _background_overlay;
    friend class render_target;

    struct _shader_constants {
        r2::rectf menu_pos;

        // colors
        r2::color menu_background;
        r2::color accent;

        // globals
        r2::vec2 resolution;
        float _pad0[2];

        float rounding;
        float border_size;
        float sidebar_width;
        float top_bar_height;

        // blur
        std::uint32_t blur_enabled;
        float noise_scale;
        float down_scale;
        float blend_amount;
    };

public:
    void init();
    void destroy();
    void render();
    void pre_resize();
    void post_resize();

    void do_blur_pass(const r2::vec4& area, r2::framebuffer* out_target, float radius,
                      blur_shader_constants* constants = nullptr, r2::textureview* in_texture = nullptr);

    void restore_render_states() const noexcept;

private:
    void init_targets();
    void update_constant_buffer(_shader_constants* data);
    void update_blur_constant_buffer(std::uint32_t& radius, blur_shader_constants* data);
};

void_end_