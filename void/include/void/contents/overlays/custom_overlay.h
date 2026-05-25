#pragma once
#include <void/util/vobj.h>
#include <void/util/scalable_float.h>
#include <void/contents/input/input_receiver.h>
#include <r2/renderer_definitions.h>
#include <functional>


void_begin_

struct custom_overlay_cfg {
    bool resizable = true;
    bool movable = true;
    bool clamp_in_window = true;
    bool liquid_glass = false;
    float liquid_glass_curve = 0.f;
    sfloat liquid_glass_size = sfloat(10.f);
    sfloat min_width = sfloat(100.f, true);
    sfloat min_height = sfloat(50.f, true);
    sfloat max_width = sfloat(200.f, true);
    sfloat max_height = sfloat(100.f, true);
};

struct custom_overlay_data {
    r2::color background;
    r2::color border;
    r2::color liquid_glass_color;
    float rounding_top = 0.f;
    float rounding_bottom = 0.f;
    // x,y range 0-1, w,h unscaled size (will be multiplied by scale later)
    r2::rectf pos;
};

enum class e_resizing_type : std::uint8_t;
class custom_overlay : protected vobj,
                       public input_receiver {
private:
    r2::rectf last_pos_;

    custom_overlay_cfg cfg_;
    custom_overlay_data data_;

    float animation_{ 0.f };
    bool has_input_{ false };

    e_resizing_type resizing_type_;
    r2::rectf move_pos_;

    using callback_type = std::function<void(void_*, custom_overlay&)>;
    using input_callback_type = std::function<class input_response(void_*, custom_overlay&, const class input_base&)>;

    callback_type render_callback_;
    callback_type update_callback_;
    input_callback_type input_callback_;

    friend class _background_overlay;

public:
    custom_overlay(void_* instance, class input_owner* input_owner,
                   const custom_overlay_cfg& cfg, const custom_overlay_data& data);

public:
    void update();
    void render();
    class input_response input(const class input_base& input);

public:
    [[nodiscard]] r2::vec2 get_pos() const noexcept;
    [[nodiscard]] r2::vec2 get_size() const noexcept;

    // set the position of the overlay, range [0-1]
    void set_pos(const r2::vec2& v) noexcept {
        data_.pos.x = v.x;
        data_.pos.y = v.y;
    }

    // set the unscaled size of the overlay
    void set_size(const r2::vec2& v) noexcept {
        data_.pos.w = v.x;
        data_.pos.h = v.y;
    }

    // set the scaled position of the overlay, range [0-display_size]
    void set_pos_scaled(const r2::vec2& v);

    // set the scaled size of the overlay
    void set_size_scaled(const r2::vec2& v);

    void set_render_callback(callback_type&& callback) {
        render_callback_ = std::move(callback);
    }
    void set_update_callback(callback_type&& callback) {
        update_callback_ = std::move(callback);
    }
    void set_input_callback(input_callback_type&& callback) {
        input_callback_ = std::move(callback);
    }

    // toggle whether the input can receive input
    // should be set in update callback
    void toggle_input(bool state) { 
        has_input_ = state; 
    }

    // set the animation of the overlay
    // should be set in render callback
    void set_animation(float value) { 
        animation_ = value; 
    }

    // access overlay data
    // should only be modified in update/render callback
    [[nodiscard]] auto& data() noexcept {
        return data_;
    }

    // access overlay cfg
    // should only be modified in update/render callback
    [[nodiscard]] auto& cfg() noexcept {
        return cfg_;
    }

    [[nodiscard]] const auto& get_last_pos() const noexcept {
        return last_pos_;
    }

private:
    e_resizing_type get_resizing_type(float mouse_x, float mouse_y);
    class input_response move_window(const class input_base& input);
    class input_response resize_window(const class input_base& input);
};

void_end_