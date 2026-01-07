#include "list.h"
#include <void/void.h>


void_begin_

list::list(void_* instance, input_owner* input_owner, std::unique_ptr<list_options>&& options, 
	       const xstr& name, std::size_t* selected, std::int32_t num_rows, bool has_search)
	: widget(instance, input_owner, static_cast<std::int32_t>(num_rows)),
	  name_(name),
	  value_(selected),
	  num_rows_(num_rows),
	  has_search_(has_search),
	  options_(std::move(options))
{
    if (has_search) {
        text_field_ = std::make_unique<textfield>(
            instance,
            input_owner,
            textfield_type::text,
            textfield_flags::movable_caret | textfield_flags::mouse_in_rect | 
                textfield_flags::stop_on_return | textfield_flags::faded_text,
            xstr("search..."), /* default text */
            xstr(), /* post text */
            30 /* max length */
        );
    }

    scrollbar_ = std::make_unique<scrollbar>(instance, input_owner);
}

void list::update(float x, float y, float w, const render_input& input, bool occluded)
{
	widget::update(x, y, w, input, occluded);

    auto& util = instance()->util();
    auto& style = instance()->style();
    const float spacing = style.spacing->get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());

    spacing_ = std::round(spacing * 0.4f);
    row_height_ = text_size_small + spacing_;

    const float full_height = static_cast<float>(num_rows_) * row_height_ + spacing_;

    if (has_search_) {
        text_field_spacing_ = std::round(spacing * 0.3f);

        search_offset_ = text_size_small +
            text_field_spacing_ + text_field_spacing_ + border_size;
    }
    else {
        search_offset_ = 0.f;
    }

    last_pos_.h = search_offset_ + full_height;

	animation_hovered_search_ = util.lerp(
		animation_hovered_search_,
		input.is_hovered(text_field_.get())
	);

    render_no_results_ = false;

	const auto options_size = options_->size();

	if (options_size > member_states_.size()) {
        options_updated_ = true;
		member_states_.resize(options_size, list_member());
	}

    if (options_size > static_cast<std::size_t>(input_count())) {
        resize_input(static_cast<std::int32_t>(options_size));
    }

    constexpr sfloat kDropdownHeight = sfloat(26.f, true);

    const float list_width = std::round(
        std::min(std::max(w * 0.45f, kDropdownHeight.get(instance()->scale()) * 3.5f),
            last_pos_.w - spacing)
    );
	const float spacing_y = std::round(spacing * 0.25f);

    list_pos_.x = x + w - list_width;
    list_pos_.y = y + spacing_y + search_offset_;
    list_pos_.w = list_width;
    list_pos_.h = last_pos_.h - spacing_y * 2.f - search_offset_;

    // search
    if (has_search_) {
        const r2::vec2 search_min = r2::vec2(list_pos_.x, list_pos_.y - search_offset_);
        const r2::vec2 search_max = search_min + r2::vec2(list_pos_.w, search_offset_ - border_size);

        // text field
        const float text_field_spacing = std::round(spacing * 0.5f);

        text_field_->update(
            r2::rectf{
                search_min.x + text_field_spacing,
                search_min.y + text_field_spacing_,
                list_width - text_field_spacing * 2.f,
                text_size_small
            },
            input,
            occluded
        );

        // update search
        if (options_updated_ ||
            last_search_ != text_field_->get_string()) {
            options_updated_ = false;

            last_search_ = text_field_->get_string();

            if (last_search_.empty()) {
                for (auto& s : member_states_)
                    s.skipped = false;

                const float pos_until_selected = static_cast<float>(*value_) * row_height_;
                scrollbar_->set_scroll(pos_until_selected);
                scrollbar_->set_des_scroll(pos_until_selected);
                scrollbar_->set_scroll_speed(0.f);
            }
            else {
                xstr search_str;
                for (const auto c : last_search_) {
                    char buf[4];
                    const std::uint32_t l = r2::unicode::put_char_to_array<char>(c, buf);

                    bool exit = false;
                    for (std::uint32_t i = 0u; i < l; i++) {
                        if (search_str.length() == search_str.kMaxSize) {
                            exit = true;
                            break;
                        }

                        search_str += static_cast<char>(
                            std::tolower(static_cast<int>(buf[i]))
                        );
                    }

                    if (exit)
                        break;
                }

                bool found_selected = false;
                float pos_until_selected = 0.f;
                for (std::int32_t i = 0; i < options_size; i++) {
                    const xstr& element = options_->element(i);

                    member_states_[i].skipped = !last_search_.empty() &&
                        element.find_ignore_case_f(search_str) == xstr::npos;

                    if (!member_states_[i].skipped) {
                        if (i == *value_)
                            found_selected = true;
                        else if (!found_selected)
                            pos_until_selected += row_height_;
                    }
                }

                if (!found_selected)
                    pos_until_selected = 0.f;

                scrollbar_->set_scroll(pos_until_selected);
                scrollbar_->set_des_scroll(pos_until_selected);
                scrollbar_->set_scroll_speed(0.f);
            }
        }
    }
    else
        search_offset_ = 0.f;

    // items
    float highest = 0.f;
    bool no_results = true;
    for (std::size_t i = 0u; i < options_size; i++) {
        member_states_[i].hovered = util.lerp(
            member_states_[i].hovered,
            input.is_hovered(this, i)
        );
        member_states_[i].selected = util.lerp(
            member_states_[i].selected,
            *value_ == i
        );

        if (member_states_[i].skipped)
            continue;

        no_results = false;

        highest += row_height_;
    }

    if (no_results &&
        !text_field_->get_string().empty()) {
        render_no_results_ = true;
    }

    // scroll
    const float highest_pos = std::max(highest - row_height_, 0.f);
    const float scrollbar_width = spacing;

    scrollbar_->update(
        r2::rectf{
            list_pos_.x + list_pos_.w - scrollbar_width - border_size,
            list_pos_.y + spacing,
            scrollbar_width,
            list_pos_.h - spacing * 2.f
        },
        highest_pos
    );
}

