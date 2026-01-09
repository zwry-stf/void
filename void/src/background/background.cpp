#include "background.h"
#include <void/void.h>
#include <void/util/error.h>
#include <r2/render_data.h>

#if defined(R2_BACKEND_D3D11)
#include <backend/d3d11/context.h>
#include <backend/d3d11/texture2d.h>
#endif


void_begin_

void _background::init()
{
    // check if downsample value is valid
#if defined(_DEBUG)
    float v = 1.f;
    bool found = false;
    for (int i = 0; i < 5; i++) {
        if (instance()->options().get<options::option_DownsampleValue>() == v) {
            found = true;
            break;
        }

        v *= 0.5f;
    }

    assert(found && "invalid downsample value");
#endif

    init_targets();

    auto& renderer = instance()->renderer();
    auto* ctx = renderer.context();
    
    /// vertex Shader
    auto vs_res = instance()->resources().load_resource(
        void_resources::BackgroundVertexShader_shader);

    r2::vertex_attribute_desc vs_desc[] = {
        { "POSITION", r2::vertex_attribute_format::f32f32, offsetof(r2::vec4, x), false, 0 },
        { "TEXCOORD", r2::vertex_attribute_format::f32f32, offsetof(r2::vec4, z), false, 0 },
    };

    auto compiled_vs = ctx->compile_vertexshader(
        reinterpret_cast<const char*>(vs_res.data()), vs_res.size()
    );
    if (compiled_vs->has_error())
        throw error(error_code::background_init, 
            compiled_vs->get_error(), compiled_vs->get_detail());

    data_blur_vs_ = ctx->create_vertexshader(compiled_vs.get());
    if (data_blur_vs_->has_error())
        throw error(error_code::background_init, 
            data_blur_vs_->get_error(), data_blur_vs_->get_detail());

    data_inputlayout_ = ctx->create_inputlayout(
        vs_desc, static_cast<std::uint32_t>(sizeof(vs_desc) / sizeof(vs_desc[0])),
        compiled_vs->data(), compiled_vs->size()
    );
    if (data_inputlayout_->has_error())
        throw error(error_code::background_init, 
            data_inputlayout_->get_error(), data_inputlayout_->get_detail());

    /// composition pixel shader
    auto ps_comp_res = instance()->resources().load_resource(
        void_resources::BackgroundComposition_shader);
    auto compiled_ps = ctx->compile_pixelshader(
        reinterpret_cast<const char*>(ps_comp_res.data()), ps_comp_res.size()
    );
    if (compiled_ps->has_error())
        throw error(error_code::background_init,
            compiled_ps->get_error(), compiled_ps->get_detail());

    auto ps = ctx->create_pixelshader(compiled_ps.get());
    if (ps->has_error())
        throw error(error_code::background_init,
            ps->get_error(), ps->get_detail());

    data_shader_composition_ = ctx->create_shaderprogram(
        data_blur_vs_.get(), ps.get()
    );
    if (data_shader_composition_->has_error())
        throw error(error_code::background_init, 
            data_shader_composition_->get_error(), data_shader_composition_->get_detail());

    if (!instance()->options().get<options::option_NoBlur>()) {
        /// blur pixel shader
        auto ps_blur_res = instance()->resources().load_resource(
            void_resources::BackgroundBlur_shader);
        compiled_ps = ctx->compile_pixelshader(
            reinterpret_cast<const char*>(ps_blur_res.data()), ps_blur_res.size()
        );
        if (compiled_ps->has_error())
            throw error(error_code::background_init,
                compiled_ps->get_error(), compiled_ps->get_detail());

        ps = ctx->create_pixelshader(compiled_ps.get());
        if (ps->has_error())
            throw error(error_code::background_init,
                ps->get_error(), ps->get_detail());

        data_blur_shader_ = ctx->create_shaderprogram(
            data_blur_vs_.get(), ps.get()
        );
        if (data_blur_shader_->has_error())
            throw error(error_code::background_init, 
                data_blur_shader_->get_error(), data_blur_shader_->get_detail());

        /// downsample pixel shader
        auto ps_ds_res = instance()->resources().load_resource(
            void_resources::BackgroundDownsample_shader);
        compiled_ps = ctx->compile_pixelshader(
            reinterpret_cast<const char*>(ps_ds_res.data()), ps_ds_res.size()
        );
        if (compiled_ps->has_error())
            throw error(error_code::background_init,
                compiled_ps->get_error(), compiled_ps->get_detail());

        ps = ctx->create_pixelshader(compiled_ps.get());
        if (ps->has_error())
            throw error(error_code::background_init,
                ps->get_error(), ps->get_detail());

        data_shader_downsample_ = ctx->create_shaderprogram(
            data_blur_vs_.get(), ps.get()
        );
        if (data_shader_downsample_->has_error())
            throw error(error_code::background_init, 
                data_shader_downsample_->get_error(), data_shader_downsample_->get_detail());
    }

    /// create constant buffer
    r2::buffer_desc cb_desc;
    cb_desc.dynamic = true;
    cb_desc.usage   = r2::buffer_usage::uniform;
    cb_desc.size_bytes = sizeof(_shader_constants);

    data_constant_buffer_ = ctx->create_buffer(cb_desc);
    if (data_constant_buffer_->has_error())
        throw error(error_code::background_init,
            data_constant_buffer_->get_error(), data_constant_buffer_->get_detail());

    if (!instance()->options().get<options::option_NoBlur>()) {
        cb_desc.size_bytes = sizeof(blur_shader_constants_);
        data_blur_constant_buffer_ = ctx->create_buffer(cb_desc);
        if (data_blur_constant_buffer_->has_error())
            throw error(error_code::background_init, 
                data_blur_constant_buffer_->get_error(), data_blur_constant_buffer_->get_detail());

        cb_desc.size_bytes = sizeof(r2::vec4);
        data_cb_downsample_ = ctx->create_buffer(cb_desc);
        if (data_cb_downsample_->has_error())
            throw error(error_code::background_init, 
                data_cb_downsample_->get_error(), data_cb_downsample_->get_detail());
    }

    /// create vertex buffer
    // x and y are pos, z and w are uv
    const r2::vec4 vertices_full[] = {
        { -1.0f, -1.f, 0.0f, 1.0f },
        { -1.0f,  1.f, 0.0f, 0.0f },
        {  1.f,  -1.f, 1.0f, 1.0f },
        {  1.f,   1.f, 1.0f, 0.0f },
    };

    r2::buffer_desc vb_desc;
    vb_desc.dynamic = false;
    vb_desc.usage = r2::buffer_usage::vertex;
    vb_desc.size_bytes = sizeof(vertices_full);
    vb_desc.vb_stride = sizeof(r2::vec4);

    data_quad_vb_full_ = ctx->create_buffer(vb_desc, vertices_full);
    if (data_quad_vb_full_->has_error())
        throw error(error_code::background_init,
            data_quad_vb_full_->get_error(), data_quad_vb_->get_detail());

    data_inputlayout_->link(data_quad_vb_full_.get());

    if (!instance()->options().get<options::option_NoBlur>()) {
        r2::vec2 uv = (scaled_size_ / renderer.get_render_size()) *
            r2::vec2(2.f) - r2::vec2(1.f);

        const r2::vec4 vertices[] = {
            { -1.0f, -uv.y, 0.0f, 1.0f },
            { -1.0f,   1.f, 0.0f, 0.0f },
            {  uv.x, -uv.y, 1.0f, 1.0f },
            {  uv.x,   1.f, 1.0f, 0.0f },
        };

        vb_desc.size_bytes = sizeof(vertices);
        data_quad_vb_ = ctx->create_buffer(vb_desc, vertices);
        if (data_quad_vb_->has_error())
            throw error(error_code::background_init,
                data_quad_vb_->get_error(), data_quad_vb_->get_detail());

        data_inputlayout_->link(data_quad_vb_.get());
    }

    /// create index buffer
    const std::uint16_t indices[] = {
        0u, 1u, 2u,
        2u, 1u, 3u
    };

    r2::buffer_desc ib_desc;
    ib_desc.dynamic = false;
    ib_desc.size_bytes = sizeof(indices);
    ib_desc.usage = r2::buffer_usage::index;
    ib_desc.ib_type = r2::index_buffer_type::u16;
    data_quad_ib_ = ctx->create_buffer(ib_desc, indices);
    if (data_quad_ib_->has_error())
        throw error(error_code::background_init, 
            data_quad_ib_->get_error(), data_quad_ib_->get_detail());
}

