#include "keybind_manager.h"


void_begin_

void keybind_manager::update()
{
    for (auto& bind : binds_)
        bind->update(instance());
}

keybind_owner* keybind_manager::add_keybind(keybind* bind, key default_key, keybind_mode mode)
{
    return binds_.emplace_back(std::make_unique<keybind_owner>(bind, default_key, mode)).get();
}

void_end_