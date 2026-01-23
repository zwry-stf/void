#include <void/builder/tab.h>
#include <void/builder/builder.h>
#include <void/void.h>
#include <contents/container/child_tab_normal.h>
#include <contents/container/tab_normal.h>
#include <config/config_tab.h>
#include <theme/theme_tab.h>


void_begin_

/// child_tab_base_builder

child_tab_base_builder::child_tab_base_builder(void_* instance, menu_builder* builder, child_tab* tab_instance)
    : base_builder_object(instance, builder),
      tab_instance_(tab_instance)
{
}

child_tab_base_builder& child_tab_base_builder::icon(int resource_id)
{
    get_child_tab()->set_icon(resource_id);
    return *this;
}

/// child_tab_normal_builder

child_tab_normal_builder& child_tab_normal_builder::icon(int resource_id)
{
    child_tab_base_builder::icon(resource_id);
    return *this;
}

child_tab_normal_builder& child_tab_normal_builder::disable_scroll()
{
    get_child_tab<child_tab_normal>()->disable_scroll(true);

    return *this;
}

group_builder child_tab_normal_builder::left_group(const xstr& name)
{
    set_last_group_name(name);

    auto* group = get_child_tab<child_tab_normal>()->add_group(
        std::make_unique<::vo::group>(
            instance(),
            name,
            group_area::left
        )
    );

    return group_builder(instance(), builder(), group);
}

group_builder child_tab_normal_builder::right_group(const xstr& name)
{
    set_last_group_name(name);

    auto* group = get_child_tab<child_tab_normal>()->add_group(
        std::make_unique<::vo::group>(
            instance(),
            name,
            group_area::right
        )
    );

    return group_builder(instance(), builder(), group);
}


/// tab_builder

tab_builder::tab_builder(void_* instance, menu_builder* builder, tab_normal* tab_instance)
    : base_builder_object(instance, builder),
      tab_instance_(tab_instance)
{
}

child_tab_normal_builder tab_builder::child(const xstr& name)
{
    set_last_child_name(name);

    auto* child_tab = tab_instance_->add_child_tab(
        std::make_unique<::vo::child_tab_normal>(
            instance(), instance(), instance(),
            name
        )
    );

    return child_tab_normal_builder(instance(), builder(), child_tab);
}

child_tab_base_builder tab_builder::config_tab(const xstr& name)
{
    auto* child_tab = tab_instance_->add_child_tab(
        std::make_unique<::vo::config_tab>(
            instance(), instance(), instance(),
            instance()->config().get_config_instance(),
            name
        )
    );

    return child_tab_base_builder(instance(), builder(), child_tab);
}

child_tab_base_builder tab_builder::theme_tab(const xstr& name)
{
    auto* child_tab = tab_instance_->add_child_tab(
        std::make_unique<::vo::theme_tab>(
            instance(), instance(), instance(),
            instance()->theme().get_theme_instance(),
            name
        )
    );

    return child_tab_base_builder(instance(), builder(), child_tab);
}

child_tab_base_builder tab_builder::custom_tab_impl(std::unique_ptr<child_tab>&& tab)
{
    auto* child_tab = tab_instance_->add_child_tab(std::move(tab));
    
    return child_tab_base_builder(instance(), builder(), child_tab);
}

tab_builder& tab_builder::icon(int resource_id)
{
    tab_instance_->set_icon(resource_id);

    return *this;
}

void_end_