void _background::destroy()
{
    pre_resize();

    data_blur_vs_.reset();
    data_shader_composition_.reset();
    data_shader_downsample_.reset();
    data_blur_shader_.reset();

    data_quad_vb_.reset();
    data_quad_vb_full_.reset();
    data_quad_ib_.reset();
    data_cb_downsample_.reset();
    data_constant_buffer_.reset();
    data_blur_constant_buffer_.reset();
}

void _background::render()
{
    auto& renderer = instance()->renderer();
    auto& render_target = instance()->render_target();
    auto* ctx = renderer.context();
    auto& bg = instance()->background();

    /// blur passes
    if (!instance()->options().get<options::option_NoBlur>() &&
        bg.blur_enabled() != 0) {
        const auto& menu_pos = instance()->pos();

        do_blur_pass(
            r2::vec4(menu_pos.x, menu_pos.y,
                menu_pos.x + menu_pos.w, menu_pos.y + menu_pos.h),
            data_pass_fbo_.get(),
            bg.blur_radius->get(instance()->scale()),
            &blur_shader_constants_
        );
    }
    else {
        ctx->set_inputlayout(data_inputlayout_.get());
        ctx->set_index_buffer(data_quad_ib_.get());
    }

    /// menu composition pass
    _shader_constants data;
    update_constant_buffer(&data);
    data_constant_buffer_->update(&data, sizeof(data));
    assert(!data_constant_buffer_->has_error());

    render_target.bind_menu();

    ctx->set_texture(data_pass_view_.get());
    ctx->set_vertex_buffer(data_quad_vb_full_.get());
    ctx->set_shaderprogram(data_shader_composition_.get());

    const auto& menu_pos = instance()->pos();
    const float ssize = bg.shadow_size->get(instance()->scale());
    
    ctx->set_scissor_rect(
        {
            static_cast<std::int32_t>(std::floor(menu_pos.x - ssize)),
            static_cast<std::int32_t>(std::floor(menu_pos.y - ssize)),
            static_cast<std::int32_t>(std::ceil(menu_pos.x + menu_pos.w + ssize)),
            static_cast<std::int32_t>(std::ceil(menu_pos.y + menu_pos.h + ssize))
        }
    );

    ctx->set_uniform_buffer(
        data_constant_buffer_.get(),
        r2::shader_bind_type::ps, 1u
    );

    ctx->draw_indexed(6u);
}

