#include <void/accessors/render_target.h>
#include <void/void.h>
#include <void/util/error.h>
#include <background/background.h>
#if defined(R2_BACKEND_D3D11)
#include <backend/d3d11/context.h>
#endif


void_begin_

struct downsample_data {
    r2::vec2 resolution;
    float animation;
    float _pad;
};

error render_target::init()
{
    auto* ctx = instance()->renderer().context();
    auto* background = instance()->background().get_background_instance();

    if (!instance()->options().get<options::option_MenuMSAA>())
        return error(error_code::none); /* we'll use the 2d renderer instead */

    /// pixel shader
    auto ps_res = instance()->resources().load_resource(void_resources::MenuDownsample_shader);
    std::unique_ptr<r2::compiled_shader> cshader = ctx->compile_pixelshader(
        reinterpret_cast<const char*>(ps_res.data()), ps_res.size()
    );
    if (cshader->has_error()) {
        return error(
            error_code::render_target_init,
            cshader->get_error(), 
            cshader->get_detail()
        );
    }

    std::unique_ptr<r2::pixelshader> ps = ctx->create_pixelshader(cshader.get());
    if (ps->has_error()) {
        return  error(
            error_code::render_target_init,
            ps->get_error(),
            ps->get_detail()
        );
    }

    menu_shader_ = ctx->create_shaderprogram(
        background->data_blur_vs_.get(), 
        ps.get()
    );
    if (menu_shader_->has_error()) {
        return error(
            error_code::render_target_init,
            menu_shader_->get_error(), 
            menu_shader_->get_detail()
        );
    }

    /// constant buffer
    r2::buffer_desc bdesc{};
    bdesc.dynamic = true;
    bdesc.size_bytes = sizeof(downsample_data);
    bdesc.usage = r2::buffer_usage::uniform;

    menu_cb_ = ctx->create_buffer(bdesc);
    if (menu_cb_->has_error()) {
        return error(
            error_code::render_target_init,
            menu_cb_->get_error(),
            menu_cb_->get_detail()
        );
    }

    /// vertex buffer
    bdesc.size_bytes = sizeof(r2::vec4) * 4u;
    bdesc.usage = r2::buffer_usage::vertex;
    bdesc.vb_stride = sizeof(r2::vec4);

    menu_vb_ = ctx->create_buffer(bdesc);
    if (menu_vb_->has_error()) {
        return error(
            error_code::render_target_init,
            menu_vb_->get_error(), 
            menu_vb_->get_detail()
        );
    }

    background->data_inputlayout_->link(menu_vb_.get());

    return error(error_code::none);
}

void render_target::destroy()
{
    destroy_targets();

    menu_shader_.reset();
    menu_cb_.reset();
    menu_vb_.reset();
}

void render_target::destroy_targets()
{
    main_fbo_.reset();
    main_rtv_.reset();
    menu_tex_.reset();
    menu_view_.reset();
    menu_fbo_.reset();
}

void render_target::bind_main() const noexcept
{
    instance()->renderer().context()->set_framebuffer(
        main_fbo_.get()
    );
}

void render_target::clear_menu() const noexcept
{
    instance()->renderer().context()->clear_framebuffer(
        menu_fbo_.get()
    );
}

void render_target::bind_menu() const noexcept
{
    instance()->renderer().context()->set_framebuffer(
        menu_fbo_.get()
    );
}

void render_target::draw_menu() noexcept
{
    auto& renderer = instance()->renderer();

    const auto& menu_pos = instance()->pos();
    const auto render_size = renderer.get_render_size();

    constexpr float kAnimationValue = 0.15f;

    const r2::vec2 base_min = { menu_pos.x, menu_pos.y };
    const r2::vec2 base_max = { menu_pos.x + menu_pos.w, menu_pos.y + menu_pos.h };

    r2::vec2 uv_min = base_min / render_size;
    r2::vec2 uv_max = base_max / render_size;

    const r2::vec2 offset = r2::vec2{
        menu_pos.w * kAnimationValue * (1.f - instance()->animation()),
        menu_pos.h * kAnimationValue * (1.f - instance()->animation())
    };

    const r2::vec2 draw_min = base_min + offset;
    const r2::vec2 draw_max = base_max - offset;

    draw_menu_impl(draw_min, draw_max, uv_min, uv_max, instance()->alpha());
}

