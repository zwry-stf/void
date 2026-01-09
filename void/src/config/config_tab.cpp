#include "config_tab.h"
#include <void/void.h>
#include "config.h"
#include "config_drawable.h"
#include <void/contents/widgets/scrollbar.h>


void_begin_

enum class tab_selected_ids : std::int32_t {
    selected_child_tab,
    selected_add,
    selected_max
};

config_tab::config_tab(void_* instance, input_owner* input_owner,
                       input_owner_overlay* overlay_owner, _config* config_instance, const xstr& name)
    : child_tab(instance, input_owner,
                std::to_underlying(tab_selected_ids::selected_max), 
                std::to_underlying(tab_selected_ids::selected_child_tab),
                overlay_owner, name),
      config_instance_(config_instance)
{
}

float config_tab::update(float x, float y, bool selected, const render_input& input)
{
    render_no_results_ = false;
    auto& style = instance()->style();
    const float spacing = style.spacing->get(instance()->scale());
    scrollbar_width_ = spacing;

    const float ret = child_tab::update(x, y, selected, input);
    if (!selected)
        return ret;

    instance()->toggle_search(true);

    config_instance_->update(selected_config_);

    auto& renderer = instance()->renderer();
    auto& util = instance()->util();
    const auto& menu_pos = instance()->pos();

    scrollbar_width_ = spacing;

    // search
    if (last_search_ != instance()->get_search_text()) {
        last_search_ = instance()->get_search_text();
        if (last_search_.empty()) {
            for (auto& cfg : config_instance_->configs_)
                cfg->set_skipped(false);
        }
        else {
            // convert to lowercase wstring
            std::wstring search_text;
            search_text.reserve(last_search_.size());
            for (const auto c : last_search_) {
                wchar_t buf[4];
                const std::uint32_t l = r2::unicode::put_char_to_array<wchar_t>(c, buf);
                if (l == r2::unicode::codepoint_invalid)
                    continue;

                for (std::uint32_t i = 0u; i < l; i++) {
                    search_text += static_cast<wchar_t>(
                        std::tolower(static_cast<int>(buf[i]))
                    );
                }
            }

            for (auto& cfg : config_instance_->configs_)
                cfg->search(search_text);
        }
    }

    // drawables
    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float top_bar_height = style.top_bar_height.get(instance()->scale());
    const float config_height = style.config_height.get(instance()->scale());

    const float side_spacing = spacing * 2.f;
    const float full_width = menu_pos.w - sidebar_width - scrollbar_width_;
    const float full_inner_width = full_width - side_spacing * 2.f;
    const float pos_x = menu_pos.x + sidebar_width;
    const float pos_y = menu_pos.y + top_bar_height + side_spacing - scrollbar_->get_scroll();

    const float lowest_pos = menu_pos.y + menu_pos.h;
    const float highest_pos = menu_pos.y + top_bar_height;

    float ypos = pos_y;
    std::int32_t config_id = 0;
    highest_pos_ = 0.f;
    for (auto& cfg : config_instance_->configs_) {
        if (!cfg->is_skipped()) {
            const bool occluded = ypos > lowest_pos ||
                ypos + config_height < highest_pos;

            cfg->update(
                pos_x + side_spacing,
                ypos,
                full_inner_width,
                input,
                selected_config_ == config_id,
                occluded
            );

            ypos += config_height + spacing;
            highest_pos_ += config_height + spacing;
        }

        config_id++;
    }

    highest_pos_ = std::max(
        highest_pos_ - (config_height + spacing), // remove last config
        0.f
    );

    // add button
    add_animation_hovered_ = util.lerp(
        add_animation_hovered_,
        input.is_hovered(this, tab_selected_ids::selected_add)
    );
    add_animation_selected_ = util.lerp(
        add_animation_selected_,
        input.is_selected(this, tab_selected_ids::selected_add)
    );

    add_visible_ = false;
    if (!instance()->is_search_opened() ||
        instance()->get_search_text().empty()) {
        add_visible_ = true;

        const float button_spacing = std::round(config_height * 0.25f);
        const float button_size = config_height - button_spacing * 2.f;

        add_pos_ = r2::rectf{ 
            pos_x + side_spacing + std::round((full_inner_width - button_size) * 0.5f), 
            ypos + button_spacing,
            button_size,
            button_size 
        };
    }

    // no results found
    if (instance()->is_search_opened() &&
        !instance()->get_search_text().empty() &&
        ypos == pos_y) {
        render_no_results_ = true;

        if (!no_results_width_calculated_) {
            if (renderer.get_text_width_strict(kNoResultsText, no_results_width_))
                no_results_width_calculated_ = true;
            else
                no_results_width_ = renderer.get_text_width(kNoResultsText);
            no_results_width_ = std::ceil(no_results_width_);
        }

        no_results_pos_ = r2::vec2{
            pos_x + std::round((full_width - no_results_width_) * 0.5f),
            menu_pos.y + top_bar_height + side_spacing
        };
    }
 
    return ret;
}