void list::render(float alpha)
{
    if (was_occluded_) {
        return;
    }

    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const float spacing = style.spacing->get(instance()->scale());
    const float rounding = style.rounding->get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());
    const float border_size = style.border_size.get(instance()->scale());

    if (has_search_) {
        // background
        const r2::vec2 search_min = r2::vec2(list_pos_.x, list_pos_.y - search_offset_);
        const r2::vec2 search_max = search_min + r2::vec2(list_pos_.w, search_offset_ - border_size);

        renderer.add_rect_filled(
            search_min, search_max,
            util.disable_color(style.accent2(), animation_disabled_).alpha(alpha),
            rounding * 0.5f,
            r2::e_rounding_flags::rounding_top
        );

        // text field
        const r2::color text_color = util.disable_color(style.text().interp(style.text_accent(),
            0.4f + text_field_->animation_selected() * 0.6f), animation_disabled_
        );

        text_field_->render(
            alpha,
            text_color
        );

        // border
        renderer.add_rect_inner(
            search_min, search_max,
            util.disable_color(style.accent(), animation_disabled_).alpha(
                alpha * (0.5f + animation_hovered_search_ * 0.5f)),
            border_size,
            rounding * 0.5f,
            r2::e_rounding_flags::rounding_top
        );
    }

    // text
    const float text_spacing = std::round((last_pos_.h - text_size_small) * 0.5f);

    renderer.push_clip_rect(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(list_pos_.x - border_size, last_pos_.y + last_pos_.w),
        true
    );

    renderer.add_text(
        r2::vec2(last_pos_.x, last_pos_.y + text_spacing),
        util.disable_color(style.text(), animation_disabled_).alpha(alpha),
        name_
    );

    renderer.pop_clip_rect();

    // background
    const r2::vec2 list_min = r2::vec2(list_pos_.x, list_pos_.y);
    const r2::vec2 list_max = list_min + r2::vec2(list_pos_.w, list_pos_.h);
    const auto rounding_flag = has_search_ ?
        r2::e_rounding_flags::rounding_bottom : r2::e_rounding_flags::rounding_all;

    renderer.add_rect_filled(
        list_min, list_max,
        util.disable_color(style.accent2(), animation_disabled_).alpha(alpha),
        rounding * 0.5f,
        rounding_flag
    );

    // border
    renderer.add_rect_inner(
        list_min, list_max,
        util.disable_color(style.accent(), animation_disabled_).alpha(
            alpha * (0.5f + animation_hovered_ * 0.5f)),
        border_size,
        rounding * 0.5f,
        rounding_flag
    );
    
    // items
    const float clip_rect_rightx = list_max.x - scrollbar_->get_width() - border_size;
    renderer.push_clip_rect(
        list_min,
        r2::vec2(clip_rect_rightx, list_max.y),
        true
    );

    const float left_offset = std::round(spacing * 0.5f);
    const float hovered_offset = std::round(spacing * 0.5f);

    float pos_y = list_pos_.y + spacing_ - scrollbar_->get_scroll();
    float highest = 0.f;
    bool no_results = true;
    const auto options_size = options_->size();

    for (std::size_t i = 0u; i < options_size; i++) {
        if (member_states_[i].skipped)
            continue;

        const xstr& element = options_->element(i);

        if (pos_y + text_size_small >= list_min.y &&
            pos_y <= list_max.y) {
            renderer.add_text_faded(
                r2::vec2(list_pos_.x + left_offset +
                    member_states_[i].hovered * hovered_offset,
                    pos_y),
                style.text_accent().alpha(alpha * member_states_[i].selected * 0.5f),
                style.text_accent().transparent(),
                clip_rect_rightx - border_size, clip_rect_rightx,
                element, true
            );

            renderer.add_text(
                r2::vec2(list_pos_.x + left_offset +
                    member_states_[i].hovered * hovered_offset,
                    pos_y),
                style.text().interp(style.text_accent(), member_states_[i].selected).alpha(
                    alpha * (0.7f + member_states_[i].hovered * 0.3f) * 
                    (0.6f + member_states_[i].selected * 0.4f)),
                element
            );
        }

        no_results = false;

        pos_y += row_height_;
        highest += row_height_;
    }

    if (render_no_results_) {
        constexpr xstr kNoResults = "no results";
        if (!no_results_width_calculated_) {
            if (renderer.get_text_width_strict(kNoResults, no_results_width_))
                no_results_width_calculated_ = true;
            else
                no_results_width_ = renderer.get_text_width(kNoResults);
            no_results_width_ = std::ceil(no_results_width_);
        }

        const float text_spacing_x = std::round(
            ((list_max.x - list_min.x) - no_results_width_) * 0.5f
        );

        renderer.add_text(
            list_min + r2::vec2(text_spacing_x, spacing_),
            style.text(),
            kNoResults
        );
    }

    renderer.pop_clip_rect();

    scrollbar_->render(alpha);
}

