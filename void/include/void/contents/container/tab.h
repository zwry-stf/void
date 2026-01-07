#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <r2/renderer_definitions.h>
#include <void/contents/input/events.h>
#include <void/contents/input/input_response.h>


void_begin_

class input_owner_overlay;

class tab : protected vobj {
    const bool is_container_;
    input_owner_overlay* const parent_overlay_owner_;

public:
    tab(void_* instance, input_owner_overlay* overlay_owner, bool is_container = false);

    tab(const tab&) = delete;
    tab& operator=(const tab&) = delete;

public:
    virtual float update(float x, float y, float w, 
                         const render_input& input, bool selected) = 0;
    virtual void render(bool selected) = 0;
    virtual input_response input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id) = 0;

    virtual void on_activate(bool first = false) { (void)first; };
    virtual void on_scale_change() {};

    [[nodiscard]] bool is_container() const noexcept { 
        return is_container_;
    }
};

void_end_