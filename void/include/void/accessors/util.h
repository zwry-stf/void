#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>
#include <optional>
#include <cstdint>
#include <cstddef>


void_begin_

class util : protected vobj {
private:

public:
    using vobj::vobj;

public:
    bool load_pixels(const std::uint8_t* data, std::size_t size, std::uint8_t*& out_pixels,
                     int* out_width = nullptr, int* out_height = nullptr);
    void free_pixels(std::uint8_t* pixels);

    [[nodiscard]] std::optional<r2::vec2> get_window_size() const noexcept;

    [[nodiscard]] float lerp(float current, float target, float speed = 1.f) const noexcept;
    [[nodiscard]] float lerp(float current, bool target, float speed = 1.f) const noexcept;
    [[nodiscard]] float lerp2(float current, float target, float speed = 1.f) const noexcept;
    [[nodiscard]] float lerp2(float current, bool target, float speed = 1.f) const noexcept;
    [[nodiscard]] v_always_inline float lerp_ex(float current, float target, float t) const noexcept {
        return current + (target - current) * (std::min)(t, 1.f);
    }
    [[nodiscard]] r2::color disable_color(const r2::color& col, const float animation) const noexcept;
    
public:
    inline static constexpr float g_min_alpha = 1.f / 255.f;

    [[nodiscard]] v_always_inline static constexpr bool is_in_rect(float posx, float posy, float x, float y, 
                                                                   float w, float h) noexcept {
        return (posx >= x && posx <= x + w && posy >= y && posy <= y + h);
    }

    [[nodiscard]] v_always_inline static constexpr bool is_in_rect(float posx, float posy,
                                                                   const r2::rectf& p) noexcept {
        return (posx >= p.x && posx <= p.x + p.w && posy >= p.y && posy <= p.y + p.h);
    }

    [[nodiscard]] v_always_inline static constexpr bool is_in_quad(float posx, float posy, float x, float y,
                                                                   float x2, float y2) noexcept {
        return (posx >= x && posx <= x2 && posy >= y && posy <= y2);
    }

    [[nodiscard]] v_always_inline static constexpr bool is_in_quad(float posx, float posy, 
                                                                   const r2::vec4& p) noexcept {
        return (posx >= p.x && posx <= p.z && posy >= p.y && posy <= p.w);
    }

    [[nodiscard]] v_always_inline static float round_to_even(float v) noexcept {
        return std::round(v * 0.5f) * 2.f;
    }
};

void_end_