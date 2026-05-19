#include <void/accessors/icons.h>
#include <void/void.h>
#include <r2/font/font_atlas.h>
#include <void/util/error.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>
#include <stb_image.h>


void_begin_

error icons::init()
{
    auto& renderer = instance()->renderer();
    for (auto& i : icons_) {
        for (auto& s : i.sizes) {
            if (!s.created) {
                const auto res = create_icon(i.resource_id, s);
                if (res != error(error_code::none)) {
                    return res;
                }
            }
            s.icon_data.tex = renderer.font_texture();
        }
    }

    return error(error_code::none);
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

void icons::destroy_render()
{
    for (auto& i : icons_) {
        for (auto it = i.sizes.begin(); it != i.sizes.end();) {
            auto& s = *it;
            if (!s.created ||
                s.add_to_atlas) {
                it++;
                continue;
            }

            it = i.sizes.erase(it);
        }
    }
}

void icons::on_scale_changed()
{
    auto& renderer = instance()->renderer();
    auto* font_atlas = renderer.font_atlas();

    for (auto& i : icons_) {
        for (auto& s : i.sizes) {
            if (!s.created)
                continue;

            font_atlas->remove_rect(s.rect_id);
        }
        i.sizes.clear();
    }
}

const scaled_icon* icons::get_or_create(icon_handle handle, std::uint32_t size, bool add_to_atlas, bool ignore_size)
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif

    size = std::max<std::uint32_t>(size, 2u);

    assert(handle != kInvalidHandle);
    assert(handle < icons_.size());

    auto& created_icon = icons_[handle];
    {
        std::int32_t closest = instance()->options().get<options::option_MaxIconSizeDiff>();
        const scaled_icon* scaled = nullptr;
        for (auto& s : created_icon.sizes) {
            if (ignore_size) {
                return &s.icon_data;
            }
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
    icon.add_to_atlas = add_to_atlas;
    if (instance()->is_initialized()) {
        if (create_icon(created_icon.resource_id, icon) != error(error_code::none)) {
            instance()->options().get<options::option_CriticalErrorCallback>()();
            return nullptr;
        }
    }

    return &created_icon.sizes.emplace_back(std::move(icon)).icon_data;
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

error icons::create_icon(int resource_id, internal_scaled_icon& data, bool ignore_size)
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
        return error(error_code::load_icon);
    }

    int data_width;
    int data_height;
    if (ignore_size) {
        data_width = width;
        data_height = height;
        data.icon_data.size = static_cast<std::uint32_t>(height);
    }
    else {
        const float size_ratio = static_cast<float>(width) / static_cast<float>(height);
        data_width = static_cast<int>(static_cast<float>(data.icon_data.size) * size_ratio);
        data_height = static_cast<int>(data.icon_data.size);
    }

    if (data.add_to_atlas) {
        const auto rect_id = font_atlas->register_rect(
            static_cast<std::uint32_t>(data_width),
            static_cast<std::uint32_t>(data_height)
        );
        if (!rect_id) {
            return error(error_code::load_icon);
        }

        data.rect_id = *rect_id;
        font_atlas->get_rect_uv(
            data.rect_id,
            data.icon_data.uv_min,
            data.icon_data.uv_max
        );
        data.icon_data.tex = renderer.font_texture();
        data.created = true;
    }

    temp_buffer_.clear();
    temp_buffer_.resize(
        data_width * data_height
    );
    auto data_stride = data_width * 4;
    auto* dst_data = temp_buffer_.data();

    // resize
    unsigned char* _ret = stbir_resize_uint8_linear(
        pixels, width, height, width * 4,
        reinterpret_cast<std::uint8_t*>(dst_data), 
        data_width, data_height, data_stride,
        STBIR_RGBA
    );
    if (_ret == nullptr) {
        util.free_pixels(pixels);
        return error(error_code::load_icon);
    }

    util.free_pixels(pixels);

    if (!data.add_to_atlas) {
        r2::texture_desc tdesc{};
        tdesc.width = static_cast<std::uint32_t>(data_width);
        tdesc.height = static_cast<std::uint32_t>(data_height);
        tdesc.format = r2::texture_format::rgba8_unorm;

        data.texture = renderer.context()->create_texture2d(tdesc, dst_data);
        if (data.texture->has_error()) {
            return error(
                error_code::load_icon,
                data.texture->get_error(),
                data.texture->get_detail()
            );
        }

        r2::textureview_desc vdesc{};
        data.texture_view = renderer.context()->create_textureview(data.texture.get(), vdesc);
        if (data.texture_view->has_error()) {
            return error(
                error_code::load_icon,
                data.texture_view->get_error(), 
                data.texture_view->get_detail()
            );
        }
        
        data.icon_data.uv_min = r2::vec2(0.f);
        data.icon_data.uv_max = r2::vec2(1.f);
        data.icon_data.tex = data.texture_view->native_texture_handle();
        data.created = true;
    }
    else {
        font_atlas->write_data(
            data.rect_id,
            temp_buffer_.data(),
            temp_buffer_.size()
        );
        renderer.queue_atlas_update();
    }

    return error(error_code::none);
}

void_end_