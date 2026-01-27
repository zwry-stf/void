#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>


void_begin_

struct scaled_icon {
    r2::texture_handle tex;
    r2::vec2 uv_min, uv_max;
    std::uint32_t size;
};

class icons : protected vobj {
private:
    struct internal_scaled_icon {
        std::uint32_t rect_id = static_cast<std::uint32_t>(-1);
        scaled_icon icon_data;
        bool created{ false };
        bool add_to_atlas{ false };
        std::unique_ptr<r2::texture2d> texture;
        std::unique_ptr<r2::textureview> texture_view;
    };
    struct internal_created_icon {
        int resource_id;
        std::vector<internal_scaled_icon> sizes;
    };
    std::vector<internal_created_icon> icons_;
    std::vector<std::uint32_t> temp_buffer_;

public:
    using vobj::vobj;

public:
    using icon_handle = std::uint32_t;
    inline static constexpr icon_handle kInvalidHandle = static_cast<icon_handle>(-1);

public:
    void init();
    void destroy();
    void destroy_render();
    void on_scale_changed();

    [[nodiscard]] const scaled_icon* get_or_create(icon_handle handle, std::uint32_t size,
                                                   bool add_to_atlas = true, bool ignore_size = false);
    [[nodiscard]] const scaled_icon* get_or_create(icon_handle handle, float size, 
                                                   bool add_to_atlas = true, bool ignore_size = false) {
        return get_or_create(
            handle,
            static_cast<std::uint32_t>(std::round(size)),
            add_to_atlas,
            ignore_size
        );
    }
    [[nodiscard]] icon_handle get_or_create_handle(int resource_id);

private:
    void create_icon(int resource_id, internal_scaled_icon& data, bool ignore_size = false);
};

void_end_