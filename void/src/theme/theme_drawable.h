#pragma once
#include <void/contents/widgets/textfield.h>
#include <r2/renderer_definitions.h>
#include <void/accessors/icons.h>



void_begin_

enum class drawable_selected_ids : std::int32_t;
class theme_drawable : protected vobj,
                       public input_receiver {
private:
    class _theme* const theme_instance_;
    std::wstring name_;
    std::wstring name_lowercase_;

    r2::color accent_color_;
    r2::color accent2_color_;
    r2::color background_color_;

    bool is_deleted_{ false };
    bool seen_in_refresh_{ true }; // should be true
    bool skipped_{ false };

    r2::rectf last_pos_;
    float animation_hovered_{ 0.f };
    float animation_selected_{ 0.f };

    struct theme_button {
        r2::rectf pos;
        float animation_hovered{ 0.f };
        float animation_selected{ 0.f };
        icons::icon_handle icon_handle{ icons::kInvalidHandle };
    };
    theme_button button_save_{};
    theme_button button_load_{};
    theme_button button_delete_{};

    std::unique_ptr<textfield> text_field_;

    bool was_occluded_{ false };

public:
    theme_drawable(void_* instance, input_owner* input_owner, class _theme* theme_instance,
                    const std::wstring& name);

public:
    void update(float x, float y, const render_input& input, bool selected, bool occluded);
    void render(float alpha);
    input_response input(const input_base& input, std::int32_t& selected_theme, std::int32_t theme_id);

    void on_scale_changed();
    void on_activate();

public:
    [[nodiscard]] const auto& get_name() const noexcept {
        return name_;
    }
    [[nodiscard]] bool is_skipped() const noexcept {
        return skipped_;
    }
    [[nodiscard]] bool& seen_in_refresh() { 
        return seen_in_refresh_; 
    }
    void queue_delete() noexcept {
        is_deleted_ = true;
    }
    [[nodiscard]] bool can_be_deleted() const noexcept { 
        return is_deleted_; 
    }
    void set_skipped(bool s) noexcept {
        skipped_ = s;
    }
    void set_colors(const r2::color& accent, const r2::color& accent2, const r2::color& background) {
        accent_color_ = accent;
        accent2_color_ = accent2;
        background_color_ = background;
    }
    void search(const std::wstring& text);

private:
    void on_stop_typing(const std::u32string& text);
    void update_button(int icon_id, const r2::vec2& pos, float button_size, theme_button& button,
                       const render_input& input, drawable_selected_ids id);
    void render_button(const theme_button& button, float alpha);

    void generate_name_lowercase();
    void update_textfield();
};

void_end_