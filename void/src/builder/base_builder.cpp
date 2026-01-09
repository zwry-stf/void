#include <void/builder/base.h>
#include <void/builder/builder.h>
#include <void/void.h>


void_begin_

const xstr& base_builder_object::get_last_child_name() const
{
    return builder()->last_child_;
}

const xstr& base_builder_object::get_last_group_name() const
{
    return builder()->last_group_;
}

void base_builder_object::set_last_child_name(const xstr& s)
{
    builder()->last_child_ = s;
}

void base_builder_object::set_last_group_name(const xstr& s)
{
    builder()->last_group_ = s;
}

void_end_