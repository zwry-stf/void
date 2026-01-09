#include <void/accessors/icons.h>
#include <void/void.h>
#include <r2/font/font_atlas.h>
#include <void/util/error.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
#include <stb_image.h>


void_begin_

void icons::init()
{
    auto& renderer = instance()->renderer();
    for (auto& i : icons_) {
        for (auto& s : i.sizes) {
            if (!s.created) {
                create_icon(i.resource_id, s);
            }
            s.icon_data.tex = renderer.font_texture();
        }
    }
}

void icons::destroy()
{
    auto& renderer = instance()->renderer();
    auto* font_atlas = renderer.font_atlas();

    for (auto& i : icons_) {
        for (auto& s : i.sizes) {
            if (!s.created)
                continue;

            font_atlas->remove_rect(s.rect_id);
        }
    }

    icons_.clear();
}

const scaled_icon* icons::get_or_create(icon_handle handle, std::uint32_t size)
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif

    assert(handle != kInvalidHandle);
    assert(handle < icons_.size());

    auto& created_icon = icons_[handle];
    {
        std::int32_t closest = instance()->options().get<options::option_MaxIconSizeDiff>();
        const scaled_icon* scaled = nullptr;
        for (auto& s : created_icon.sizes) {
            const std::int32_t off = std::abs(
                static_cast<std::int32_t>(s.icon_data.size) -
                static_cast<std::int32_t>(size)
            );

            if (off < closest) {
                scaled = &s.icon_data;
            }
        }
        if (scaled != nullptr) {
            return scaled;
        }
    }

    internal_scaled_icon icon;
    icon.icon_data.size = size;
    if (instance()->is_initialized()) {
        create_icon(created_icon.resource_id, icon);
    }

    return &created_icon.sizes.emplace_back(icon).icon_data;
}

icons::icon_handle icons::get_or_create_handle(int resource_id)
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif

    for (std::size_t i = 0u; i < icons_.size(); i++) {
        if (icons_[i].resource_id == resource_id)
            return static_cast<icon_handle>(i);
    }

    icons_.emplace_back(resource_id);

    return static_cast<icon_handle>(icons_.size() - 1u);
}

void icons::create_icon(int resource_id, internal_scaled_icon& data)
{
    assert(!data.created);

    auto& util = instance()->util();
    auto& renderer = instance()->renderer();
    auto* font_atlas = renderer.font_atlas();

    // load img
    auto res = instance()->resources().load_resource(resource_id);

    std::uint8_t* pixels;
    int width, height;
    if (!util.load_pixels(
        res.data(),
        res.size(),
        pixels,
        &width, &height
    )) {
        throw error(error_code::load_icon);
    }

    float size_ratio = static_cast<float>(width) / static_cast<float>(height);

    data.rect_id = font_atlas->register_rect(
        static_cast<std::uint32_t>(static_cast<float>(data.icon_data.size) * size_ratio),
        data.icon_data.size
    );
    font_atlas->get_rect_uv(
        data.rect_id,
        data.icon_data.uv_min,
        data.icon_data.uv_max
    );
    data.icon_data.tex = renderer.font_texture();
    data.created = true;

    const auto& r = font_atlas->get_rect(data.rect_id);

    auto* dst_pixels = font_atlas->get_data32().data();
    auto* dst_data = dst_pixels + r.pos_y * font_atlas->get_width() + r.pos_x;

    // resize
    unsigned char* _ret = stbir_resize_uint8_linear(
        pixels, width, height, width * 4,
        reinterpret_cast<std::uint8_t*>(dst_data), 
            static_cast<int>(r.width), static_cast<int>(r.height), 
            font_atlas->get_width() * sizeof(std::uint32_t),
        STBIR_RGBA
    );
    if (_ret == nullptr) {
        throw error(error_code::load_icon);
    }

    util.free_pixels(pixels);

    renderer.queue_atlas_update();
}

void_end_