void _background::pre_resize()
{
    data_offscreen_tex_.reset();
    data_offscreen_view_.reset();
    data_offscreen_fbo_.reset();
    data_pass_tex_.reset();
    data_pass_view_.reset();
    data_pass_fbo_.reset();
    data_second_pass_tex_.reset();
    data_second_pass_view_.reset();
    data_second_pass_fbo_.reset();
}

void _background::post_resize()
{
    init_targets();
}

void _background::do_blur_pass(const r2::vec4& area, r2::framebuffer* out_target, float rradius,
                               blur_shader_constants* constants, r2::textureview* in_texture)
{
    blur_shader_constants local_constants{};
    blur_shader_constants* blur_constants = constants == nullptr ? &local_constants : constants;

    const float radius_f = rradius * instance()->options().get<options::option_DownsampleValue>();
    std::uint32_t radius = static_cast<std::uint32_t>(std::round(radius_f));

    update_blur_constant_buffer(radius, blur_constants);

    auto& renderer = instance()->renderer();
    auto& render_target = instance()->render_target();
    auto* ctx = renderer.context();

    ctx->set_inputlayout(data_inputlayout_.get());
    ctx->set_index_buffer(data_quad_ib_.get());
    ctx->set_vertex_buffer(data_quad_vb_.get());

    if (in_texture == nullptr) {
        if (!data_use_backbuffer_) {
            const std::int32_t offset = static_cast<std::int32_t>(std::ceil(rradius));

            const r2::rect blur_rect = {
                static_cast<std::int32_t>(std::floor(area.x)) - offset,
                static_cast<std::int32_t>(std::floor(area.y)) - offset,
                static_cast<std::int32_t>(std::ceil(area.z)) + offset,
                static_cast<std::int32_t>(std::ceil(area.w)) + offset
            };

            if (needs_resolve_)
                ctx->resolve_subresource(
                    data_offscreen_fbo_.get(),
                    render_target.main_fbo(),
                    std::nullopt,
                    blur_rect, blur_rect
                );
            else
                ctx->copy_subresource(
                    data_offscreen_fbo_.get(),
                    render_target.main_fbo(),
                    blur_rect, blur_rect
                );
        }

        in_texture = data_offscreen_view_.get();
    }

    // downsample
    r2::vec2 data = r2::vec2(1.f) / renderer.get_render_size();
    data_cb_downsample_->update(&data, sizeof(data));
    assert(!data_cb_downsample_->has_error());

    ctx->set_texture(nullptr);
    ctx->set_framebuffer(data_pass_fbo_.get());
    ctx->set_uniform_buffer(
        data_cb_downsample_.get(),
        r2::shader_bind_type::ps, 1u
    );
    ctx->set_shaderprogram(data_shader_downsample_.get());
    ctx->set_texture(in_texture);

    const std::int32_t offset = static_cast<std::int32_t>(radius);
    ctx->set_scissor_rect(
        {
            static_cast<std::int32_t>(std::floor(area.x *
                instance()->options().get<options::option_DownsampleValue>())) - offset,
            static_cast<std::int32_t>(std::floor(area.y *
                instance()->options().get<options::option_DownsampleValue>())) - offset,
            static_cast<std::int32_t>(std::ceil(area.z *
                instance()->options().get<options::option_DownsampleValue>())) + offset,
            static_cast<std::int32_t>(std::ceil(area.w *
                instance()->options().get<options::option_DownsampleValue>())) + offset
        }
    );

    ctx->draw_indexed(6u);

    // vertical blur
    blur_constants->dir = r2::vec2(0.f, 1.f / scaled_size_.y);
    data_blur_constant_buffer_->update(
        blur_constants, sizeof(*blur_constants));
    assert(!data_blur_constant_buffer_->has_error());

    ctx->set_texture(nullptr);
    ctx->set_framebuffer(data_second_pass_fbo_.get());
    ctx->set_shaderprogram(data_blur_shader_.get());
    ctx->set_texture(data_pass_view_.get());
    ctx->set_uniform_buffer(
        data_blur_constant_buffer_.get(),
        r2::shader_bind_type::ps, 1u
    );

    ctx->set_scissor_rect(
        {
            static_cast<std::int32_t>(std::floor(area.x *
                instance()->options().get<options::option_DownsampleValue>())) - offset,
            static_cast<std::int32_t>(std::floor(area.y *
                instance()->options().get<options::option_DownsampleValue>())),
            static_cast<std::int32_t>(std::ceil(area.z *
                instance()->options().get<options::option_DownsampleValue>())) + offset,
            static_cast<std::int32_t>(std::ceil(area.w *
                instance()->options().get<options::option_DownsampleValue>()))
        }
    );

    ctx->draw_indexed(6u);

    // horizontal blur
    blur_constants->dir = r2::vec2(1.f / scaled_size_.x, 0.f);
    data_blur_constant_buffer_->update(
        blur_constants, sizeof(blur_shader_constants));
    assert(!data_blur_constant_buffer_->has_error());

    ctx->set_texture(nullptr);
    ctx->set_framebuffer(out_target);
    ctx->set_texture(data_second_pass_view_.get());

    ctx->set_scissor_rect(
        {
            static_cast<std::int32_t>(std::floor(area.x*
                instance()->options().get<options::option_DownsampleValue>())),
            static_cast<std::int32_t>(std::floor(area.y*
                instance()->options().get<options::option_DownsampleValue>())),
            static_cast<std::int32_t>(std::ceil((area.z) *
                instance()->options().get<options::option_DownsampleValue>())),
            static_cast<std::int32_t>(std::ceil((area.w) *
                instance()->options().get<options::option_DownsampleValue>()))
        }
    );

    ctx->draw_indexed(6u);
}

