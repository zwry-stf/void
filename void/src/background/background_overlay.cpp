#include "background_overlay.h"
#include <void/void.h>
#include <void/util/error.h>


void_begin_

void _background_overlay::init(_background* background)
{
    {
        auto overlay = std::make_unique<custom_overlay>(
            instance(), instance(),
            custom_overlay_cfg(), custom_overlay_data()
        );
        static bool overlay_enabled = true;
        overlay->cfg().liquid_glass = true;
        overlay->cfg().liquid_glass_curve = 1.f;
        overlay->set_render_callback([this](custom_overlay& overlay) -> void
            {
                auto& style = instance()->style();

                overlay.data().rounding_bottom =
                    overlay.data().rounding_top =
                    instance()->style().rounding->get(instance()->scale());
                overlay.data().border = instance()->style().border();
                overlay.data().background = instance()->style().overlay_background().transparent();

                overlay.cfg().liquid_glass_size.raw() = style.spacing->get(instance()->scale()) * 2.f;
                overlay.data().liquid_glass_color = r2::color::black().alpha(0.1f);

                constinit static float overlay_animation = 1.f;
                overlay_animation = instance()->util().lerp2(
                    overlay_animation, overlay_enabled);

                overlay.set_animation(overlay_animation);
            }
        );
        overlay->set_update_callback([this](custom_overlay& overlay) -> void
            {
                overlay.toggle_input(overlay_enabled);
            }
        );

        add_overlay(std::move(overlay));
    }

    auto& renderer = instance()->renderer();
    auto* ctx = renderer.context();

    /// composition shader
    auto ps_comp_res = instance()->resources().load_resource(
        void_resources::OverlayComposition_shader);
    auto compiled_ps = ctx->compile_pixelshader(
        reinterpret_cast<const char*>(ps_comp_res.data()), ps_comp_res.size()
    );
    if (compiled_ps->has_error())
        throw error(error_code::background_overlay_init,
            compiled_ps->get_error(), compiled_ps->get_detail());

    auto ps = ctx->create_pixelshader(compiled_ps.get());
    if (ps->has_error())
        throw error(error_code::background_overlay_init,
            ps->get_error(), ps->get_detail());

    data_shader_composition_ = ctx->create_shaderprogram(
        background->data_blur_vs_.get(), ps.get()
    );
    if (data_shader_composition_->has_error())
        throw error(error_code::background_overlay_init,
            data_shader_composition_->get_error(), data_shader_composition_->get_detail());

    /// liquid glass shader
    auto ps_lglass_res = instance()->resources().load_resource(
        void_resources::OverlayLiquidGlass_shader);
    compiled_ps = ctx->compile_pixelshader(
        reinterpret_cast<const char*>(ps_lglass_res.data()), ps_lglass_res.size()
    );
    if (compiled_ps->has_error())
        throw error(error_code::background_overlay_init,
            compiled_ps->get_error(), compiled_ps->get_detail());

    ps = ctx->create_pixelshader(compiled_ps.get());
    if (ps->has_error())
        throw error(error_code::background_overlay_init,
            ps->get_error(), ps->get_detail());

    data_shader_liquidglass_ = ctx->create_shaderprogram(
        background->data_blur_vs_.get(), ps.get()
    );
    if (data_shader_liquidglass_->has_error())
        throw error(error_code::background_overlay_init,
            data_shader_liquidglass_->get_error(), data_shader_liquidglass_->get_detail());

    /// constant buffer
    r2::buffer_desc cb_desc;
    cb_desc.dynamic = true;
    cb_desc.size_bytes = sizeof(_shader_constants);
    cb_desc.usage = r2::buffer_usage::uniform;

    data_constant_buffer_ = ctx->create_buffer(cb_desc);
    if (data_constant_buffer_->has_error())
        throw error(error_code::background_overlay_init,
            data_constant_buffer_->get_error(), data_constant_buffer_->get_detail());
}

void _background_overlay::destroy()
{
    data_shader_composition_.reset();
    data_shader_liquidglass_.reset();
    data_constant_buffer_.reset();
}

void _background_overlay::reset_data()
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif
    immediate_overlays_.clear();
}

