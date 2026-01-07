#include <void/accessors/util.h>
#include <void/void.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif


void_begin_

bool util::load_pixels(const std::uint8_t* data, std::size_t size, std::uint8_t*& out_pixels, 
                       int* out_width, int* out_height)
{
    out_pixels = nullptr;

    int width, height, channels;
    stbi_uc* pixels = stbi_load_from_memory(
        data,
        static_cast<int>(size), 
        &width,
        &height,
        &channels, 
        4 /* dst num channels */
    );
    if (!pixels)
        return false;

    out_pixels = pixels;
    if (out_width)
        *out_width = width;
    if (out_height)
        *out_height = height;

    return true;
}

void util::free_pixels(std::uint8_t* pixels)
{
    stbi_image_free(pixels);
}

std::optional<r2::vec2> util::get_window_size() const noexcept
{
#if defined(R2_PLATFORM_WINDOWS)
    RECT rect;
    if (!GetClientRect(
        instance()->renderer().context()->get_hwnd(), 
        &rect))
        return std::nullopt;

    if (rect.right - rect.left <= 0 ||
        rect.bottom - rect.top <= 0)
        return std::nullopt;

    return r2::vec2(
        static_cast<float>(rect.right - rect.left),
        static_cast<float>(rect.bottom - rect.top)
    );
#endif
}

float util::lerp(float current, float target, float speed) const noexcept
{
    const float t = speed *
        instance()->delta_time() *
        instance()->style().animation_speed();
    return lerp_ex(
        current,
        target, 
        t
    );
}

float util::lerp(float current, bool target, float speed) const noexcept
{
    return lerp(
        current,
        target ? 1.f : 0.f,
        speed
    );
}

float util::lerp2(float current, float target, float speed) const noexcept
{
    assert(current >= 0.f && current <= 1.f);
    float t = (std::min)(speed *
        instance()->style().animation_speed() *
        instance()->delta_time(), 1.f
    );

    t = t * (2.f - t);

    return lerp_ex(
        current, 
        target, 
        t
    );
}

float util::lerp2(float current, bool target, float speed) const noexcept
{
    return lerp2(
        current,
        target ? 1.f : 0.f,
        speed
    );
}

r2::color util::disable_color(const r2::color& col, const float animation) const noexcept
{
    assert(animation >= 0.f && animation <= 1.f);

    auto ret = col.interp(
        instance()->style().disabled(),
        animation * 0.7f
    );
    ret.a = col.a;
    return ret;
}

void_end_