void _background::restore_render_states() const noexcept
{
    auto& renderer = instance()->renderer();
    auto* ctx = renderer.context();
    const auto* render_data = renderer.render_data();

    ctx->set_inputlayout(render_data->input_layout.get());
    ctx->set_shaderprogram(render_data->shader.get());
    ctx->set_uniform_buffer(render_data->constant_buffer.get());
}

void _background::init_targets()
{
    data_use_backbuffer_ = false;

    auto& renderer = instance()->renderer();
    auto* ctx = renderer.context();
    auto& render_target = instance()->render_target();
    (void)render_target;

    if (!instance()->options().get<options::option_NoBlur>()) {
#if defined(R2_BACKEND_D3D11)
        // try to create resource view on backbuffer directly if supported
        {
            auto* context = r2::to_native(ctx);

            DXGI_SWAP_CHAIN_DESC d;
            context->get_swapchain()->GetDesc(&d);

            if (d.BufferUsage & DXGI_USAGE_SHADER_INPUT) {
                r2::textureview_desc view_desc{};
                view_desc.usage = r2::view_usage::shader_resource | r2::view_usage::render_target;
                data_offscreen_view_ = ctx->create_textureview(ctx->get_backbuffer(), view_desc);
                if (!data_offscreen_view_->has_error()) {
                    data_use_backbuffer_ = true;
                }
                else {
                    data_offscreen_view_.reset();
                }
            }
        }

        {
            auto* context = r2::to_native(ctx);
            auto* tex = r2::to_native(ctx->get_backbuffer())->texture();

            D3D11_TEXTURE2D_DESC d;
            tex->GetDesc(&d);

            if (d.SampleDesc.Count > 1u ||
                d.Format != context->get_format_no_srgb(d.Format)) { // format is srgb
                needs_resolve_ = true;
            }
            else {
                needs_resolve_ = false;
            }
        }
#elif defined(R2_BACKEND_OPENGL)
        needs_resolve_ = false;
#endif

        r2::texture_desc tex_desc{};
        tex_desc.width = static_cast<std::uint32_t>(renderer.get_render_size().x);
        tex_desc.height = static_cast<std::uint32_t>(renderer.get_render_size().y);
        tex_desc.format = r2::texture_format::backbuffer;
        tex_desc.usage = r2::texture_usage::render_target | r2::texture_usage::shader_resource;

        if (!data_use_backbuffer_) {
            data_offscreen_tex_ = ctx->create_texture2d(tex_desc);
            if (data_offscreen_tex_->has_error())
                throw error(error_code::background_init,
                    data_offscreen_tex_->get_error(), data_offscreen_tex_->get_detail());

            r2::textureview_desc view_desc{};
            view_desc.usage = r2::view_usage::shader_resource | r2::view_usage::render_target;
            data_offscreen_view_ = ctx->create_textureview(data_offscreen_tex_.get(), view_desc);
            if (data_offscreen_view_->has_error())
                throw error(error_code::background_init,
                    data_offscreen_view_->get_error(), data_offscreen_view_->get_detail());

            r2::framebuffer_desc fbo_desc{};
            fbo_desc.color_attachment.view = data_offscreen_view_.get();
            data_offscreen_fbo_ = ctx->create_framebuffer(fbo_desc);
            if (data_offscreen_fbo_->has_error())
                throw error(error_code::background_init,
                    data_offscreen_fbo_->get_error(), data_offscreen_fbo_->get_detail());
        }

        scaled_size_ = {
            std::ceil(renderer.get_render_size().x *
                instance()->options().get<options::option_DownsampleValue>()),
            std::ceil(renderer.get_render_size().y *
                instance()->options().get<options::option_DownsampleValue>()) 
        };
        tex_desc.width = static_cast<std::uint32_t>(scaled_size_.x);
        tex_desc.height = static_cast<std::uint32_t>(scaled_size_.y);

        data_pass_tex_ = ctx->create_texture2d(tex_desc);
        if (data_pass_tex_->has_error())
            throw error(error_code::background_init, 
                data_pass_tex_->get_error(), data_pass_tex_->get_detail());

        data_second_pass_tex_ = ctx->create_texture2d(tex_desc);
        if (data_second_pass_tex_->has_error())
            throw error(error_code::background_init,
                data_second_pass_tex_->get_error(), data_second_pass_tex_->get_detail());

        r2::textureview_desc view_desc{};
        view_desc.usage = r2::view_usage::shader_resource | r2::view_usage::render_target;
        data_pass_view_ = ctx->create_textureview(data_pass_tex_.get(), view_desc);
        if (data_pass_view_->has_error())
            throw error(error_code::background_init,
                data_pass_view_->get_error(), data_pass_view_->get_detail());

        data_second_pass_view_ = ctx->create_textureview(data_second_pass_tex_.get(), view_desc);
        if (data_second_pass_view_->has_error())
            throw error(error_code::background_init,
                data_second_pass_view_->get_error(), data_second_pass_view_->get_detail());

        r2::framebuffer_desc fbo_desc{};
        fbo_desc.color_attachment.view = data_pass_view_.get();
        data_pass_fbo_ = ctx->create_framebuffer(fbo_desc);
        if (data_pass_fbo_->has_error())
            throw error(error_code::background_init,
                data_pass_fbo_->get_error(), data_pass_fbo_->get_detail());

        fbo_desc.color_attachment.view = data_second_pass_view_.get();
        data_second_pass_fbo_ = ctx->create_framebuffer(fbo_desc);
        if (data_second_pass_fbo_->has_error())
            throw error(error_code::background_init,
                data_second_pass_fbo_->get_error(), data_second_pass_fbo_->get_detail());
    }
}