input_response list::input(const input_base& input)
{
    if (!can_receive_input_simple(input))
        return input_response::empty();

    auto res = text_field_->input(input);
    if (res.is_handled())
        return res;

    res = scrollbar_->input(input);
    if (res.is_handled())
        return res;

    if (!can_receive_input_this(input))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if ((input.event().is_message(message_type::mouse_button_down) &&
          input.event().get_mouse_button() == mouse_button::left) ||
        input.event().is_message(message_type::mouse_move)) {
        auto rect = list_pos_;
        rect.w -= scrollbar_->get_width() +
            instance()->style().border_size.get(instance()->scale());
        if (util::is_in_rect(mouse_x, mouse_y, rect)) {
            std::int32_t selected = static_cast<std::int32_t>(
                (mouse_y - (list_pos_.y - scrollbar_->get_scroll())) / row_height_
            );
            if (selected >= 0 &&
                selected < static_cast<std::int32_t>(options_->size()) &&
                selected < input_count()) {
                // Find in current visible options
                std::int32_t visible = 0;
                for (std::int32_t i = 0; i < static_cast<std::int32_t>(member_states_.size()); i++) {
                    if (member_states_[i].skipped)
                        continue;

                    if (visible == selected) {
                        if (input.event().is_message(message_type::mouse_button_down))
                            *value_ = static_cast<std::size_t>(i);
                        else /* MouseMove */ {
                            input.set_hovered(this, i);

                            instance()->cursors().set_cursor(cursor::hand);
                        }

                        return input_response::handled();
                    }

                    visible++;
                }
            }
        }
    }

    // scroll
    if (input.event().is_message(message_type::scroll) &&
        util::is_in_rect(mouse_x, mouse_y, list_pos_)) {
        scrollbar_->on_scroll(input.event().get_scroll());
        return input_response::handled();
    }

    return input_response::empty();
}

void list::on_activate()
{
	widget::on_activate();

	member_states_.clear();

	scrollbar_->set_scroll(0.f);
	scrollbar_->set_des_scroll(0.f);
    scrollbar_->set_scroll_speed(0.f);

	if (has_search_) {
		last_search_.clear();
		text_field_->set_string(std::u32string());
	}
    text_field_->on_activate();
}

void list::on_scale_change()
{
	widget::on_scale_change();
	no_results_width_calculated_ = false;
    text_field_->on_scale_changed();
}

bool list::matches_search(const xstr& search) noexcept
{
	return name_.find_ignore_case_f(search) != xstr::npos;
}

void_end_