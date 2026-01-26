#pragma once
#include "base.h"

#include <functional>
#include <format>
#include <utility>
#include <vector>

#include <r2/renderer_definitions.h>
#include <void/contents/widgets/list_options.h>
#include <void/util/xstr.h>
#include <void/contents/widgets/textfield.h>
#include <void/contents/input/keybind.h>


void_begin_


// forward declarations
class widget;
class widget_child;
class group;
class childwindow;
class keybind_owner;
class menu_builder;

template <class T>
class group_access;

template <class Group, class T>
class childwindow_access;

class group_base_options : public base_builder_object {
protected:
    widget* const widget_instance_;
    std::size_t config_id_;

public:
    group_base_options(void_* instance, menu_builder* builder,
                       widget* widget_instance, std::size_t config_id);

protected:
    void disabled(bool& state);
    void disabled_inverted(bool& state);
    void disabled(std::function<bool()>&& callback);
    void condition(std::function<bool()>&& callback);
    void no_config();
    
protected:
    template <typename T = widget>
        requires (std::is_base_of_v<widget, T>)
    [[nodiscard]] T* get_widget() const noexcept {
        return reinterpret_cast<T*>(widget_instance_);
    }
};

class group_item_options : public group_base_options {
public:
    using owner_type = group_access<group_item_options>;
protected:
    owner_type* group_instance_;

public:
    group_item_options(void_* instance, menu_builder* builder, 
                       owner_type* group_instance, widget* widget_instance,
                       std::size_t config_id);

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

    /// remove widget from config
    owner_type& no_config();
};

class group_with_child_base_options : public group_base_options {
protected:
    const xstr name_;
    const xstr config_path_;

protected:
    xstr build_child_config_path(const xstr& type);

public:
    group_with_child_base_options(void_* instance, menu_builder* builder,
                                  widget* widget_instance, const xstr& config_path, 
                                  const xstr& name, std::size_t config_id);

protected:
    void colorpicker(r2::color& value, bool has_alpha = true);
    void optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha = true);
    void dropdown(list_options* options, std::size_t& selected);
    void multiselect(list_options* options, std::vector<bool>& selected);
    void last_childwindow(std::int32_t);
    void last_keybind(keybind_owner* bind);
    void custom_child(std::unique_ptr<widget_child>&& child);
};

class group_with_child_options : public virtual group_with_child_base_options {
public:
    using owner_type = group_access<group_with_child_options>;
protected:
    owner_type* group_instance_;

public:
    group_with_child_options(void_* instance, menu_builder* builder,
                             owner_type* group_instance, widget* widget_instance,
                             const xstr& config_path, const xstr& name,
                             std::size_t config_id);

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

    /// remove widget from config
    owner_type& no_config();

public:
    owner_type& colorpicker(r2::color& value, bool has_alpha = true);
    owner_type& optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha = true);
    owner_type& dropdown(list_options* options, std::size_t& selected);
    owner_type& multiselect(list_options* options, std::vector<bool>& selected);
    /// Adds the last created child window as child widget.
    owner_type& last_childwindow();
    /// Adds the last created keybind as child widget.
    owner_type& last_keybind();
    /// Add a custom child widget to the last created widget.
    owner_type& custom_child(std::unique_ptr<widget_child>&& child);
};

class group_textfield_options : public group_with_child_base_options {
public:
    using owner_type = group_access<group_textfield_options>;
protected:
    owner_type* group_instance_;

private:
    xstr default_text_{};
    textfield_type type_{ textfield_type::text };
    std::size_t max_length_{ 40u };
    bool faded_text_{ true };

public:
    group_textfield_options(void_* instance, menu_builder* builder,
                            owner_type* group_instance, widget* widget_instance,
                            const xstr& config_path, const xstr& name,
                            std::size_t config_id);
    virtual ~group_textfield_options();

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

public:
    owner_type& colorpicker(r2::color& value, bool has_alpha = true);
    owner_type& optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha = true);
    owner_type& dropdown(list_options* options, std::size_t& selected);
    owner_type& multiselect(list_options* options, std::vector<bool>& selected);
    /// Adds the last created child window as child widget.
    owner_type& last_childwindow();
    /// Adds the last created keybind as child widget.
    owner_type& last_keybind();
    /// Add a custom child widget to the last created widget.
    owner_type& custom_child(std::unique_ptr<widget_child>&& child);

