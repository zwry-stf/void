#pragma once
#include <void/contents/container/tab.h>
#include <void/accessors/icons.h>
#include <vector>


void_begin_

class sidebar : protected vobj {
private:
    std::vector<std::unique_ptr<tab>> tabs_;
    std::int32_t selected_tab_{ -1 };
    icons::icon_handle icon_handle_{ icons::kInvalidHandle };

public:
    using vobj::vobj;

public:
    void render(const render_input& input);
    input_response input(const input_base& input);

    void on_activate();
    void on_scale_change();

    tab* add_tab(std::unique_ptr<tab>&& tab) {
        if (selected_tab_ == -1 &&
            tab->is_container())
            selected_tab_ = static_cast<std::int32_t>(tabs_.size());

        tabs_.push_back(std::move(tab));
        return tabs_.back().get();
    }
};

void_end_