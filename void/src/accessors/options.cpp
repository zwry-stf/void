#include <void/accessors/options.h>
#include <void/void.h>


void_begin_

void options::assert_noinit()
{
    assert(!instance()->is_initialized());
}

void_end_