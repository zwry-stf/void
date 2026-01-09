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

void render_target::init()
{
    auto* ctx = instance()->renderer().context();
    auto* background = instance()->background().get_background_instance();

    if (!instance()->options().get<options::option_MenuMSAA>())
        return; /* we'll use the 2d renderer instead */

    /// pixel shader
    auto ps_res = instance()->resources().load_resource(void_resources::MenuDownsample_shader);
    std::unique_ptr<r2::compiled_shader> cshader = ctx->compile_pixelshader(
        reinterpret_cast<const char*>(ps_res.data()), ps_res.size()
    );
    if (cshader->has_error())
        throw error(error_code::render_target_init,
            cshader->get_error(), cshader->get_detail());

    std::unique_ptr<r2::pixelshader> ps = ctx->create_pixelshader(cshader.get());
    if (ps->has_error())
        throw error(error_code::render_target_init, 
            ps->get_error(), ps->get_detail());

    menu_shader_ = ctx->create_shaderprogram(
        background->data_blur_vs_.get(), 
        ps.get()
    );
    if (menu_shader_->has_error())
        throw error(error_code::render_target_init,
            menu_shader_->get_error(), menu_shader_->get_detail());

    /// constant buffer
    r2::buffer_desc bdesc{};
    bdesc.dynamic = true;
    bdesc.size_bytes = sizeof(downsample_data);
    bdesc.usage = r2::buffer_usage::uniform;

    menu_cb_ = ctx->create_buffer(bdesc);
    if (menu_cb_->has_error())
        throw error(error_code::render_target_init,
            menu_cb_->get_error(), menu_cb_->get_detail());

    /// vertex buffer
    bdesc.size_bytes = sizeof(r2::vec4) * 4u;
    bdesc.usage = r2::buffer_usage::vertex;
    bdesc.vb_stride = sizeof(r2::vec4);

    menu_vb_ = ctx->create_buffer(bdesc);
    if (menu_vb_->has_error())
        throw error(error_code::render_target_init,
            menu_vb_->get_error(), menu_vb_->get_detail());

    background->data_inputlayout_->link(menu_vb_.get());
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
    auto* ctx = renderer.context();

    ctx->set_texture(nullptr);

    auto* background = instance()->background().get_background_instance();

    const float shadow_size = instance()->background().shadow_size->get(instance()->scale());
    const auto& menu_pos = instance()->pos();

    const auto render_size = renderer.get_render_size();

    constexpr float kAnimationValue = 0.15f;
    const r2::vec2 min = { 
        menu_pos.x - shadow_size, 
        menu_pos.y - shadow_size
    };
    const r2::vec2 max = { 
        menu_pos.x + menu_pos.w + shadow_size, 
        menu_pos.y + menu_pos.h + shadow_size 
    };

    r2::vec2 uv_min = min / render_size;
    r2::vec2 uv_max = max / render_size;

    const r2::vec2 offset = r2::vec2{
        menu_pos.w * kAnimationValue * (1.f - instance()->animation()),
        menu_pos.h * kAnimationValue * (1.f - instance()->animation())
    };

    if (instance()->options().get<options::option_MenuMSAA>()) {
        r2::vec4 vertices[4] = {
            { 0.f, 0.f, uv_min.x, uv_max.y },
            { 0.f, 0.f, uv_min.x, uv_min.y },
            { 0.f, 0.f, uv_max.x, uv_max.y },
            { 0.f, 0.f, uv_max.x, uv_min.y },
        };
        const float anim_x = offset.x / render_size.x * 2.f;
        const float anim_y = offset.y / render_size.y * 2.f;
        for (auto& v : vertices) {
            v.x = v.z * 2.f - 1.f;
            v.y = (1.f - v.w) * 2.f - 1.f;

            v.x += (v.z < uv_max.x ? +anim_x : -anim_x);
            v.y += (v.w > uv_min.y ? +anim_y : -anim_y);
        }

        menu_vb_->update(vertices, sizeof(vertices));
        assert(!menu_vb_->has_error());

        downsample_data cdata;
        cdata.resolution = render_size;
        cdata.animation = instance()->alpha();
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
                static_cast<std::int32_t>(min.x),
                static_cast<std::int32_t>(min.y),
                static_cast<std::int32_t>(max.x),
                static_cast<std::int32_t>(max.y),
            }
        );

        ctx->draw_indexed(6u);

        background->restore_render_states();
    }
    else {
#if defined(R2_BACKEND_OPENGL)
        uv_min.y = 1.f - uv_min.y;
        uv_max.y = 1.f - uv_max.y;
#endif
        renderer.add_image(
            menu_view_->native_texture_handle(),
            min + offset,
            max - offset,
            r2::color::white().alpha(instance()->alpha()),
            uv_min, uv_max
        );
        renderer.render();
        renderer.reset_render_data();
    }
}

void render_target::init_targets()
{
    auto& renderer = instance()->renderer();
    auto* ctx = renderer.context();

    r2::textureview_desc view_desc{};
    view_desc.usage = r2::view_usage::render_target;

    main_rtv_ = ctx->create_textureview(ctx->get_backbuffer(), view_desc);
    if (main_rtv_->has_error())
        throw error(error_code::render_target_init,
            main_rtv_->get_error(), main_rtv_->get_detail());

    r2::framebuffer_desc fbo_desc{};
    fbo_desc.color_attachment.view = main_rtv_.get();
    main_fbo_ = ctx->create_framebuffer(fbo_desc);
    if (main_fbo_->has_error())
        throw error(error_code::render_target_init,
            main_fbo_->get_error(), main_fbo_->get_detail());

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
    if (menu_tex_->has_error())
        throw error(error_code::render_target_init, 
            menu_tex_->get_error(), menu_tex_->get_detail());

    view_desc.usage = r2::view_usage::render_target | r2::view_usage::shader_resource;

    menu_view_ = ctx->create_textureview(menu_tex_.get(), view_desc);
    if (menu_view_->has_error())
        throw error(error_code::render_target_init,
            menu_view_->get_error(), menu_view_->get_detail());

    fbo_desc.color_attachment.view = menu_view_.get();
    menu_fbo_ = ctx->create_framebuffer(fbo_desc);
    if (menu_fbo_->has_error())
        throw error(error_code::render_target_init,
            menu_fbo_->get_error(), menu_fbo_->get_detail());
}

void_end_