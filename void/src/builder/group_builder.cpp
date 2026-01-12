#include <void/builder/group.h>
#include <void/builder/builder.h>
#include <void/void.h>
#include <contents/container/group.h>
#include <config/config.h>
#include <contents/overlays/multiselect/config_module.h>
#include <contents/input/keybind.h>

// widgets
#include <contents/widgets/toggle/toggle.h>
#include <contents/widgets/slider/slider.h>
#include <contents/widgets/colorpicker/colorpicker.h>
#include <contents/widgets/dropdown/dropdown.h>
#include <contents/widgets/multiselect/multiselect.h>
#include <contents/widgets/list/list.h>
#include <contents/widgets/button/button.h>
#include <contents/widgets/spacing/spacing.h>
#include <contents/widgets/textfield/textfield.h>

// child widgets
#include <contents/widgets/colorpicker/colorpicker_child.h>
#include <contents/widgets/colorpicker/optional_colorpicker_child.h>
#include <contents/widgets/dropdown/dropdown_child.h>
#include <contents/widgets/multiselect/multiselect_child.h>
#include <contents/widgets/childwindow/childwindow_child.h>
#include <contents/widgets/keybind/keybind_child.h>

// overlays
#include <contents/overlays/colorpicker/colorpicker.h>
#include <contents/overlays/dropdown/dropdown.h>
#include <contents/overlays/multiselect/multiselect.h>
#include <contents/overlays/childwindow/childwindow.h>


void_begin_

/// group_base_options

group_base_options::group_base_options(void_* instance, menu_builder* builder, 
                                       widget* widget_instance, std::size_t config_id)
    : base_builder_object(instance, builder),
      widget_instance_(widget_instance),
      config_id_(config_id)
{
}

void group_base_options::disabled(bool& state)
{
    get_widget()->set_disabled_callback(
        [&state]() -> bool {
            return state;
        }
    );
}

void group_base_options::disabled_inverted(bool& state)
{
    get_widget()->set_disabled_callback(
        [&state]() -> bool {
            return !state;
        }
    );
}

void group_base_options::disabled(std::function<bool()>&& callback)
{
    get_widget()->set_disabled_callback(std::move(callback));
}

void group_base_options::condition(std::function<bool()>&& callback)
{
    get_widget()->set_visible_callback(std::move(callback));
}

void group_base_options::no_config()
{
    if (config_id_ == _config::kInvalidModuleId)
        return;
    instance()->config().remove_module(config_id_);
}

/// group_item_options

group_item_options::group_item_options(void_* instance, menu_builder* builder, 
                                       owner_type* group_instance, widget* widget_instance,
                                       std::size_t config_id)
    : group_base_options(instance, builder, widget_instance, config_id),
      group_instance_(group_instance)
{
}

group_item_options::owner_type& group_item_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

group_item_options::owner_type& group_item_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

group_item_options::owner_type& group_item_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

group_item_options::owner_type& group_item_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

group_item_options::owner_type& group_item_options::no_config()
{
    group_base_options::no_config();
    return *group_instance_;
}


/// group_with_child_base_options

xstr group_with_child_base_options::build_child_config_path(const xstr& type)
{
    xstr ret = type;
    ret.append_safe(config_path_);
    return ret;
}

group_with_child_base_options::group_with_child_base_options(void_* instance, menu_builder* builder, 
                                                             widget* widget_instance, const xstr& config_path,
                                                             std::size_t config_id)
    : group_base_options(instance, builder, widget_instance, config_id),
      config_path_(config_path)
{
}