void _background_overlay::render_custom_overlays(_background* background)
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto* ctx = renderer.context();

    /// custom overlays
    _shader_constants constants;

    constants.blur_enabled = !instance()->options().get<options::option_NoBlur>() && blur_enabled();
    constants.noise_scale = noise_scale();
    constants.resolution = renderer.get_render_size();

    bool first_overlay = true;
    for (auto& o : custom_overlays_) {
        o->update();
        o->render();

        constants.overlay.animation = o->animation_;
        constants.overlay.pos = o->last_pos_;
        constants.overlay.background = o->data_.background;
        constants.overlay.border = o->cfg_.liquid_glass ? 
            o->data_.liquid_glass_color : o->data_.border;
        constants.overlay.rounding_bottom = o->data_.rounding_bottom;
        constants.overlay.rounding_top = o->data_.rounding_top;
        constants.overlay.liquid_curve_value = o->cfg_.liquid_glass_curve;

        o->cfg_.liquid_glass ?
            constants.warp_size = o->cfg_.liquid_glass_size.get(instance()->scale()) :
            constants.border_size = style.border_size.get(instance()->scale());

        constants.blend_amount = background->blend_amount();
        data_constant_buffer_->update(&constants, sizeof(constants));
        assert(!data_constant_buffer_->has_error());

        /// blur passes
        const auto& overlay = constants.overlay;

        const r2::vec4 pos = r2::vec4{
            std::floor(overlay.pos.x),
            std::floor(overlay.pos.y),
            std::ceil(overlay.pos.x + overlay.pos.w),
            std::ceil(overlay.pos.y + overlay.pos.h)
        };

        r2::textureview* input_tex = nullptr;
        if ((!instance()->options().get<options::option_NoBlur>() &&
            blur_enabled() != 0) ||
            o->cfg_.liquid_glass)
        {
            background->do_blur_pass(
                pos,
                background->data_pass_fbo_.get(),
                blur_radius->get(instance()->scale()),
                &blur_shader_constants_,
                first_overlay ? nullptr : background->data_offscreen_view_.get() // only resolve backbuffer on first overlay to save a little performance
            );

            first_overlay = false;

            input_tex = background->data_pass_view_.get();
        }
        else {
            ctx->set_inputlayout(background->data_inputlayout_.get());
            ctx->set_index_buffer(background->data_quad_ib_.get());
        }

        /// composition pass
        instance()->render_target().bind_main();

        ctx->set_vertex_buffer(
            background->data_quad_vb_full_.get());
        o->cfg_.liquid_glass ?
            ctx->set_shaderprogram(data_shader_liquidglass_.get()) :
            ctx->set_shaderprogram(data_shader_composition_.get());
        ctx->set_texture(input_tex);
        ctx->set_uniform_buffer(
            data_constant_buffer_.get(),
            r2::shader_bind_type::ps, 1u
        );

        ctx->set_scissor_rect(
            {
                static_cast<std::int32_t>(pos.x),
                static_cast<std::int32_t>(pos.y),
                static_cast<std::int32_t>(pos.z),
                static_cast<std::int32_t>(pos.w)
            }
        );

        ctx->draw_indexed(6u);
    }
}

void _background_overlay::render(_background* background)
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    auto* ctx = renderer.context();

    /// immediate overlays
    _shader_constants constants;

    constants.blur_enabled = !instance()->options().get<options::option_NoBlur>() && blur_enabled();
    constants.noise_scale = noise_scale();
    constants.resolution = renderer.get_render_size();
    constants.overlay.background = style.overlay_background();
    constants.overlay.border = style.border();
    constants.overlay.rounding_bottom =
        constants.overlay.rounding_top = style.rounding->get(instance()->scale());
    constants.border_size = style.border_size.get(instance()->scale());

    bool first_overlay = true;
    for (auto& o : immediate_overlays_) {
        constants.overlay.animation = o.animation;
        constants.overlay.pos = o.pos;
        constants.blend_amount = background->blend_amount();
        data_constant_buffer_->update(&constants, sizeof(constants));
        assert(!data_constant_buffer_->has_error());

        /// blur passes
        const auto& overlay = constants.overlay;

        const r2::vec4 pos = r2::vec4{
            std::floor(overlay.pos.x),
            std::floor(overlay.pos.y),
            std::ceil(overlay.pos.x + overlay.pos.w),
            std::ceil(overlay.pos.y + overlay.pos.h)
        };

        r2::textureview* input_tex = nullptr;
        if (!instance()->options().get<options::option_NoBlur>() &&
            blur_enabled() != 0)
        {
            background->do_blur_pass(
                pos,
                background->data_pass_fbo_.get(),
                blur_radius->get(instance()->scale()),
                &blur_shader_constants_,
                first_overlay ? nullptr : background->data_offscreen_view_.get() // only resolve backbuffer on first overlay to save a little performance
            );

            first_overlay = false;

            input_tex = background->data_pass_view_.get();
        }
        else {
            ctx->set_inputlayout(background->data_inputlayout_.get());
            ctx->set_index_buffer(background->data_quad_ib_.get());
        }

        /// composition pass
        instance()->render_target().bind_main();

        ctx->set_vertex_buffer(
            background->data_quad_vb_full_.get());
        ctx->set_shaderprogram(data_shader_composition_.get());
        ctx->set_texture(input_tex);
        ctx->set_uniform_buffer(
            data_constant_buffer_.get(), 
            r2::shader_bind_type::ps, 1u
        );

        ctx->set_scissor_rect(
            {
                static_cast<std::int32_t>(pos.x),
                static_cast<std::int32_t>(pos.y),
                static_cast<std::int32_t>(pos.z),
                static_cast<std::int32_t>(pos.w)
            }
        );

        ctx->draw_indexed(6u);
    }
}

input_response _background_overlay::input(const input_base& input)
{
    for (auto& o : custom_overlays_) {
        auto res = o->input(input);
        if (res.is_handled())
            return res;
    }

    return input_response::empty();
}

void _background_overlay::add_immediate_overlay(const immediate_overlay& overlay)
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif
    immediate_overlays_.emplace_back(overlay);
}

void_end_