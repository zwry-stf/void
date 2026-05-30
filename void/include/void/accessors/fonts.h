#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>


void_begin_

class fonts : protected vobj {
public:
    using vobj::vobj;

private:
    r2::font* font_small_{};
    r2::font* font_large_{};

public:
    [[nodiscard]] bool create();
    [[nodiscard]] bool build();

    void bind_font_small() const;
    void bind_font_large() const;
};

void_end_