#pragma once
#include "base.h"

#include <functional>
#include <format>
#include <utility>
#include <vector>

#include <r2/renderer_definitions.h>
#include <void/contents/widgets/list_options.h>
#include <void/util/xstr.h>


void_begin_


// forward declarations
class widget;
class widget_child;
class group;
class childwindow;
class menu_builder;

template <class T>
class group_access;

template <class Group, class T>
class childwindow_access;

class group_base_options : public base_builder_object {
protected:
    widget* const widget_instance_;

public:
    group_base_options(void_* instance, menu_builder* builder,
                       widget* widget_instance);

protected:
    void disabled(bool& state);
    void disabled_inverted(bool& state);
    void disabled(std::function<bool()>&& callback);
    void condition(std::function<bool()>&& callback);
    
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
                       owner_type* group_instance, widget* widget_instance);

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);
};

class group_with_child_options : public group_base_options {
public:
    using owner_type = group_access<group_with_child_options>;
protected:
    owner_type* group_instance_;

public:
    group_with_child_options(void_* instance, menu_builder* builder,
                             owner_type* group_instance, widget* widget_instance);

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
    owner_type& dropdown(list_options* options, std::size_t& selected);
    owner_type& multiselect(list_options* options, std::vector<bool>& selected);
    /// Adds the last created child window as child widget.
    owner_type& last_childwindow();
    /// Add a custom child widget to the last created widget.
    owner_type& custom_child(std::unique_ptr<widget_child>&& child);
};

class group_slider_options : public group_base_options {
public:
    using owner_type = group_access<group_slider_options>;
protected:
    owner_type* group_instance_;

public:
    group_slider_options(void_* instance, menu_builder* builder,
                         owner_type* group_instance, widget* widget_instance);
    
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
    /// Sets how many decimals the slider should round to.
    owner_type& decimal_count(int count);
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
                         owner_type* group_instance, widget* widget_instance);

public:
    /// Disable widget when state becomes true.
    owner_type& disabled(bool& state);

    /// Disable widget when state becomes false.
    owner_type& disabled_inverted(bool& state);

    /// Disable widget when callback returns true.
    owner_type& disabled(std::function<bool()>&& callback);

    /// Hide widget when callback returns false.
    owner_type& condition(std::function<bool()>&& callback);
};

class overlay_with_child_options : public group_base_options {
public:
    using owner_type = childwindow_access<childwindow_options, overlay_with_child_options>;
protected:
    owner_type* const group_instance_;
    childwindow* const childwindow_instance_;
    
public:
    overlay_with_child_options(void_* instance, menu_builder* builder,
                               owner_type* group_instance, widget* widget_instance,
                               childwindow* childwindow_instance);

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
    owner_type& dropdown(list_options* options, std::size_t& selected);
    owner_type& multiselect(list_options* options, std::vector<bool>& selected);
    /// Add a custom child widget to the last created widget.
    owner_type& custom_child(std::unique_ptr<widget_child>&& child);
};

class overlay_slider_options : public group_base_options {
public:
    using owner_type = childwindow_access<childwindow_options, overlay_slider_options>;
protected:
    owner_type* group_instance_;
    
public:
    overlay_slider_options(void_* instance, menu_builder* builder,
                           owner_type* group_instance, widget* widget_instance);

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
    /// Sets how many decimals the slider should round to.
    owner_type& decimal_count(int count);
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

    friend class overlay_with_child_options;

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
    overlay_with_child_options::owner_type toggle(const xstr& name, bool& value);
};

class group_builder : public base_builder_object {
protected:
    class group* const group_instance_;
    xstr last_widget_name_{};
    std::int32_t last_childwindow_{ -1 };

    template <class T>
    friend class group_access;
    friend class group_with_child_options;

protected:
    group_builder(const group_builder&) = default;

public:
    group_builder(void_* instance, menu_builder* builder, group* group_instance);

public:
    childwindow_options::owner_type childwindow(const xstr& name);
    group_with_child_options::owner_type button(const xstr& name, const xstr& button_text, std::function<void()>&& callback);
    group_with_child_options::owner_type colorpicker(const xstr& name, r2::color& value, bool has_alpha = true);
    group_with_child_options::owner_type dropdown(const xstr& name, list_options* options, std::size_t& selected);
    group_item_options::owner_type list(const xstr& name, list_options* options, std::size_t& selected,
                                        bool has_search = true, int rows = 5);
    group_with_child_options::owner_type multiselect(const xstr& name, list_options* options, std::vector<bool>& selected);
    group_slider_options::owner_type slider(const xstr& name, float& value, float min = 0.f, float max = 1.f,
                                            const std::format_string<float>& format = "{:.2f}");
    group_spacing_options::owner_type spacing();
    group_with_child_options::owner_type toggle(const xstr& name, bool& value);
};

template <class T>
class group_access : public group_builder {
    // cant use requires, so we'll statically assert here
    static_assert(std::is_base_of_v<base_builder_object, T>,
        "group_access<T>: T must derive from base_builder_object");

private:
    T options_;

public:
    template <class T>
    group_access(const group_builder& v, T* item_instance)
        : group_builder(v.instance(), v.builder(), v.group_instance_),
          options_(v.instance(), v.builder(), this, item_instance) { 
        last_widget_name_ = v.last_widget_name_;
        last_childwindow_ = v.last_childwindow_;
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