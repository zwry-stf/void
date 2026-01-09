#pragma once
#include "void.inline.inl"


void_begin_

enum class e_resizing_type : std::uint8_t;
class void_ : public input_owner,
              public input_owner_overlay,
              public input_receiver {
private:
    r2::renderer2d renderer_;

    std::atomic<bool> initialized_{ false };
    std::atomic<bool> open_;
    r2::rectf pos_;
    float animation_;
    float alpha_;
    time_point frame_start_;
    float delta_time_;
    float scale_;

    // search
    std::unique_ptr<class textfield> search_text_field_;
    bool has_search_{ false };
    bool search_opened_{ false };
    float search_animation_{ 0.f };
    float search_opened_animation_{ 0.f };
    float search_hovered_animation_{ 0.f };
    r2::vec4 search_pos_;
    icons::icon_handle search_icon_{ icons::kInvalidHandle };

    // move
    e_resizing_type resizing_type_;
    r2::rectf move_pos_;

public:
    void_();
    ~void_();

public:
    void init(const r2::platform_init_data& pinit, const r2::backend_init_data& binit);
    void destroy();

    void pre_resize();
    void post_resize();

    void init_render(const r2::platform_init_data& pinit, const r2::backend_init_data& binit);
    void destroy_render();

    void render();
    void update_frame_time();
    void reset_hovered_state();
    input_response on_input(const message_event& event);

    void toggle_menu(bool open);

    void set_scale(float scale);
    void set_scale_auto();

public:
    [[nodiscard]] bool is_initialized() const noexcept {
        return initialized_.load(std::memory_order_acquire);
    }
    [[nodiscard]] float scale() const noexcept {
        return scale_;
    }
    [[nodiscard]] bool is_open() const noexcept {
        return open_.load(std::memory_order_acquire);
    }
    [[nodiscard]] const auto& pos() const noexcept {
        return pos_;
    }
    [[nodiscard]] float animation() const noexcept {
        return animation_;
    }
    [[nodiscard]] float alpha() const noexcept {
        return alpha_;
    }
    [[nodiscard]] float delta_time() const noexcept {
        return delta_time_;
    }
    [[nodiscard]] auto frame_start() const noexcept {
        return frame_start_;
    }

    [[nodiscard]] class menu_builder get_builder() noexcept;

    // search

    [[nodiscard]] const auto& search_pos() const noexcept {
        return search_pos_;
    }
    [[nodiscard]] bool has_search() const noexcept {
        return has_search_;
    }
    [[nodiscard]] bool is_search_opened() const noexcept {
        return search_opened_;
    }
    void toggle_search(bool enable) noexcept {
        has_search_ = enable;
    }
    [[nodiscard]] const auto& get_search_text() const noexcept {
        return search_text_field_->get_string();
    }
    void set_search_text(const std::u32string& string) {
        search_text_field_->set_string(string);
    }

public:
    // accessors
    _define_void_access(renderer_, renderer);
    _define_void_accessor(resources, resources);
    _define_void_accessor(cursors, cursors);
    _define_void_accessor(style, style);
    _define_void_accessor(fonts, fonts);
    _define_void_accessor(icons, icons);
    _define_void_accessor(render_target, render_target);
    _define_void_accessor(options, options);
    _define_void_accessor(background, background);
    _define_void_accessor(util, util);
    _define_void_accessor(input, input);
    _define_void_accessor(account, account);
    _define_void_accessor(notifications, notifications);
    _define_void_accessor(watermark, watermark);
    _define_void_accessor(config, config);
    _define_void_accessor(theme, theme);

private:
    [[nodiscard]] float calculate_scale() noexcept;
    void render_menu();
    void render_overlays();
    void render_search();
    e_resizing_type get_resizing_type(float mouse_x, float mouse_y) const noexcept;
    input_response input_move(const input_base& input);
    input_response input_search(const input_base& input);
    void update_scale(float scale);

private:
    std::unique_ptr<class _background> background_;
    std::unique_ptr<class _background_overlay> background_overlay_;
    std::unique_ptr<class sidebar> sidebar_;
    std::unique_ptr<class _config> config_;
    std::unique_ptr<class _theme> theme_;

    friend class background;
    friend class config;
    friend class theme;
    friend class menu_builder;
};

void_end_