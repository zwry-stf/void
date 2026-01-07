#include <void/accessors/theme.h>
#include <void/void.h>


void_begin_

_theme* theme::get_theme_instance() const noexcept
{
    return instance()->theme_.get();
}

void_end_