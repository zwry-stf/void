#include "child_tab_normal.h"
#include <void/contents/widgets/scrollbar.h>
#include <void/void.h>


void_begin_

child_tab_normal::child_tab_normal(void_* instance, input_owner* input_owner, 
                                   input_owner_overlay* overlay_owner, const xstr& name)
    : child_tab(
        instance, input_owner,
        overlay_owner, name
    )
{
}

float child_tab_normal::update(float x, float y, bool selected, const render_input& input)
{
    auto& style = instance()->style();
    const float spacing = style.spacing->get(instance()->scale());
    scrollbar_width_ = spacing;

    const float ret = child_tab::update(x, y, selected, input);
    if (!selected)
        return ret;

    instance()->toggle_search(true);

    const auto& menu_pos = instance()->pos();

    // search
    if (last_search_ != instance()->get_search_text()) {
        if (!instance()->is_search_opened())
            instance()->set_search_text(std::u32string());

        last_search_ = instance()->get_search_text();

        xstr search_str;
        for (auto& c : last_search_) {
            char buf[5];
            const std::uint32_t l = r2::unicode::put_char_to_array(
                static_cast<r2::unicode::unicode_type>(c), 
                buf
            );
            buf[l] = '\0';
            search_str.append_safe(&buf[0]);
            if (search_str.length() == search_str.kMaxSize)
                break;
        }

        for (auto& group : groups_)
            group->search(search_str);
    }
    
    // groups
    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float top_bar_height = style.top_bar_height.get(instance()->scale());
    const float side_spacing = spacing * 2.f;
    full_width_ = menu_pos.w - sidebar_width - (scroll_disabled_ ? 0.f : scrollbar_width_);
    const float full_inner_width = full_width_ - side_spacing * 2.f;
    const float area_width = std::floor(
        (full_inner_width - side_spacing *
            static_cast<float>(std::to_underlying(group_area::max) - 1)) / 
        static_cast<float>(std::to_underlying(group_area::max))
    );
    const float middle_spacing = full_inner_width - area_width * 2.f;
    const float pos_x = menu_pos.x + sidebar_width;

    const float group_spacing = std::round(spacing * 1.5f);

    const float pos_y = menu_pos.y + top_bar_height + side_spacing - (scroll_disabled_ ? 0.f : scrollbar_->get_scroll());
    for (auto& f : positions_areas_)
        f = pos_y;
    for (auto& f : highest_areas_)
        f = 0.f;

    float highest = 0.f;

    float substract_last = 0.f;
    no_results_found_ = true;
    for (auto& group : groups_) {
        auto area = group->area();

        bool is_empty;
        float substract = 0.f;
        const float offset = area == group_area::left ? 0.f : (area_width + middle_spacing);
        const float group_height = group->update(
            pos_x + side_spacing + offset, 
            positions_areas_[(int)area], area_width,
            input, 
            substract,
            is_empty
        );

        if (!is_empty) {
            positions_areas_[(int)area] += group_height;
            highest_areas_[(int)area] += group_height;
        }

        if (!group->is_occluded()) {
            no_results_found_ = false;
        }

        if (highest_areas_[(int)area] > highest) {
            highest = highest_areas_[(int)area];
            substract_last = substract;
        }

        if (!is_empty) {
            positions_areas_[(int)area] += group_spacing;
            highest_areas_[(int)area] += group_spacing;
        }
    }

    highest_pos_ = std::max(
        highest - substract_last, 
        0.f
    );
    if (highest_pos_ > 0.f)
        highest_pos_ += side_spacing;

    update_scrollbar();

    return ret;
}

void child_tab_normal::render(float alpha, bool selected)
{
    child_tab::render(alpha, selected);
    if (!selected)
        return;

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    const auto& menu_pos = instance()->pos();

    const float spacing = style.spacing->get(instance()->scale());
    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float top_bar_height = style.top_bar_height.get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());
    const float side_spacing = spacing * 2.f;
    const float pos_x = menu_pos.x + sidebar_width;

    instance()->fonts().bind_font_small();

    // groups
    renderer.push_clip_rect(
        r2::vec2(menu_pos.x + sidebar_width,
            menu_pos.y + top_bar_height),
        r2::vec2(menu_pos.x + menu_pos.w - border_size,
            menu_pos.y + menu_pos.h - border_size),
        true
    );

    for (auto& group : groups_) {
        group->render(alpha);
    }

    // no results found
    if (instance()->is_search_opened() && 
        !instance()->get_search_text().empty() &&
        no_results_found_) {
        constexpr xstr kNoResults = "no results found";

        if (!no_results_width_calculated_) {
            if (renderer.get_text_width_strict(kNoResults, no_results_width_))
                no_results_width_calculated_ = true;
            else
                no_results_width_ = renderer.get_text_width(kNoResults);
            no_results_width_ = std::ceil(no_results_width_);
        }

        const float text_spacing_x = std::round((full_width_ - no_results_width_) * 0.5f);
        renderer.add_text(
            r2::vec2(pos_x + text_spacing_x, 
                menu_pos.y + top_bar_height + side_spacing),
            style.text().alpha(alpha),
            kNoResults
        );
    }

    renderer.pop_clip_rect();

    if (!scroll_disabled_)
        render_scrollbar(alpha);
}

input_response child_tab_normal::input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id)
{
    // parent input
    auto res = child_tab::input(input, selected_tab, tab_id);
    if (res.is_handled())
        return res;

    if (selected_tab != tab_id)
        return input_response::empty();

    // groups input
    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    const float top_bar_height = instance()->style().top_bar_height.get(instance()->scale());

    const bool selected_only = input.event().has_cursor_pos() &&
        (mouse_y < instance()->pos().y + top_bar_height ||
            mouse_y > instance()->pos().y + instance()->pos().h);

    for (auto& group : groups_) {
        res = group->input(input, selected_only);
        if (res.is_handled())
            return res;
    }

    // scrollbar
    res = input_scrollbar(input);
    if (res.is_handled())
        return res;

    return input_response::empty();
}

void child_tab_normal::on_activate(bool parent_change, bool first)
{
    child_tab::on_activate(parent_change, first);

    for (auto& group : groups_)
        group->on_activate();

    instance()->set_search_text(std::u32string());
}

void child_tab_normal::on_scale_change()
{
    child_tab::on_scale_change();

    for (auto& group : groups_)
        group->on_scale_change();

    no_results_width_calculated_ = false;
}

void_end_