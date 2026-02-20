#pragma once
#include <void/contents/container/child_tab.h>
#include "group.h"
#include <vector>
#include <string>


void_begin_

class child_tab_normal : public child_tab {
private:
    std::vector<std::unique_ptr<group>> groups_;
    std::u32string last_search_;
    bool scroll_disabled_{ false };

    float full_width_;
    bool no_results_found_{ false };
    bool no_results_width_calculated_{ false };
    float no_results_width_;

    float positions_areas_[std::to_underlying(group_area::max)];
    float highest_areas_[std::to_underlying(group_area::max)];

public:
    child_tab_normal(void_* instance, input_owner* input_owner, 
                     input_owner_overlay* overlay_owner, const xstr& name);

public:
    virtual float update(float x, float y, bool selected, const render_input& input) override;
    virtual void render(float alpha, bool selected) override;
    virtual input_response input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id);

    virtual void on_activate(bool parent_change, bool first = false) override;
    virtual void on_scale_change() override;

    group* add_group(std::unique_ptr<group>&& group) {
        groups_.push_back(std::move(group));
        return groups_.back().get();
    }
    void disable_scroll(bool s) noexcept {
        scroll_disabled_ = s; 
    }
};

void_end_