void _background::update_constant_buffer(_shader_constants* data)
{
    auto& bg = instance()->background();
    data->blur_enabled = !instance()->options().get<options::option_NoBlur>() && bg.blur_enabled();

    auto& style = instance()->style();

    data->resolution = instance()->renderer().get_render_size();
    data->menu_pos = instance()->pos();
    data->rounding = style.rounding->get(instance()->scale());
    data->menu_background = style.background();
    data->accent = style.border();
    data->border_size = style.border_size.get(instance()->scale());
    data->sidebar_width = style.sidebar_width.get(instance()->scale());
    data->shadow_size = bg.shadow_size->get(instance()->scale());
    data->shadow_color = bg.shadow_color();
    data->top_bar_height = style.top_bar_height.get(instance()->scale());
    data->noise_scale = bg.noise_scale();
    data->blend_amount = bg.blend_amount();
    data->down_scale = instance()->options().get<options::option_DownsampleValue>();
}

void _background::update_blur_constant_buffer(std::uint32_t& radius, blur_shader_constants* data)
{
    if (radius == 0)
        radius = 1;

    const std::uint32_t max_radius = (sizeof(data->gaussianp) /
        sizeof(data->gaussianp[0])) - 1u;
    if (radius > max_radius)
        radius = max_radius;

    const std::uint32_t samples = radius * 2u;

    if (data->samples != samples) {
        data->samples = samples;

        const float sigma = static_cast<float>(samples) * 0.3f;

        std::vector<float> weights(radius + 1u);
        for (std::uint32_t d = 0u; d <= radius; ++d) {
            const float x = static_cast<float>(d);
            const float t = x / sigma;
            weights[d] = std::exp(-0.5f * (t * t));
        }

        std::uint32_t part_count = 0u;

        data->gaussianp[part_count].weight = weights[0];
        data->gaussianp[part_count].offset = 0.f;
        ++part_count;

        for (std::uint32_t d = 1u; d + 1u <= radius; d += 2u) {
            auto& s = data->gaussianp[part_count++]; 
            const float w0 = weights[d];
            const float w1 = weights[d + 1u];
            const float wsum = w0 + w1;

            s.weight = wsum;
            s.offset = static_cast<float>(d) +
                ((wsum > 0.f) ? (w1 / wsum) : 0.5f);
        }

        if ((radius & 1u) != 0u) {
            auto& s = data->gaussianp[part_count++];
            s.weight = weights[radius];
            s.offset = static_cast<float>(radius);
        }

        float total_weight = data->gaussianp[0].weight;
        for (std::uint32_t i = 1u; i < part_count; ++i) {
            total_weight += 2.f * data->gaussianp[i].weight;
        }

        const float inv_total = (total_weight > 0.f) ? (1.f / total_weight) : 1.f;
        for (std::uint32_t i = 0u; i < part_count; ++i) {
            data->gaussianp[i].weight *= inv_total;
        }

        data->parts = part_count;
    }
}

void_end_