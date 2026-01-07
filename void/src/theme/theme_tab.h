#pragma once
#include <void/contents/container/child_tab.h>
#include <string>


void_begin_

class theme_tab : public child_tab {
private:
    class _theme* const theme_instance_;

    std::u32string last_search_;

    float add_animation_hovered_{ 0.f };
    float add_animation_selected_{ 0.f };
    bool add_visible_;
    r2::rectf add_pos_;

    bool render_no_results_{ false };
    r2::vec2 no_results_pos_;

    bool  no_results_width_calculated_{ false };
    float no_results_width_;

    std::int32_t selected_theme_;

public:
    theme_tab(void_* instance, input_owner* input_owner, 
              input_owner_overlay* overlay_owner, class _theme* theme_instance, const xstr& name);

    inline static constexpr xstr kNoResultsText = "no results found";

public:
    virtual float update(float x, float y, bool selected, const render_input& input) override;
    virtual void render(float alpha, bool selected) override;
    virtual input_response input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id) override;

    virtual void on_activate(bool parent_change, bool first = false) override;
    virtual void on_scale_change() override;
};

void_end_