#pragma once
#include "keybind.h"
#include <void/util/vobj.h>
#include <vector>


void_begin_

class keybind_manager : protected vobj {
private:
    std::vector<std::unique_ptr<keybind_owner>> binds_;

public:
    using vobj::vobj;

public:
    void update();

    [[nodiscard]] keybind_owner* add_keybind(keybind* bind, key default_key, keybind_mode mode);
    [[nodiscard]] const auto& binds() const noexcept {
        return binds_;
    }
};

void_end_