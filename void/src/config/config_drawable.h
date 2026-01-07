#pragma once
#include <void/contents/widgets/textfield.h>
#include <r2/renderer_definitions.h>
#include <void/accessors/icons.h>



void_begin_

enum class drawable_selected_ids : std::int32_t;
class config_drawable : protected vobj,
                        public input_receiver {
private:
    class _config* const config_instance_;
    std::wstring name_;
    std::wstring name_lowercase_;
    std::string modified_time_;

    bool is_deleted_{ false };
    bool seen_in_refresh_{ true }; // should be true
    bool skipped_{ false };

    r2::rectf last_pos_;
    float animation_hovered_{ 0.f };
    float animation_selected_{ 0.f };

    bool text_width_calculated_{ false };
    float text_width_;

    struct config_button {
        r2::rectf pos;
        float animation_hovered{ 0.f };
        float animation_selected{ 0.f };
        icons::icon_handle icon_handle{ icons::kInvalidHandle };
    };
    config_button button_save_{};
    config_button button_load_{};
    config_button button_delete_{};

    std::unique_ptr<textfield> text_field_;

    bool was_occluded_{ false };

public:
    config_drawable(void_* instance, input_owner* input_owner, class _config* config_instance,
                    const std::wstring& name, std::string&& modified_time);

public:
    void update(float x, float y, float w, const render_input& input, bool selected, bool occluded);
    void render(float alpha);
    input_response input(const input_base& input, std::int32_t& selected_config, std::int32_t config_id);

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
    void set_modified_time(std::string&& s) {
        modified_time_ = std::move(s); 
    }
    void search(const std::wstring& text);

private:
    void on_stop_typing(const std::u32string& text);
    void update_button(int icon_id, const r2::vec2& pos, float button_size, config_button& button,
                       const render_input& input, drawable_selected_ids id);
    void render_button(const config_button& button, float alpha);

    void generate_name_lowercase();
    void update_textfield();
};

void_end_