#pragma once
#include "group.h"
#include <memory>
#include <concepts>


void_begin_

class child_tab_base_builder : protected base_builder_object {
private:
    class child_tab* const tab_instance_;

public:
    child_tab_base_builder(void_* instance, menu_builder* builder, class child_tab* tab_instance);

public:
    child_tab_base_builder& icon(int resource_id);

protected:
    template <typename T = child_tab>
        requires(std::is_base_of_v<child_tab, T>)
    [[nodiscard]] T* get_child_tab() const noexcept {
        return reinterpret_cast<T*>(tab_instance_);
    }
};

class child_tab_normal_builder : public child_tab_base_builder {
public:
    using child_tab_base_builder::child_tab_base_builder;

public:
    child_tab_normal_builder& icon(int resource_id);
    child_tab_normal_builder& disable_scroll();

    group_builder left_group(const xstr& name);
    group_builder right_group(const xstr& name);
};

class tab_builder : protected base_builder_object {
private:
    class tab_normal* const tab_instance_;

public:
    tab_builder(void_* instance, menu_builder* builder, class tab_normal* tab_instance);

public:
    child_tab_normal_builder child(const xstr& name);
    child_tab_base_builder config_tab(const xstr& name);
    child_tab_base_builder theme_tab(const xstr& name);

    template <class T, class... Args>
        requires(requires(void_* instance, const xstr& name, Args&&... args) {
        new T(instance, instance, instance, name, std::forward<Args>(args)...);
    })
    child_tab_base_builder custom_tab(const xstr& name, Args&&... args) {
        auto tab = std::make_unique<T>(
            instance(), instance(), instance(),
            name,
            std::forward<Args>(args)...
        );

        return custom_tab_impl(std::move(tab));
    }
    child_tab_base_builder custom_tab_impl(std::unique_ptr<child_tab>&& tab);

    tab_builder& icon(int resource_id);
};

void_end_