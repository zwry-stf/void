#pragma once
#include <void/contents/overlays/overlay.h>
#include <void/contents/widgets/textfield.h>
#include <util/color.h>
#include <void/util/scalable_float.h>
#include <void/accessors/icons.h>


void_begin_

class colorpicker_overlay : public overlay,
                            public input_receiver {
private:
    r2::color* const color_;
    const bool has_alpha_;
    const sfloat text_field_size_;

    r2::color last_color_;
    r2::color hue_;
    color::hsv hsv_;

    float spacing_;
    float bar_width_;
    float brightness_square_size_;

    r2::rectf square_pos_;
    r2::rectf hue_pos_;
    r2::rectf alpha_pos_;

    r2::vec4 field_pos_;

    r2::vec4 copy_pos_;
    float copy_animation_hovered_{ 0.f };
    float copy_animation_selected_{ 0.f };
    icons::icon_handle copy_icon_{ icons::kInvalidHandle };

    std::unique_ptr<textfield> text_field_;

public:
    colorpicker_overlay(void_* instance, input_owner_overlay* input_owner,
                        r2::color* color, bool has_alpha);

public:
    virtual void update(const overlay_render_input& input) override;
    virtual void render() override;
    virtual input_response input(const overlay_input& input) override;
    virtual void on_scale_changed() override;

    [[nodiscard]] const auto& get_color_string() const noexcept {
        return text_field_->get_string();
    }
    [[nodiscard]] auto* get_color() const noexcept {
        return color_; 
    }
    [[nodiscard]] bool has_alpha() const noexcept {
        return has_alpha_; 
    }

    static void color_to_string(const r2::color& c, std::u32string& out, bool has_alpha);
    static bool string_to_color(const std::u32string& s, r2::color& out, bool has_alpha, bool allow_no_alpha = true);

    void on_stop_typing(const std::u32string& s);

private:
    void update_brightness_square();
    void render_brightness_square();
    void update_hue_bar();
    void render_hue_bar();
    void update_alpha_bar();
    void render_alpha_bar();
    void update_text_field();
    void render_text_field();

    void load_color();
    void load_hue();
    void load_hsv();
};

void_end_