void config_tab::render(float alpha, bool selected)
{
    child_tab::render(alpha, selected);

    if (!selected)
        return;

    auto& renderer = instance()->renderer();
    auto& style = instance()->style();
    const auto& menu_pos = instance()->pos();

    // drawables
    const float border_size = style.border_size.get(instance()->scale());
    const float sidebar_width = style.sidebar_width.get(instance()->scale());
    const float top_bar_height = style.top_bar_height.get(instance()->scale());

    renderer.push_clip_rect(
        r2::vec2(menu_pos.x + sidebar_width,
            menu_pos.y + top_bar_height),
        r2::vec2(menu_pos.x + menu_pos.w - border_size,
            menu_pos.y + menu_pos.h - border_size),
        true
    );

    for (auto& cfg : config_instance_->configs_) {
        if (!cfg->is_skipped()) {
            cfg->render(alpha);
        }
    }

    if (add_visible_) {
        const r2::color button_color = style.accent2().interp(
            style.accent(),
            std::max(
                add_animation_hovered_ * 0.6f - add_animation_selected_ * 0.3f, 0.f)
        ).alpha(alpha);

        const r2::vec2 button_min = r2::vec2(add_pos_.x, add_pos_.y);
        const r2::vec2 button_max = button_min + r2::vec2(add_pos_.w, add_pos_.h);

        const auto outer_offset = r2::vec2(border_size);
        renderer.add_shadow_rect_filled(
            button_min - outer_offset, button_max + outer_offset,
            style.accent().alpha(alpha * add_animation_selected_ * 0.5f),
            0.f, /* rounding */
            (add_pos_.h * 0.5f)
        );

        renderer.add_rect_inner_fast(
            button_min - outer_offset, button_max + outer_offset,
            button_color,
            border_size * 2.f
        );

        const r2::vec2 mid = (button_min + button_max) * r2::vec2(0.5f);

        renderer.prim_rect(
            r2::vec2(mid.x - border_size, button_min.y + border_size * 2.f),
            r2::vec2(mid.x + border_size, button_max.y - border_size * 2.f),
            button_color
        );

        renderer.prim_rect(
            r2::vec2(button_min.x + border_size * 2.f, mid.y - border_size),
            r2::vec2(button_max.x - border_size * 2.f, mid.y + border_size),
            button_color
        );
    }

    // no results found
    if (render_no_results_) {
        renderer.add_text(
            no_results_pos_,
            style.text().alpha(alpha),
            kNoResultsText
        );
    }

    renderer.pop_clip_rect();

    render_scrollbar(alpha);
}

input_response config_tab::input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id)
{
    // parent input
    auto res = child_tab::input(input, selected_tab, tab_id);
    if (res.is_handled())
        return res;

    if (selected_tab != tab_id)
        return input_response::empty();

    res = input_scrollbar(input);
    if (res.is_handled())
        return res;

    // drawables
    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    const float top_bar_height = instance()->style().top_bar_height.get(instance()->scale());

    if (!input.event().has_cursor_pos() ||
        mouse_y >= instance()->pos().y + top_bar_height) {
        std::int32_t config_id = 0;
        for (auto& cfg : config_instance_->configs_) {
            if (!cfg->is_skipped()) {
                res = cfg->input(
                    input, 
                    selected_config_, 
                    config_id
                );
                if (res.is_handled())
                    return res;
            }

            config_id++;
        }
    }
    else if (!input.nothing_selected()) {
        std::int32_t config_id = 0;
        for (auto& cfg : config_instance_->configs_) {
            if (!cfg->is_skipped()) {
                if (input.is_selected(cfg.get())) {
                    res = cfg->input(
                        input,
                        selected_config_,
                        config_id
                    );
                    if (res.is_handled())
                        return res;
                }
            }

            config_id++;
        }
    }

    if (!input.nothing_selected() &&
        !input.is_selected(this, tab_selected_ids::selected_add))
        return input_response::empty();

    // add button
    if (add_visible_) {
        if (input.event().is_message(message_type::mouse_move)) {
            if (input.is_selected(this, tab_selected_ids::selected_add) ||
                (util::is_in_rect(mouse_x, mouse_y, add_pos_) && input.nothing_selected())) {
                input.set_hovered(this, tab_selected_ids::selected_add);

                instance()->cursors().set_cursor(cursor::hand);

                return input_response::handled();
            }
        }

        else if (input.event().is_message(message_type::mouse_button_down) &&
                 input.event().get_mouse_button() == mouse_button::left) {
            if (util::is_in_rect(mouse_x, mouse_y, add_pos_)) {
                input.set_selected(this, tab_selected_ids::selected_add);

                return input_response::handled();
            }
        }

        else if (input.event().is_message(message_type::mouse_button_up) &&
                 input.event().get_mouse_button() == mouse_button::left &&
                 input.is_selected(this, tab_selected_ids::selected_add)) {
            if (util::is_in_rect(mouse_x, mouse_y, add_pos_)) {
                config_instance_->create_new();
            }

            input.clear_selected();

            return input_response::handled();
        }
    }

    return input_response::empty();
}

void config_tab::on_activate(bool parent_change, bool first)
{
    child_tab::on_activate(parent_change, first);

    for (auto& c : config_instance_->configs_)
        c->on_activate();

    instance()->set_search_text(std::u32string());

    add_animation_hovered_ = 0.f;
    add_animation_selected_ = 0.f;

    config_instance_->refresh_configs(selected_config_);
}

void config_tab::on_scale_change()
{
    child_tab::on_scale_change();

    for (auto& c : config_instance_->configs_)
        c->on_scale_changed();

    no_results_width_calculated_ = false;
}

void_end_