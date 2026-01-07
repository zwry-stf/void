#include <void/contents/container/tab.h>


void_begin_

tab::tab(void_* instance, input_owner_overlay* overlay_owner, bool is_container)
    : vobj(instance),
      parent_overlay_owner_(overlay_owner),
      is_container_(is_container)
{
}

void_end_