void render_target::draw_menu_impl(const r2::vec2& screen_min, const r2::vec2& screen_max, 
                                   const r2::vec2& uv_min, const r2::vec2& uv_max, float alpha) noexcept
{
    auto& renderer = instance()->renderer();
    auto* ctx = renderer.context();

    ctx->set_texture(nullptr);

    auto* background = instance()->background().get_background_instance();

    const auto render_size = renderer.get_render_size();

    if (instance()->options().get<options::option_MenuMSAA>()) {
        r2::vec4 vertices[4] = {
            { 0.f, 0.f, uv_min.x, uv_max.y },
            { 0.f, 0.f, uv_min.x, uv_min.y },
            { 0.f, 0.f, uv_max.x, uv_max.y },
            { 0.f, 0.f, uv_max.x, uv_min.y },
        };

        const float ndc_left = screen_min.x / render_size.x * 2.f - 1.f;
        const float ndc_right = screen_max.x / render_size.x * 2.f - 1.f;
        const float ndc_top = (1.f - screen_min.y / render_size.y) * 2.f - 1.f;
        const float ndc_bottom = (1.f - screen_max.y / render_size.y) * 2.f - 1.f;

        vertices[0].x = ndc_left;   vertices[0].y = ndc_bottom;
        vertices[1].x = ndc_left;   vertices[1].y = ndc_top;
        vertices[2].x = ndc_right;  vertices[2].y = ndc_bottom;
        vertices[3].x = ndc_right;  vertices[3].y = ndc_top;

        menu_vb_->update(vertices, sizeof(vertices));
        assert(!menu_vb_->has_error());

        downsample_data cdata;
        cdata.resolution = render_size;
        cdata.animation = alpha;
        menu_cb_->update(&cdata, sizeof(cdata));
        assert(!menu_cb_->has_error());

        ctx->set_inputlayout(background->data_inputlayout_.get());
        ctx->set_vertex_buffer(menu_vb_.get());
        ctx->set_index_buffer(background->data_quad_ib_.get());
        ctx->set_shaderprogram(menu_shader_.get());
        ctx->set_texture(menu_view_.get());
        ctx->set_uniform_buffer(
            menu_cb_.get(),
            r2::shader_bind_type::ps, 1u
        );

        ctx->set_scissor_rect(
            {
                static_cast<std::int32_t>(screen_min.x),
                static_cast<std::int32_t>(screen_min.y),
                static_cast<std::int32_t>(screen_max.x),
                static_cast<std::int32_t>(screen_max.y),
            }
        );

        ctx->draw_indexed(6u);

        background->restore_render_states();
    }
    else {
        r2::vec2 adjusted_uv_min = uv_min;
        r2::vec2 adjusted_uv_max = uv_max;
#if defined(R2_BACKEND_OPENGL)
        adjusted_uv_min.y = 1.f - adjusted_uv_min.y;
        adjusted_uv_max.y = 1.f - adjusted_uv_max.y;
#endif

        renderer.add_image(
            menu_view_->native_texture_handle(),
            screen_min,
            screen_max,
            r2::color::white().alpha(alpha),
            adjusted_uv_min,
            adjusted_uv_max
        );
        renderer.render();
        renderer.reset_render_data();
    }
}

error render_target::init_targets()
{
    auto& renderer = instance()->renderer();
    auto* ctx = renderer.context();

    r2::textureview_desc view_desc{};
    view_desc.usage = r2::view_usage::render_target;

    main_rtv_ = ctx->create_textureview(ctx->get_backbuffer(), view_desc);
    if (main_rtv_->has_error()) {
        return error(
            error_code::render_target_init,
            main_rtv_->get_error(), 
            main_rtv_->get_detail()
        );
    }

    r2::framebuffer_desc fbo_desc{};
    fbo_desc.color_attachment.view = main_rtv_.get();
    main_fbo_ = ctx->create_framebuffer(fbo_desc);
    if (main_fbo_->has_error()) {
        return error(
            error_code::render_target_init,
            main_fbo_->get_error(), 
            main_fbo_->get_detail()
        );
    }

    const std::uint32_t msaa_count = 1u;
    std::uint32_t msaa_quality = instance()->options().get<options::option_MenuMSAA>() ? 4u : 1u;
#if defined(R2_BACKEND_D3D11)
    auto* context = to_native(ctx);
    auto* device = context->get_device();
    device->CheckMultisampleQualityLevels(
        context->get_backbuffer_format_no_srgb(),
        msaa_count,
        &msaa_quality
    );
#elif defined(R2_BACKEND_OPENGL)
    msaa_quality = 1u;
#endif
    assert(msaa_quality > 0u);

    const auto render_size = renderer.get_render_size();

    r2::texture_desc tex_desc{};
    tex_desc.width = static_cast<std::uint32_t>(render_size.x);
    tex_desc.height = static_cast<std::uint32_t>(render_size.y);
    tex_desc.format = r2::texture_format::backbuffer;
    tex_desc.mip_levels = 1u;
    tex_desc.sample_desc.count = msaa_count;
    tex_desc.sample_desc.quality = msaa_quality - 1u;
    tex_desc.usage = r2::texture_usage::render_target | r2::texture_usage::shader_resource;

    menu_tex_ = ctx->create_texture2d(tex_desc);
    if (menu_tex_->has_error()) {
        return error(
            error_code::render_target_init,
            menu_tex_->get_error(),
            menu_tex_->get_detail()
        );
    }

    view_desc.usage = r2::view_usage::render_target | r2::view_usage::shader_resource;

    menu_view_ = ctx->create_textureview(menu_tex_.get(), view_desc);
    if (menu_view_->has_error()) {
        return error(
            error_code::render_target_init,
            menu_view_->get_error(), 
            menu_view_->get_detail()
        );
    }

    fbo_desc.color_attachment.view = menu_view_.get();
    menu_fbo_ = ctx->create_framebuffer(fbo_desc);
    if (menu_fbo_->has_error()) {
        return error(
            error_code::render_target_init,
            menu_fbo_->get_error(),
            menu_fbo_->get_detail()
        );
    }

    return error(error_code::none);
}

void_end_