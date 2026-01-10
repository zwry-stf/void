#pragma once
#include <config/config_manager.h>
#include <r2/renderer_definitions.h>


void_begin_

class theme_data {
public:
    r2::color accent;
    r2::color accent2;
    r2::color background;
    r2::color group_background;
    r2::color overlay_background;
    r2::color icon;
    r2::color text;
    r2::color text_accent;
    r2::color border;
    r2::color highlight;
    r2::color grey;
    bool blur;
    float blur_radius;
    float noise;
    float rounding;
    float spacing;
    float animation_speed;
    bool overlay_blur;
    float overlay_blur_radius;
    float overlay_noise;
    float blend_amount;
    float overlay_size;
    float notification_time;

public:
    void load(void_* instance);
    void save(void_* instance, bool create);
    void interp(void_* instance, const theme_data& a, const theme_data& b, float t);
};

class theme_drawable;
class _theme : public config_manager {
private:
    std::vector<std::unique_ptr<theme_drawable>> themes_;

    float animation_loading_desired_{ 0.f };
    float animation_loading_{ 0.f };

    theme_data old_theme_;
    theme_data new_theme_;

public:
    _theme(void_* instance);
    ~_theme();

    inline static constexpr std::string_view kThemeExtension = ".theme";

public:
    void init();
    void destroy();

public:
    bool create_new();
    bool load(const std::wstring& name, bool first = false);
    bool save(const std::wstring& name, theme_drawable* drawable, bool create = false);
    bool remove(const std::wstring& name, theme_drawable* drawable);
    bool rename(const std::wstring& name, const std::wstring& new_name);

    bool refresh_themes(std::int32_t& selected_theme);

    void update(std::int32_t& selected_theme);

    void animate();

private:
    bool init_drawable(const std::wstring& path, theme_drawable* drawable);

    friend class theme_tab;
};

void_end_