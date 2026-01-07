#include <void/accessors/background_access.h>
#include <void/void.h>
#include <background/background.h>
#include <background/background_overlay.h>


void_begin_

void background::do_blur_pass(const r2::vec4& area, r2::framebuffer* out_target, float radius, r2::textureview* in_texture)
{
    get_background_instance()->do_blur_pass(area, out_target, radius, nullptr, in_texture);
}

void background::add_immediate_overlay(const r2::rectf& rect, const float animation)
{
    get_background_overlay_instance()->add_immediate_overlay(
        immediate_overlay{ rect, animation }
    );
}

_background* background::get_background_instance() const noexcept
{
    return instance()->background_.get();
}

_background_overlay* background::get_background_overlay_instance() const noexcept
{
    return instance()->background_overlay_.get();
}

void_end_