#pragma once
#include <void/def.h>


void_begin_

class void_;

class vobj {
private:
    void_* const instance_;

public:
    vobj(void_* instance) noexcept
        : instance_(instance) { }
    virtual ~vobj() = default;

protected:
    [[nodiscard]] auto* instance() const noexcept {
        return instance_;
    }
};

void_end_