public:
    /// Set the default text shown when text is empty
    // Default: empty
    owner_type& default_text(const xstr& text);

    /// Set the textfield's type
    // Default: textfield_type::text
    owner_type& type(textfield_type type);

    /// Enable/Disable faded text
    // Default: true
    owner_type& faded(bool state = true);

    /// Set the max string length
    // Default: 40
    owner_type& max_length(std::size_t length);
};

class group_slider_options : public group_base_options {
public:
    using owner_type = group_access<group_slider_options>;
protected:
    owner_type* group_instance_;

public:
    group_slider_options(void_* instance, menu_builder* builder,
                         owner_type* group_instance, widget* widget_instance,
                         std::size_t config_id);
    
public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

    /// remove widget from config
    owner_type& no_config();

public:
    /// Sets how many decimals the slider should round to.
    owner_type& decimal_count(int count);

    /// Adds a custom format applied when condition returns true on value.
    owner_type& format_condition(std::function<bool(float, std::string&)>&& condition);
};

class group_spacing_options : public group_base_options {
public:
    using owner_type = group_access<group_spacing_options>;
protected:
    owner_type* group_instance_;

public:
    group_spacing_options(void_* instance, menu_builder* builder,
                          owner_type* group_instance, widget* widget_instance);
    
public:
    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);
};

// overlay widget options

class childwindow_options;
class overlay_item_options : public group_base_options {
public:
    using owner_type = childwindow_access<childwindow_options, overlay_item_options>;
protected:
    owner_type* group_instance_;
    
public:
    overlay_item_options(void_* instance, menu_builder* builder,
                         owner_type* group_instance, widget* widget_instance,
                         std::size_t config_id);

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

    /// remove widget from config
    owner_type& no_config();
};

class overlay_with_child_base_options : public group_base_options {
protected:
    childwindow* const childwindow_instance_;
    const xstr config_path_;

protected:
    xstr build_overlay_child_config_path(const xstr& type);
    
public:
    overlay_with_child_base_options(void_* instance, menu_builder* builder,
                                    widget* widget_instance, childwindow* childwindow_instance, 
                                    const xstr& config_path, std::size_t config_id);
public:
    void colorpicker(r2::color& value, bool has_alpha = true);
    void optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha = true);
    void dropdown(list_options* options, std::size_t& selected);
    void multiselect(list_options* options, std::vector<bool>& selected);
    void custom_child(std::unique_ptr<widget_child>&& child);
};

class overlay_with_child_options : public overlay_with_child_base_options {
public:
    using owner_type = childwindow_access<childwindow_options, overlay_with_child_options>;
protected:
    owner_type* const group_instance_;
    
public:
    overlay_with_child_options(void_* instance, menu_builder* builder,
                               owner_type* group_instance, widget* widget_instance,
                               childwindow* childwindow_instance, const xstr& config_path,
                               std::size_t config_id);

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

    /// remove widget from config
    owner_type& no_config();

public:
    owner_type& colorpicker(r2::color& value, bool has_alpha = true);
    owner_type& optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha = true);
    owner_type& dropdown(list_options* options, std::size_t& selected);
    owner_type& multiselect(list_options* options, std::vector<bool>& selected);
    /// Add a custom child widget to the last created widget.
    owner_type& custom_child(std::unique_ptr<widget_child>&& child);
};

