#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <r2/renderer_definitions.h>
#include <void/util/time_point.h>
#include <void/contents/input/events.h>
#include <void/contents/input/input_response.h>
#include <string>
#include <functional>


void_begin_

enum class textfield_flags : std::uint8_t {
    none = 0u,
    stop_on_return = (1u << 0u),
    mouse_in_rect  = (1u << 1u), // whether client rect should be the whole area or only on text
    movable_caret  = (1u << 2u),
    faded_text     = (1u << 3u),
};

enum class textfield_type : std::uint8_t {
    text,
    color,
    integer,
    decimal
};

inline constexpr textfield_flags operator|(const textfield_flags& a, const textfield_flags& b) noexcept {
    return static_cast<textfield_flags>(
        std::to_underlying(a) | std::to_underlying(b)
    );
}
inline constexpr bool operator&(const textfield_flags& a, const textfield_flags& b) noexcept {
    return static_cast<textfield_flags>(std::to_underlying(a) & std::to_underlying(b)) == b;
}

class textfield : private vobj,
                  public input_receiver {
private:
    const xstr default_text_;
    const xstr post_text_;
    const std::size_t max_length_;
    const textfield_flags flags_;
    const textfield_type type_;

    std::u32string buffer_;

    r2::rectf last_pos_;
    bool text_selected_{ false };
    bool caret_moving_{ false };
    std::int32_t typing_pos_{ 0 };
    std::int32_t last_typing_pos_{ 0 };
    std::int32_t selected_start_pos_{ 0 };
    float selected_start_render_pos_{ 0.f };
    float caret_click_pos_;
    bool need_caret_pos_{ false };
    float animated_caret_pos_{ 0.f };

    float selected_overlay_animation_{ 0.f };
    float animation_selected_{ 0.f };

    time_point last_blink_;
    float animation_blinking_{ 0.f };
    bool blink_toggle_{ false };

    float text_width_;
    float prefix_width_;
    float buffer_width_;
    float typing_pos_width_;
    bool text_marked_dirty_{ true };
    bool post_text_calculated_{ false };
    float post_text_width_;

    float text_render_x_{ 0.f };
    float text_render_x_animation_{ 0.f };
    float text_scroll_x_{ 0.f };

    std::function<void(const std::u32string&)> callback_;

public:
    textfield(void_* instance, class input_owner* input_owner, textfield_type type, textfield_flags flags, 
              const xstr& default_text, const xstr& post_text, std::size_t max_length);

    textfield(const textfield&) = delete;
    textfield& operator=(const textfield&) = delete;

public:
    float update(const r2::rectf& pos, const render_input& input, bool occluded);
    void render(float alpha, const r2::color& text_color);
    input_response input(const input_base& input);

    void on_activate();
    void on_scale_changed();

    [[nodiscard]] bool is_valid_char(char32_t u) const noexcept;

    void start_typing(bool reset_scroll = true);

    [[nodiscard]] float animation_selected() const noexcept {
        return animation_selected_;
    }
    [[nodiscard]] const auto& get_string() const noexcept {
        return buffer_;
    }
    void set_string(const std::u32string& s);

    void set_stop_callback(std::function<void(const std::u32string&)>&& fn) {
        callback_ = std::move(fn);
    }

private:
    void reset_blinking() noexcept;
    void clear_selected();
    void invoke_stop_callback();
};

void_end_