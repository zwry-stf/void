#include "theme.h"
#include <void/void.h>
#include "theme_drawable.h"
#include "theme.h"


void_begin_

_theme::_theme(void_* instance)
	: config_manager(instance,
		std::wstring(kThemeExtension.begin(), kThemeExtension.end()),
		"theme"
	  )
{
}

_theme::~_theme() = default;

void _theme::init()
{
	const std::filesystem::path main_path = instance()->config().get_main_path();

	do_init(main_path / "themes");

	const std::wstring* v;
	if (load_last_file(v))
		load(*v);
}

void _theme::destroy()
{
	themes_.clear();
}

bool _theme::create_new()
{
    std::wstring path;
    if (!do_create_new(path))
        return false;

    // add
    auto drawable = std::make_unique<theme_drawable>(
        instance(), 
        instance(), /* input owner */
        this,
        path
    );

    if (!save(path, drawable.get(), true)) {
        show_error(xstr("Failed to create new theme "), &path);
        return false;
    }

    themes_.push_back(std::move(drawable));

    // notification
    auto& style = instance()->style();
    instance()->notifications().create_note()
        << style.text_accent() << type_name_pascal_ << xstr(" created ")
        << style.accent() << path
        << style.text() << xstr(".");

    return true;
}

bool _theme::load(const std::wstring& name, bool first)
{
    std::vector<std::uint8_t> buffer;
    if (!do_load(name, buffer))
        return false;

    if (buffer.size() != sizeof(new_theme_))
        return false;

    std::memcpy(&new_theme_, buffer.data(), sizeof(new_theme_));

    // parse data
    if (first ||
        !instance()->options().get<options::option_AnimateThemes>()) {
        new_theme_.load(instance());
    }
    else {
        old_theme_.save(instance(), false); // store current state in old theme

        animation_loading_desired_ = animation_loading_ + 1.f;
    }

    save_last_file(name);

    if (!first) {
        auto& style = instance()->style();
        instance()->notifications().create_note()
            << style.text_accent() << type_name_pascal_ << xstr(" loaded ")
            << style.grey() << name
            << style.text() << xstr(".");
    }

    instance()->callbacks().invoke<callbacks::callback_OnLoadTheme>();

    return true;
}

bool _theme::save(const std::wstring& name, theme_drawable* drawable, bool create)
{
    theme_data data;
    data.save(instance(), create);

    // save file
    if (!do_save(
        name,
        reinterpret_cast<std::uint8_t*>(&data),
        sizeof(data)
    )) {
        return false;
    }

    drawable->set_colors(
        data.accent,
        data.accent2, 
        data.background
    );

    save_last_file(name);

    // notification
    if (!create) {
        auto& style = instance()->style();
        instance()->notifications().create_note()
            << style.text_accent() << type_name_pascal_ << xstr(" saved ")
            << style.grey() << name
            << style.text() << xstr(".");
    }

    instance()->callbacks().invoke<callbacks::callback_OnSaveTheme>();

    return true;
}

bool _theme::remove(const std::wstring& name, theme_drawable* drawable)
{
    if (!do_remove(name))
        return false;

    drawable->queue_delete();

    return true;
}

bool _theme::rename(const std::wstring& name, const std::wstring& new_name)
{
    return do_rename(name, new_name);
}