class overlay_textfield_options : public overlay_with_child_base_options {
public:
    using owner_type = childwindow_access<childwindow_options, overlay_textfield_options>;
protected:
    owner_type* const group_instance_;

private:
    xstr default_text_{};
    textfield_type type_{ textfield_type::text };
    std::size_t max_length_{ 40u };
    bool faded_text_{ true };
    
public:
    overlay_textfield_options(void_* instance, menu_builder* builder,
                              owner_type* group_instance, widget* widget_instance,
                              childwindow* childwindow_instance, const xstr& config_path,
                              std::size_t config_id);
    virtual ~overlay_textfield_options() override;

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

public:
    owner_type& colorpicker(r2::color& value, bool has_alpha = true);
    owner_type& optional_colorpicker(r2::color& value, bool& enabled, bool has_alpha = true);
    owner_type& dropdown(list_options* options, std::size_t& selected);
    owner_type& multiselect(list_options* options, std::vector<bool>& selected);
    /// Add a custom child widget to the last created widget.
    owner_type& custom_child(std::unique_ptr<widget_child>&& child);

public:
    /// Set the default text shown when text is empty
    // Default: empty
    owner_type& default_text(const xstr& text);

    /// Set the textfield's type
    // Default: textfield_type::text
    owner_type& type(textfield_type type);

    /// Enable/Disable faded text
    // Default: true
    owner_type& faded(bool state = true);

    /// Set the max string length
    // Default: 40
    owner_type& max_length(std::size_t length);
};

class overlay_slider_options : public group_base_options {
public:
    using owner_type = childwindow_access<childwindow_options, overlay_slider_options>;
protected:
    owner_type* group_instance_;
    
public:
    overlay_slider_options(void_* instance, menu_builder* builder,
                           owner_type* group_instance, widget* widget_instance,
                           std::size_t config_id);

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);

    /// remove widget from config
    owner_type& no_config();

public:
    /// Sets how many decimals the slider should round to.
    owner_type& decimal_count(int count);
    
    /// Adds a custom format applied when condition returns true on value.
    owner_type& format_condition(std::function<bool(float, std::string&)>&& condition);
};

class overlay_spacing_options : public group_base_options {
public:
    using owner_type = childwindow_access<childwindow_options, overlay_spacing_options>;
protected:
    owner_type* group_instance_;

public:
    overlay_spacing_options(void_* instance, menu_builder* builder,
                            owner_type* group_instance, widget* widget_instance);

public:
    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);
};

class childwindow_options : public base_builder_object {
public:
    using owner_type = group_access<childwindow_options>;
private:
    owner_type* const group_instance_;
    childwindow* const childwindow_instance_;
    xstr last_overlay_widget_name_{};

    friend class overlay_with_child_options;

protected:
    xstr build_overlay_config_path(const xstr& type);

public:
    childwindow_options(void_* instance, menu_builder* builder,
                        owner_type* group_instance, childwindow* childwindow_instance);

public:
    overlay_with_child_options::owner_type button(const xstr& name, const xstr& button_text, std::function<void()>&& callback);
    overlay_with_child_options::owner_type colorpicker(const xstr& name, r2::color& value, bool has_alpha = true);
    overlay_with_child_options::owner_type dropdown(const xstr& name, list_options* options, std::size_t& selected);
    overlay_item_options::owner_type list(const xstr& name, list_options* options, std::size_t& selected,
                                          bool has_search = true, int rows = 5);
    overlay_with_child_options::owner_type multiselect(const xstr& name, list_options* options, std::vector<bool>& selected);
    overlay_slider_options::owner_type slider(const xstr& name, float& value, float min = 0.f, float max = 1.f,
                                              const std::format_string<float>& format = "{:.2f}");
    overlay_spacing_options::owner_type spacing();
    overlay_textfield_options::owner_type textfield(const xstr& name, std::function<void(const std::u32string& s)>&& callback);
    overlay_with_child_options::owner_type toggle(const xstr& name, bool& value);

    overlay_item_options::owner_type custom_widget(std::unique_ptr<widget>&& widget);
};

class keybind_options_base : public base_builder_object {
protected:
    keybind_owner* const keybind_instance_;

public:
    keybind_options_base(void_* instance, menu_builder* builder,
                         keybind_owner* keybind_instance);

public:
    void disabled(bool& state);
    void disabled_inverted(bool& state);
    void disabled(std::function<bool()>&& callback);
    void mode(keybind_mode mode);
    void key(::vo::key k);
    void key(mouse_button k);
    void name(const xstr& name);
};

