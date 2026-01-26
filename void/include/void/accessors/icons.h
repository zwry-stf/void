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
        std::uint32_t rect_id;
        scaled_icon icon_data;
        bool created{ false };
    };
    struct internal_created_icon {
        int resource_id;
        std::vector<internal_scaled_icon> sizes;
    };
    std::vector<internal_created_icon> icons_;

public:
    using vobj::vobj;

public:
    using icon_handle = std::uint32_t;
    inline static constexpr icon_handle kInvalidHandle = static_cast<icon_handle>(-1);

public:
    void init();
    void destroy();
    void on_scale_changed();

    [[nodiscard]] const scaled_icon* get_or_create(icon_handle handle, std::uint32_t size);
    [[nodiscard]] const scaled_icon* get_or_create(icon_handle handle, float size) {
        return get_or_create(
            handle,
            static_cast<std::uint32_t>(std::round(size))
        );
    }
    [[nodiscard]] icon_handle get_or_create_handle(int resource_id);

private:
    void create_icon(int resource_id, internal_scaled_icon& data);
};

void_end_