bool _theme::refresh_themes(std::int32_t& selected_theme)
{
    std::wstring cfg_name;
    if (selected_theme >= 0 &&
        selected_theme < static_cast<std::int32_t>(themes_.size()))
        cfg_name = themes_[selected_theme]->get_name();

    std::error_code ec;
    auto iterator = std::filesystem::directory_iterator(get_folder_path(), ec);
    if (ec) {
        show_error(xstr("Failed to list themes"), nullptr, ec);
        return false;
    }

    for (auto& cfg : themes_)
        cfg->seen_in_refresh() = false;

    std::int32_t selected_index = 0;
    for (auto file : iterator) {
        if (!file.is_regular_file())
            continue;

        if (file.path().extension().string() != kThemeExtension)
            continue;

        auto str = file.path().filename().wstring();
        for (std::size_t i = 0u; i < kThemeExtension.length(); i++)
            str.pop_back();

        // update selected_index
        if (!cfg_name.empty() &&
            cfg_name == str)
            selected_index = static_cast<std::int32_t>(themes_.size()) - 1;

        // check if exists
        bool found = false;
        for (auto& cfg : themes_)
            if (cfg->get_name() == str)
            {
                cfg->seen_in_refresh() = true;
                found = true;
                break;
            }

        // add
        if (!found) {
            auto drawable = std::make_unique<theme_drawable>(
                instance(),
                instance(), /* input owner */
                this,
                str
            );
            if (init_drawable(str, drawable.get())) {
                themes_.push_back(std::move(drawable));
            }
        }
    }

    // check not found cfgs
    for (auto& cfg : themes_) {
        if (!cfg->seen_in_refresh())
            cfg->queue_delete();
    }

    update(selected_index);

    return true;
}

void _theme::update(std::int32_t& selected_index)
{
    std::int32_t i = 0;
    for (auto it = themes_.begin(); it != themes_.end();) {
        if (it->get()->can_be_deleted()) {
            if (i == selected_index)
                selected_index = -1;
            else if (i < selected_index)
                selected_index--;
            it = themes_.erase(it);
            i--;
        }
        else
            it++;
        i++;
    }
}

void _theme::animate()
{
    if (!instance()->options().get<options::option_AnimateThemes>())
        return;

    const float remaining = std::min(animation_loading_desired_ - animation_loading_, 1.f);
    if (remaining <= 0.f)
        return;

    // jump to final value if difference is very small
    if (remaining < util::g_min_alpha) {
        new_theme_.load(instance());
        animation_loading_ = animation_loading_desired_;
        return;
    }

    animation_loading_ = animation_loading_desired_ - 1.f + instance()->util().lerp2(
        1.f - remaining,
        1.f,
        0.6f
    );
    const float animation = std::clamp(
        animation_loading_ - animation_loading_desired_ + 1.f,
        0.f, 
        1.f
    );

    theme_data curr;
    curr.interp(instance(), old_theme_, new_theme_, animation);

    curr.load(instance());
}

bool _theme::init_drawable(const std::wstring& path, theme_drawable* drawable)
{
    std::vector<std::uint8_t> buffer;
    if (!do_load(path, buffer))
        return false;

    if (buffer.size() != sizeof(theme_data))
        return false;

    theme_data* data = reinterpret_cast<theme_data*>(buffer.data());

    drawable->set_colors(
        data->accent,
        data->accent2,
        data->background
    );

    return true;
}

/// theme data

void theme_data::load(void_* instance)
{
    auto& style = instance->style();

    style.accent = this->accent;
    style.accent2 = this->accent2;
    style.background = this->background;
    style.overlay_background = this->overlay_background;
    style.group_background = this->group_background;
    style.icon = this->icon;
    style.text = this->text;
    style.text_accent = this->text_accent;
    style.border = this->border;
    style.highlight = this->highlight;
    style.grey = this->grey;
    style.rounding->raw() = this->rounding;
    style.spacing->raw() = this->spacing;
    style.animation_speed = this->animation_speed;
    style.overlay_width->raw() = this->overlay_size;
    style.notification_time = this->notification_time;

    auto& bg = instance->background();
    bg.blur_enabled = this->blur;
    bg.blur_radius->raw() = this->blur_radius;
    bg.noise_scale = this->noise;
    bg.blend_amount = this->blend_amount;

    bg.overlay_blur_enabled = this->overlay_blur;
    bg.overlay_blur_radius->raw() = this->overlay_blur_radius;
    bg.overlay_noise_scale = this->overlay_noise;
}