class group_keybind_options : public keybind_options_base {
public:
    using owner_type = group_access<group_keybind_options>;
private:
    owner_type* const group_instance_;
    std::size_t config_id1_;
    std::size_t config_id2_;
    
public:
    group_keybind_options(void_* instance, menu_builder* builder,
                          owner_type* group_instance, keybind_owner* keybind_instance,
                          std::size_t config_id1, std::size_t config_id2);

public:
    /// Disable keybind when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable keybind when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable keybind when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    owner_type& mode(keybind_mode mode);
    owner_type& key(::vo::key k);
    owner_type& key(mouse_button k);
    owner_type& no_config();
    owner_type& name(const xstr& name);
};

class group_builder : public base_builder_object {
protected:
    class group* const group_instance_;
    xstr last_widget_name_{};
    std::int32_t last_childwindow_{ -1 };
    keybind_owner* last_keybind_{ nullptr };

    template <class T>
    friend class group_access;
    friend class group_with_child_options;
    friend class group_textfield_options;
    friend class childwindow_options;

protected:
    xstr build_config_path(const xstr& type);

protected:
    group_builder(const group_builder&) = default;

public:
    group_builder(void_* instance, menu_builder* builder, group* group_instance);

public:
    childwindow_options::owner_type childwindow(const xstr& name);
    group_keybind_options::owner_type keybind(::vo::keybind& bind);
    group_with_child_options::owner_type button(const xstr& name, const xstr& button_text, std::function<void()>&& callback);
    group_with_child_options::owner_type colorpicker(const xstr& name, r2::color& value, bool has_alpha = true);
    group_with_child_options::owner_type dropdown(const xstr& name, list_options* options, std::size_t& selected);
    group_item_options::owner_type list(const xstr& name, list_options* options, std::size_t& selected,
                                        bool has_search = true, int rows = 5);
    group_with_child_options::owner_type multiselect(const xstr& name, list_options* options, std::vector<bool>& selected);
    group_slider_options::owner_type slider(const xstr& name, float& value, float min = 0.f, float max = 1.f,
                                            const std::format_string<float>& format = "{:.2f}");
    group_spacing_options::owner_type spacing();
    group_textfield_options::owner_type textfield(const xstr& name, std::function<void(const std::u32string& s)>&& callback);
    group_with_child_options::owner_type toggle(const xstr& name, bool& value);

    group_item_options::owner_type custom_widget(std::unique_ptr<widget>&& widget);
};

template <class T>
class group_access : public group_builder {
    // cant use requires, so we'll statically assert here
    static_assert(std::is_base_of_v<base_builder_object, T>,
        "group_access<T>: T must derive from base_builder_object");

private:
    T options_;

public:
    template <class... Types>
        requires std::constructible_from<
            T,
                decltype(std::declval<group_builder&>().instance()),
                decltype(std::declval<group_builder&>().builder()),
                group_access*,
                Types...
        >
    group_access(const group_builder& v, Types&&... args)
        : group_builder(v.instance(), v.builder(), v.group_instance_),
          options_(v.instance(), v.builder(), this, std::forward<Types>(args)...) { 
        last_widget_name_ = v.last_widget_name_;
        last_childwindow_ = v.last_childwindow_;
        last_keybind_ = v.last_keybind_;
    }

    group_access(const group_access<T>& v) = default;

public:
    /// Access the last widget's options (group-level).
    [[nodiscard]] T* operator->() noexcept {
        return &options_;
    }
};

template <class Group, class T>
class childwindow_access : public group_access<Group> {
    static_assert(std::is_base_of_v<base_builder_object, T>,
        "group_access<T>: T must derive from base_builder_object");

private:
    T cw_options_;

public:
    template <class... Types>
        requires std::constructible_from<
            T,
                decltype(std::declval<childwindow_access&>().instance()),
                decltype(std::declval<childwindow_access&>().builder()),
                childwindow_access*,
                Types...
        >
    childwindow_access(const group_access<Group>& v, Types&&... args)
        : group_access<Group>(v),
          cw_options_(this->instance(), this->builder(), this, std::forward<Types>(args)...) {
    }

public:
    /// Access the last widget's options inside the current childwindow.
    [[nodiscard]] T* l() noexcept {
        return &cw_options_;
    }
};

void_end_