void group_with_child_base_options::colorpicker(r2::color& value, bool has_alpha)
{
    const auto overlay_id = instance()->create_overlay(
        std::make_unique<colorpicker_overlay>(
            instance(), instance(), instance(),
            &value, has_alpha
        )
    );

    get_widget()->add_child(
        std::make_unique<colorpicker_child>(
            instance(), instance(), instance(),
            overlay_id
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<r2::color>>(
            build_child_config_path("colorpicker"),
            &value
        )
    );
}

void group_with_child_base_options::optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha)
{
    const auto overlay_id = instance()->create_overlay(
        std::make_unique<colorpicker_overlay>(
            instance(), instance(), instance(),
            &value, has_alpha
        )
    );

    get_widget()->add_child(
        std::make_unique<optional_colorpicker_child>(
            instance(), instance(), instance(),
            overlay_id,
            &enabled
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<r2::color>>(
            build_child_config_path("color_ocolor"),
            &value
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<bool>>(
            build_child_config_path("enabled_ocolor"),
            &enabled
        )
    );
}

void group_with_child_base_options::dropdown(list_options* options, std::size_t& selected)
{
    const auto overlay_id = instance()->create_overlay(
        std::make_unique<dropdown_overlay>(
            instance(), instance(), instance(),
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    get_widget()->add_child(
        std::make_unique<dropdown_child>(
            instance(), instance(), instance(),
            overlay_id
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<std::size_t>>(
            build_child_config_path("dropdown"),
            &selected
        )
    );
}

void group_with_child_base_options::multiselect(list_options* options, std::vector<bool>& selected)
{
    const auto overlay_id = instance()->create_overlay(
        std::make_unique<multiselect_overlay>(
            instance(), instance(), instance(),
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    get_widget()->add_child(
        std::make_unique<multiselect_child>(
            instance(), instance(), instance(),
            overlay_id
        )
    );

    instance()->config().add_module(
        std::make_unique<multiselect_config_module>(
            build_child_config_path("multiselect"),
            &selected
        )
    );
}

void group_with_child_base_options::last_childwindow(std::int32_t overlay_id)
{
    assert(instance()->get_overlay(overlay_id)->is_childwindow());

    get_widget()->add_child(
        std::make_unique<childwindow_child>(
            instance(), instance(), instance(),
            overlay_id
        )
    );
}

void group_with_child_base_options::last_keybind(keybind_owner* bind)
{
    get_widget()->add_child(
        keybind_child::create_keybind(
            instance(), instance(), instance(),
            bind
        )
    );
}

void group_with_child_base_options::custom_child(std::unique_ptr<widget_child>&& child)
{
    get_widget()->add_child(std::move(child));
}


/// group_with_child_options

group_with_child_options::group_with_child_options(void_* instance, menu_builder* builder,
                                                   owner_type* group_instance, widget* widget_instance, 
                                                   const xstr& config_path, std::size_t config_id)
    : group_with_child_base_options(instance, builder, widget_instance, config_path, config_id),
      group_instance_(group_instance)
{
}

group_with_child_options::owner_type& group_with_child_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::no_config()
{
    group_base_options::no_config();
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::colorpicker(r2::color& value, bool has_alpha)
{
    group_with_child_base_options::colorpicker(
        value, 
        has_alpha
    );
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha)
{
    group_with_child_base_options::optional_colorpicker(value, enabled, has_alpha);
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::dropdown(list_options* options, std::size_t& selected)
{
    group_with_child_base_options::dropdown(
        options, 
        selected
    );
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::multiselect(list_options* options, std::vector<bool>& selected)
{
    group_with_child_base_options::multiselect(
        options, 
        selected
    );
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::last_childwindow()
{
    const auto overlay_id = group_instance_->last_childwindow_;
    assert(overlay_id != -1);

    group_with_child_base_options::last_childwindow(overlay_id);
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::last_keybind()
{
    auto* const keybind = group_instance_->last_keybind_;
    assert(keybind != nullptr);

    group_with_child_base_options::last_keybind(keybind);
    return *group_instance_;
}

group_with_child_options::owner_type& group_with_child_options::custom_child(std::unique_ptr<class widget_child>&& child)
{
    group_with_child_base_options::custom_child(std::move(child));
    return *group_instance_;
}


/// group_textfield_options

group_textfield_options::group_textfield_options(void_* instance, menu_builder* builder, 
                                                 owner_type* group_instance, widget* widget_instance,
                                                 const xstr& config_path, std::size_t config_id)
    : group_with_child_base_options(instance, builder, widget_instance, config_path, config_id),
      group_instance_(group_instance)
{
}

group_textfield_options::~group_textfield_options()
{
    // update textfield
    textfield_flags flags = textfield_flags::mouse_in_rect | 
        textfield_flags::movable_caret | textfield_flags::stop_on_return;
    if (faded_text_)
        flags = flags | textfield_flags::faded_text;

    get_widget<textfield_widget>()->change_textfield(
        std::make_unique<textfield>(
            instance(), instance(),
            type_,
            flags,
            default_text_,
            xstr(), /* post text */
            max_length_
        )
    );
}

group_textfield_options::owner_type& group_textfield_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::colorpicker(r2::color& value, bool has_alpha)
{
    group_with_child_base_options::colorpicker(value, has_alpha);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha)
{
    group_with_child_base_options::optional_colorpicker(value, enabled, has_alpha);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::dropdown(list_options* options, std::size_t& selected)
{
    group_with_child_base_options::dropdown(options, selected);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::multiselect(list_options* options, std::vector<bool>& selected)
{
    group_with_child_base_options::multiselect(options, selected);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::last_childwindow()
{
    const auto overlay_id = group_instance_->last_childwindow_;
    assert(overlay_id != -1);

    group_with_child_base_options::last_childwindow(overlay_id);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::last_keybind()
{
    auto* const keybind = group_instance_->last_keybind_;
    assert(keybind != nullptr);

    group_with_child_base_options::last_keybind(keybind);
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::custom_child(std::unique_ptr<class widget_child>&& child)
{
    group_with_child_base_options::custom_child(std::move(child));
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::default_text(const xstr& text)
{
    default_text_ = text;
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::type(textfield_type type)
{
    type_ = type;
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::faded(bool state)
{
    faded_text_ = state;
    return *group_instance_;
}

group_textfield_options::owner_type& group_textfield_options::max_length(std::size_t length)
{
    max_length_ = length;
    return *group_instance_;
}


/// group_slider_options

group_slider_options::group_slider_options(void_* instance, menu_builder* builder,
                                           owner_type* group_instance, widget* widget_instance,
                                           std::size_t config_id)
    : group_base_options(instance, builder, widget_instance, config_id),
      group_instance_(group_instance)
{
}

group_slider_options::owner_type& group_slider_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

group_slider_options::owner_type& group_slider_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

group_slider_options::owner_type& group_slider_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

group_slider_options::owner_type& group_slider_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

group_slider_options::owner_type& group_slider_options::no_config()
{
    group_base_options::no_config();
    return *group_instance_;
}

group_slider_options::owner_type& group_slider_options::decimal_count(int count)
{
    get_widget<slider>()->set_decimal_count(count);
    return *group_instance_;
}


/// group_spacing_options

group_spacing_options::group_spacing_options(void_* instance, menu_builder* builder, 
                                             owner_type* group_instance, widget* widget_instance)
    : group_base_options(instance, builder, widget_instance, _config::kInvalidModuleId),
      group_instance_(group_instance)
{
}

group_spacing_options::owner_type& group_spacing_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}


/// overlay_item_options

overlay_item_options::overlay_item_options(void_* instance, menu_builder* builder, 
                                           owner_type* group_instance, widget* widget_instance,
                                           std::size_t config_id)
    : group_base_options(instance, builder, widget_instance, config_id),
      group_instance_(group_instance)
{
}

overlay_item_options::owner_type& overlay_item_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

overlay_item_options::owner_type& overlay_item_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

overlay_item_options::owner_type& overlay_item_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

overlay_item_options::owner_type& overlay_item_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

overlay_item_options::owner_type& overlay_item_options::no_config()
{
    group_base_options::no_config();
    return *group_instance_;
}


/// overlay_with_child_base_options

xstr overlay_with_child_base_options::build_overlay_child_config_path(const xstr& type)
{
    xstr ret = type;
    ret.append_safe(config_path_);

    return ret;
}

overlay_with_child_base_options::overlay_with_child_base_options(void_* instance, menu_builder* builder,
                                                                 widget* widget_instance, childwindow* childwindow_instance,
                                                                 const xstr& config_path, std::size_t config_id)
    : group_base_options(instance, builder, widget_instance, config_id),
      childwindow_instance_(childwindow_instance),
      config_path_(config_path)
{
}

void overlay_with_child_base_options::colorpicker(r2::color& value, bool has_alpha)
{
    const auto overlay_id = childwindow_instance_->create_overlay(
        std::make_unique<colorpicker_overlay>(
            instance(), instance(),
            childwindow_instance_,
            &value, has_alpha
        )
    );

    get_widget()->add_child(
        std::make_unique<colorpicker_child>(
            instance(), instance(),
            childwindow_instance_,
            overlay_id
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<r2::color>>(
            build_overlay_child_config_path("colorpicker"),
            &value
        )
    );
}

void overlay_with_child_base_options::optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha)
{
    const auto overlay_id = childwindow_instance_->create_overlay(
        std::make_unique<colorpicker_overlay>(
            instance(), instance(),
            childwindow_instance_,
            &value, has_alpha
        )
    );

    get_widget()->add_child(
        std::make_unique<optional_colorpicker_child>(
            instance(), instance(),
            childwindow_instance_,
            overlay_id,
            &enabled
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<r2::color>>(
            build_overlay_child_config_path("color_ocolor"),
            &value
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<bool>>(
            build_overlay_child_config_path("enabled_ocolor"),
            &enabled
        )
    );
}

void overlay_with_child_base_options::dropdown(list_options* options, std::size_t& selected)
{
    const auto overlay_id = childwindow_instance_->create_overlay(
        std::make_unique<dropdown_overlay>(
            instance(), instance(),
            childwindow_instance_,
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    get_widget()->add_child(
        std::make_unique<dropdown_child>(
            instance(), instance(),
            childwindow_instance_,
            overlay_id
        )
    );

    instance()->config().add_module(
        std::make_unique<default_config_module<std::size_t>>(
            build_overlay_child_config_path("dropdown"),
            &selected
        )
    );
}

void overlay_with_child_base_options::multiselect(list_options* options, std::vector<bool>& selected)
{
    const auto overlay_id = childwindow_instance_->create_overlay(
        std::make_unique<multiselect_overlay>(
            instance(), instance(),
            childwindow_instance_,
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    get_widget()->add_child(
        std::make_unique<multiselect_child>(
            instance(), instance(),
            childwindow_instance_,
            overlay_id
        )
    );

    instance()->config().add_module(
        std::make_unique<multiselect_config_module>(
            build_overlay_child_config_path("multiselect"),
            &selected
        )
    );
}

void overlay_with_child_base_options::custom_child(std::unique_ptr<widget_child>&& child)
{
    get_widget()->add_child(std::move(child));
}


/// overlay_with_child_options

overlay_with_child_options::overlay_with_child_options(void_* instance, menu_builder* builder,
                                                       owner_type* group_instance, widget* widget_instance,
                                                       childwindow* childwindow_instance, const xstr& config_path,
                                                       std::size_t config_id)
    : overlay_with_child_base_options(instance, builder, widget_instance, 
        childwindow_instance, config_path, config_id),
      group_instance_(group_instance)
{
}

overlay_with_child_options::owner_type& overlay_with_child_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::no_config()
{
    group_base_options::no_config();
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::colorpicker(r2::color& value, bool has_alpha)
{
    overlay_with_child_base_options::colorpicker(value, has_alpha);
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha)
{
    overlay_with_child_base_options::optional_colorpicker(value, enabled, has_alpha);
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::dropdown(list_options* options, std::size_t& selected)
{
    overlay_with_child_base_options::dropdown(options, selected);
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::multiselect(list_options* options, std::vector<bool>& selected)
{
    overlay_with_child_base_options::multiselect(options, selected);
    return *group_instance_;
}

overlay_with_child_options::owner_type& overlay_with_child_options::custom_child(std::unique_ptr<class widget_child>&& child)
{
    overlay_with_child_base_options::custom_child(std::move(child));
    return *group_instance_;
}


/// overlay_textfield_options

overlay_textfield_options::overlay_textfield_options(void_* instance, menu_builder* builder,
                                                     owner_type* group_instance, widget* widget_instance,
                                                     childwindow* childwindow_instance, const xstr& config_path,
                                                     std::size_t config_id)
    : overlay_with_child_base_options(instance, builder, widget_instance,
        childwindow_instance, config_path, config_id),
      group_instance_(group_instance)
{
}

overlay_textfield_options::~overlay_textfield_options()
{
    // update textfield
    textfield_flags flags = textfield_flags::mouse_in_rect |
        textfield_flags::movable_caret | textfield_flags::stop_on_return;
    if (faded_text_)
        flags = flags | textfield_flags::faded_text;

    get_widget<textfield_widget>()->change_textfield(
        std::make_unique<textfield>(
            instance(), instance(),
            type_,
            flags,
            default_text_,
            xstr(), /* post text */
            max_length_
        )
    );
}

overlay_textfield_options::owner_type& overlay_textfield_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::colorpicker(r2::color& value, bool has_alpha)
{
    overlay_with_child_base_options::colorpicker(value, has_alpha);
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha)
{
    overlay_with_child_base_options::optional_colorpicker(value, enabled, has_alpha);
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::dropdown(list_options* options, std::size_t& selected)
{
    overlay_with_child_base_options::dropdown(options, selected);
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::multiselect(list_options* options, std::vector<bool>& selected)
{
    overlay_with_child_base_options::multiselect(options, selected);
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::custom_child(std::unique_ptr<class widget_child>&& child)
{
    overlay_with_child_base_options::custom_child(std::move(child));
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::default_text(const xstr& text)
{
    default_text_ = text;
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::type(textfield_type type)
{
    type_ = type;
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::faded(bool state)
{
    faded_text_ = state;
    return *group_instance_;
}

overlay_textfield_options::owner_type& overlay_textfield_options::max_length(std::size_t length)
{
    max_length_ = length;
    return *group_instance_;
}


/// overlay_slider_options

overlay_slider_options::overlay_slider_options(void_* instance, menu_builder* builder,
                                               owner_type* group_instance, widget* widget_instance,
                                               std::size_t config_id)
    : group_base_options(instance, builder, widget_instance, config_id),
      group_instance_(group_instance)
{
}

overlay_slider_options::owner_type& overlay_slider_options::disabled(bool& state)
{
    group_base_options::disabled(state);
    return *group_instance_;
}

overlay_slider_options::owner_type& overlay_slider_options::disabled_inverted(bool& state)
{
    group_base_options::disabled_inverted(state);
    return *group_instance_;
}

overlay_slider_options::owner_type& overlay_slider_options::disabled(std::function<bool()>&& callback)
{
    group_base_options::disabled(std::move(callback));
    return *group_instance_;
}

overlay_slider_options::owner_type& overlay_slider_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

overlay_slider_options::owner_type& overlay_slider_options::no_config()
{
    group_base_options::no_config();
    return *group_instance_;
}

overlay_slider_options::owner_type& overlay_slider_options::decimal_count(int count)
{
    get_widget<slider>()->set_decimal_count(count);
    return *group_instance_;
}


/// overlay_spacing_options

overlay_spacing_options::overlay_spacing_options(void_* instance, menu_builder* builder, 
                                                 owner_type* group_instance, widget* widget_instance)
    : group_base_options(instance, builder, widget_instance, _config::kInvalidModuleId),
      group_instance_(group_instance)
{
}

overlay_spacing_options::owner_type& overlay_spacing_options::condition(std::function<bool()>&& callback)
{
    group_base_options::condition(std::move(callback));
    return *group_instance_;
}

/// childwindow_options

xstr childwindow_options::build_overlay_config_path(const xstr& type)
{
    xstr ret = last_overlay_widget_name_;
    ret.append_safe(type);
    ret.append_safe(group_instance_->last_widget_name_);
    ret.append_safe(get_last_group_name());
    ret.append_safe(get_last_child_name());

    return ret;
}

childwindow_options::childwindow_options(void_* instance, menu_builder* builder,
                                         owner_type* group_instance, childwindow* childwindow_instance)
    : base_builder_object(instance, builder),
      group_instance_(group_instance),
      childwindow_instance_(childwindow_instance)
{
}

overlay_item_options::owner_type childwindow_options::custom_widget(std::unique_ptr<widget>&& w)
{
    auto* widget = childwindow_instance_->add_widget(std::move(w));

    return overlay_item_options::owner_type(
        *group_instance_,
        widget,
        _config::kInvalidModuleId
    );
}

overlay_with_child_options::owner_type childwindow_options::toggle(const xstr& name, bool& value)
{
    last_overlay_widget_name_ = name;

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::toggle>(
            instance(), instance(),
            name,
            &value
        )
    );

    const auto config_path = build_overlay_config_path("toggle");
    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<bool>>(
            config_path,
            &value
        )
    );

    return overlay_with_child_options::owner_type(
        *group_instance_, 
        widget,
        childwindow_instance_,
        config_path,
        config_id
    );
}

overlay_with_child_options::owner_type childwindow_options::button(const xstr& name, const xstr& button_text, std::function<void()>&& callback)
{
    last_overlay_widget_name_ = name;

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::button>(
            instance(), instance(),
            name, button_text,
            std::move(callback)
        )
    );

    return overlay_with_child_options::owner_type(
        *group_instance_,
        widget,
        childwindow_instance_,
        build_overlay_config_path("button"),
        _config::kInvalidModuleId
    );
}

overlay_with_child_options::owner_type childwindow_options::colorpicker(const xstr& name, r2::color& value, bool has_alpha)
{
    last_overlay_widget_name_ = name;

    const auto overlay_id = childwindow_instance_->create_overlay(
        std::make_unique<colorpicker_overlay>(
            instance(), instance(),
            childwindow_instance_,
            &value, has_alpha
        )
    );

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::colorpicker>(
            instance(), instance(),
            childwindow_instance_,
            overlay_id,
            name
        )
    );

    const auto config_path = build_overlay_config_path("colorpicker");
    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<r2::color>>(
            config_path,
            &value
        )
    );

    return overlay_with_child_options::owner_type(
        *group_instance_,
        widget,
        childwindow_instance_,
        config_path,
        config_id
    );
}

overlay_with_child_options::owner_type childwindow_options::dropdown(const xstr& name, list_options* options, std::size_t& selected)
{
    last_overlay_widget_name_ = name;

    const auto overlay_id = childwindow_instance_->create_overlay(
        std::make_unique<dropdown_overlay>(
            instance(), instance(),
            childwindow_instance_,
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::dropdown>(
            instance(), instance(),
            childwindow_instance_,
            overlay_id,
            name
        )
    );

    const auto config_path = build_overlay_config_path("dropdown");
    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<std::size_t>>(
            config_path,
            &selected
        )
    );

    return overlay_with_child_options::owner_type(
        *group_instance_,
        widget,
        childwindow_instance_,
        config_path,
        config_id
    );
}

overlay_item_options::owner_type childwindow_options::list(const xstr& name, list_options* options, std::size_t& selected, bool has_search, int rows)
{
    last_overlay_widget_name_ = name;

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::list>(
            instance(), instance(),
            std::unique_ptr<list_options>(options),
            name,
            &selected,
            rows,
            has_search
        )
    );

    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<std::size_t>>(
            build_overlay_config_path("list"),
            &selected
        )
    );

    return overlay_item_options::owner_type(
        *group_instance_,
        widget,
        config_id
    );
}

overlay_with_child_options::owner_type childwindow_options::multiselect(const xstr& name, list_options* options, std::vector<bool>& selected)
{
    last_overlay_widget_name_ = name;

    const auto overlay_id = childwindow_instance_->create_overlay(
        std::make_unique<multiselect_overlay>(
            instance(), instance(), 
            childwindow_instance_,
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::multiselect>(
            instance(), instance(),
            childwindow_instance_,
            overlay_id,
            name
        )
    );

    const auto config_path = build_overlay_config_path("multiselect");
    const auto config_id = instance()->config().add_module(
        std::make_unique<multiselect_config_module>(
            config_path,
            &selected
        )
    );

    return overlay_with_child_options::owner_type(
        *group_instance_,
        widget,
        childwindow_instance_,
        config_path,
        config_id
    );
}

overlay_slider_options::owner_type childwindow_options::slider(const xstr& name, float& value, float min, float max, const std::format_string<float>& format)
{
    last_overlay_widget_name_ = name;

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::slider>(
            instance(), instance(),
            name,
            &value,
            min, max,
            format
        )
    );

    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<float>>(
            build_overlay_config_path("slider"),
            &value
        )
    );

    return overlay_slider_options::owner_type(
        *group_instance_, 
        widget,
        config_id
    );
}

overlay_spacing_options::owner_type childwindow_options::spacing()
{
    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::spacing>(
            instance(), instance()
        )
    );

    return overlay_spacing_options::owner_type(*group_instance_, widget);
}

overlay_textfield_options::owner_type childwindow_options::textfield(const xstr& name, std::function<void(const std::u32string& s)>&& callback)
{
    last_overlay_widget_name_ = name;

    auto* widget = childwindow_instance_->add_widget(
        std::make_unique<::vo::textfield_widget>(
            instance(), instance(),
            name, std::move(callback),
            textfield_type::text,
            textfield_flags::mouse_in_rect | textfield_flags::stop_on_return |
            textfield_flags::faded_text | textfield_flags::movable_caret,
            xstr(), /* default text */
            40u
        )
    );

    return overlay_textfield_options::owner_type(
        *group_instance_,
        widget,
        childwindow_instance_,
        build_overlay_config_path("textfield"),
        _config::kInvalidModuleId
    );
}


/// keybind_options_base

keybind_options_base::keybind_options_base(void_* instance, menu_builder* builder,
                                           keybind_owner* keybind_instance)
    : base_builder_object(instance, builder),
      keybind_instance_(keybind_instance)
{
}

void keybind_options_base::disabled(bool& state)
{
    keybind_instance_->set_disabled_callback(
        [&state]() -> bool {
            return state;
        }
    );
}

void keybind_options_base::disabled_inverted(bool& state)
{
    keybind_instance_->set_disabled_callback(
        [&state]() -> bool {
            return !state;
        }
    );
}

void keybind_options_base::disabled(std::function<bool()>&& callback)
{
    keybind_instance_->set_disabled_callback(std::move(callback));
}

void keybind_options_base::mode(keybind_mode mode)
{
    keybind_instance_->set_mode(mode);
}

void keybind_options_base::key(::vo::key k)
{
    keybind_instance_->set_key(k);
}

void keybind_options_base::key(mouse_button k)
{
    keybind_instance_->set_key(k);
}


/// group_keybind_options

group_keybind_options::group_keybind_options(void_* instance, menu_builder* builder, 
                                             owner_type* group_instance, keybind_owner* keybind_instance)
    : keybind_options_base(instance, builder, keybind_instance),
      group_instance_(group_instance)
{
}

group_keybind_options::owner_type& group_keybind_options::disabled(bool& state)
{
    keybind_options_base::disabled(state);
    return *group_instance_;
}

group_keybind_options::owner_type& group_keybind_options::disabled_inverted(bool& state)
{
    keybind_options_base::disabled_inverted(state);
    return *group_instance_;
}

group_keybind_options::owner_type& group_keybind_options::disabled(std::function<bool()>&& callback)
{
    keybind_options_base::disabled(std::move(callback));
    return *group_instance_;
}

group_keybind_options::owner_type& group_keybind_options::mode(keybind_mode mode)
{
    keybind_options_base::mode(mode);
    return *group_instance_;
}

group_keybind_options::owner_type& group_keybind_options::key(::vo::key k)
{
    keybind_options_base::key(k);
    return *group_instance_;
}

group_keybind_options::owner_type& group_keybind_options::key(mouse_button k)
{
    keybind_options_base::key(k);
    return *group_instance_;
}


/// group_builder

xstr group_builder::build_config_path(const xstr& type)
{
    xstr ret = last_widget_name_;
    ret.append_safe(type);
    ret.append_safe(get_last_group_name());
    ret.append_safe(get_last_child_name());

    return ret;
}

group_builder::group_builder(void_* instance, menu_builder* builder, group* group_instance)
    : base_builder_object(instance, builder),
      group_instance_(group_instance)
{
}

group_item_options::owner_type group_builder::custom_widget(std::unique_ptr<widget>&& w)
{
    auto* widget = group_instance_->add_widget(std::move(w));

    return group_item_options::owner_type(
        *this, 
        widget,
        _config::kInvalidModuleId
    );
}

group_with_child_options::owner_type group_builder::toggle(const xstr& name, bool& value)
{
    last_widget_name_ = name;

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::toggle>(
            instance(), instance(),
            name, &value
        )
    );

    const auto config_path = build_config_path("toggle");
    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<bool>>(
            config_path,
            &value
        )
    );

    return group_with_child_options::owner_type(
        *this,
        widget,
        config_path,
        config_id
    );
}

group_with_child_options::owner_type group_builder::colorpicker(const xstr& name, r2::color& value, bool has_alpha)
{
    last_widget_name_ = name;

    const auto overlay_id = instance()->create_overlay(
        std::make_unique<colorpicker_overlay>(
            instance(), instance(), instance(),
            &value, has_alpha
        )
    );

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::colorpicker>(
            instance(), instance(), instance(),
            overlay_id,
            name
        )
    );

    const auto config_path = build_config_path("colorpicker");
    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<r2::color>>(
            config_path,
            &value
        )
    );

    return group_with_child_options::owner_type(
        *this,
        widget,
        config_path,
        config_id
    );
}

group_with_child_options::owner_type group_builder::dropdown(const xstr& name, list_options* options, std::size_t& selected)
{
    last_widget_name_ = name;

    const auto overlay_id = instance()->create_overlay(
        std::make_unique<dropdown_overlay>(
            instance(), instance(), instance(),
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::dropdown>(
            instance(), instance(), instance(),
            overlay_id,
            name
        )
    );

    const auto config_path = build_config_path("dropdown");
    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<std::size_t>>(
            config_path,
            &selected
        )
    );

    return group_with_child_options::owner_type(
        *this, 
        widget,
        config_path,
        config_id
    );
}

group_item_options::owner_type group_builder::list(const xstr& name, list_options* options, std::size_t& selected,
                                                     bool has_search, int rows)
{
    last_widget_name_ = name;

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::list>(
            instance(), instance(),
            std::unique_ptr<list_options>(options),
            name,
            &selected,
            rows,
            has_search
        )
    );

    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<std::size_t>>(
            build_config_path("list"),
            &selected
        )
    );

    return group_item_options::owner_type(
        *this, 
        widget,
        config_id
    );
}

group_with_child_options::owner_type group_builder::multiselect(const xstr& name, list_options* options, std::vector<bool>& selected)
{
    last_widget_name_ = name;

    const auto overlay_id = instance()->create_overlay(
        std::make_unique<multiselect_overlay>(
            instance(), instance(), instance(),
            std::unique_ptr<list_options>(options),
            &selected
        )
    );

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::multiselect>(
            instance(), instance(), instance(),
            overlay_id,
            name
        )
    );

    const auto config_path = build_config_path("multiselect");
    const auto config_id = instance()->config().add_module(
        std::make_unique<multiselect_config_module>(
            config_path,
            &selected
        )
    );

    return group_with_child_options::owner_type(
        *this, 
        widget,
        config_path,
        config_id
    );
}


group_slider_options::owner_type group_builder::slider(const xstr& name, float& value, float min, float max,
                                                       const std::format_string<float>& format)
{
    last_widget_name_ = name;

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::slider>(
            instance(), instance(),
            name, &value,
            min, max,
            format
        )
    );

    const auto config_id = instance()->config().add_module(
        std::make_unique<default_config_module<float>>(
            build_config_path("slider"),
            &value
        )
    );

    return group_slider_options::owner_type(
        *this, 
        widget,
        config_id
    );
}

group_spacing_options::owner_type group_builder::spacing()
{
    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::spacing>(
            instance(), instance()
        )
    );

    return group_spacing_options::owner_type(*this, widget);
}

group_textfield_options::owner_type group_builder::textfield(const xstr& name, std::function<void(const std::u32string& s)>&& callback)
{
    last_widget_name_ = name;

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::textfield_widget>(
            instance(), instance(),
            name, std::move(callback),
            textfield_type::text,
            textfield_flags::mouse_in_rect | textfield_flags::stop_on_return |
                textfield_flags::faded_text | textfield_flags::movable_caret,
            xstr(), /* default text */
            40u
        )
    );

    return group_textfield_options::owner_type(
        *this,
        widget,
        build_config_path("textfield"),
        _config::kInvalidModuleId
    );
}

group_with_child_options::owner_type group_builder::button(const xstr& name, const xstr& button_text, std::function<void()>&& callback)
{
    last_widget_name_ = name;

    auto* widget = group_instance_->add_widget(
        std::make_unique<::vo::button>(
            instance(), instance(),
            name, button_text,
            std::move(callback)
        )
    );

    return group_with_child_options::owner_type(
        *this,
        widget,
        build_config_path("button"),
        _config::kInvalidModuleId
    );
}

group_access<childwindow_options> group_builder::childwindow(const xstr& name)
{
    last_widget_name_ = name;

    const auto overlay_id = instance()->create_overlay(
        std::make_unique<::vo::childwindow>(
            instance(), instance(), instance(),
            name
        )
    );

    last_childwindow_ = overlay_id;

    return group_access<childwindow_options>(
        *this,
        instance()->get_overlay<::vo::childwindow>(overlay_id)
    );
}

group_keybind_options::owner_type group_builder::keybind(::vo::keybind& bind)
{
    last_keybind_ = instance()->input().add_keybind(&bind);

    return group_keybind_options::owner_type(
        *this,
        last_keybind_
    );
}

void_end_