void theme_data::save(void_* instance, bool create)
{
    auto& style = instance->style();

    auto& bg = instance->background();

    if (create) {
        this->accent = style.accent.get_default();
        this->accent2 = style.accent2.get_default();
        this->background = style.background.get_default();
        this->group_background = style.group_background.get_default();
        this->overlay_background = style.overlay_background.get_default();
        this->icon = style.icon.get_default();
        this->text = style.text.get_default();
        this->text_accent = style.text_accent.get_default();
        this->border = style.border.get_default();
        this->highlight = style.highlight.get_default();
        this->grey = style.grey.get_default();
        this->blur = bg.blur_enabled.get_default();
        this->blur_radius = bg.blur_radius.get_default().raw();
        this->noise = bg.noise_scale.get_default();
        this->blend_amount = bg.blend_amount.get_default();
        this->rounding = style.rounding.get_default().raw();
        this->spacing = style.spacing.get_default().raw();
        this->animation_speed = style.animation_speed.get_default();
        this->overlay_size = style.overlay_width.get_default().raw();
        this->notification_time = style.notification_time.get_default();
        this->overlay_blur = bg.overlay_blur_enabled.get_default();
        this->overlay_blur_radius = bg.overlay_blur_radius.get_default().raw();
        this->overlay_noise = bg.overlay_noise_scale.get_default();
    }
    else {
        this->accent = style.accent();
        this->accent2 = style.accent2();
        this->background = style.background();
        this->group_background = style.group_background();
        this->overlay_background = style.overlay_background();
        this->icon = style.icon();
        this->text = style.text();
        this->text_accent = style.text_accent();
        this->border = style.border();
        this->highlight = style.highlight();
        this->grey = style.grey();
        this->blur = bg.blur_enabled();
        this->blur_radius = bg.blur_radius->raw();
        this->noise = bg.noise_scale();
        this->blend_amount = bg.blend_amount();
        this->rounding = style.rounding->raw();
        this->spacing = style.spacing->raw();
        this->animation_speed = style.animation_speed();
        this->overlay_size = style.overlay_width->raw();
        this->notification_time = style.notification_time();
        this->overlay_blur = bg.overlay_blur_enabled();
        this->overlay_blur_radius = bg.overlay_blur_radius->raw();
        this->overlay_noise = bg.overlay_noise_scale();
    }
}

void theme_data::interp(void_* instance, const theme_data& a, const theme_data& b, float t)
{
    auto& util = instance->util();

    this->accent = a.accent.interp(b.accent, t);
    this->accent2 = a.accent2.interp(b.accent2, t);
    this->background = a.background.interp(b.background, t);
    this->group_background = a.group_background.interp(b.group_background, t);
    this->overlay_background = a.overlay_background.interp(b.overlay_background, t);
    this->icon = a.icon.interp(b.icon, t);
    this->text = a.text.interp(b.text, t);
    this->text_accent = a.text_accent.interp(b.text_accent, t);
    this->border = a.border.interp(b.border, t);
    this->highlight = a.highlight.interp(b.highlight, t);
    this->grey = a.grey.interp(b.grey, t);

    this->blur = t < 0.05f ? a.blur : b.blur;
    this->blur_radius = util.lerp_ex(a.blur_radius, b.blur_radius, t);
    this->noise = util.lerp_ex(a.noise, b.noise, t);
    this->blend_amount = util.lerp_ex(a.blend_amount, b.blend_amount, t);
    this->rounding = util.lerp_ex(a.rounding, b.rounding, t);
    this->spacing = util.lerp_ex(a.spacing, b.spacing, t);
    this->animation_speed = util.lerp_ex(a.animation_speed, b.animation_speed, t);

    this->overlay_blur = util.lerp_ex(a.overlay_blur, b.overlay_blur, t);
    this->overlay_blur_radius = util.lerp_ex(a.overlay_blur_radius, b.overlay_blur_radius, t);
    this->overlay_noise = util.lerp_ex(a.overlay_noise, b.overlay_noise, t);
}

void_end_