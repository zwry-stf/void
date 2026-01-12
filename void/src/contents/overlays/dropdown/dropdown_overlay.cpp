#include "dropdown.h"
#include <void/void.h>


void_begin_

dropdown_overlay::dropdown_overlay(void_* instance, input_owner* input_owner,
                                   input_owner_overlay* overlay_owner, std::unique_ptr<list_options>&& options, std::size_t* value)
	: overlay(instance, overlay_owner, false, false),
	  input_receiver(input_owner, static_cast<std::int32_t>(options->size())),
	  options_(std::move(options)),
	  value_(value)
{
}

void dropdown_overlay::update(const overlay_render_input& input)
{
	overlay::update(input);

    const auto& menu_pos = instance()->pos();
    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();
    const auto display_size = renderer.get_render_size();

    // Values
    last_pos_.x = menu_pos.x + set_pos_.x;
    last_pos_.y = menu_pos.y + set_pos_.y;

    const float animation = instance()->alpha() * alpha_;
    if (animation < util::g_min_alpha)
        return;

    const auto options_size = options_->size();

    if (options_size > member_animations_.size()) {
        member_animations_.resize(options_size, member_animation());
    }
    if (options_size > static_cast<std::size_t>(input_count())) {
        resize_input(static_cast<std::int32_t>(options_size));
    }

    const float spacing = style.spacing->get(instance()->scale());
    const float text_size_small = style.text_size_small.get(instance()->scale());

    spacing_ = std::round(spacing * 0.4f);
    row_height_ = text_size_small + spacing_;

    const float full_height = std::max(static_cast<float>(options_size), 1.f) *
        row_height_ + spacing_;

    last_pos_.w = parent_size_.x * (0.7f + std::sqrt(animation_) * 0.3f);
    last_pos_.h = full_height * alpha_;

    // clamp position in window
    if (pos_changed_) {
        pos_changed_ = false;

        if (last_pos_.y + full_height > display_size.y) {
            last_pos_.y -= full_height + parent_size_.y;
            set_pos_.y -= full_height + parent_size_.y;
        }
    }

    // items
    auto render_input = input_owner_->input_get_render_input();
    for (std::size_t i = 0u; i < options_size; i++) {
        member_animations_[i].hovered = util.lerp(member_animations_[i].hovered, render_input.is_hovered(this, i));
        member_animations_[i].selected = util.lerp(member_animations_[i].selected, *value_ == i);
    }
}

void dropdown_overlay::render()
{
    const float animation = instance()->alpha() * alpha_;
    if (animation < util::g_min_alpha)
        return;

    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    const float spacing = style.spacing->get(instance()->scale());

    instance()->background().add_immediate_overlay(
        last_pos_, alpha_
    );

    // items
    const float clip_rect_rightx = last_pos_.x + last_pos_.w;
    renderer.push_clip_rect(
        r2::vec2(last_pos_.x, last_pos_.y),
        r2::vec2(clip_rect_rightx, last_pos_.y + last_pos_.h),
        false
    );

    const float left_offset = std::round(spacing * 0.5f);
    const float hovered_offset = std::round(spacing * 0.5f);

    const std::size_t options_size = options_->size();

    const float fade_width = spacing;

    float pos_y = last_pos_.y + spacing_;
    for (std::size_t i = 0u; i < options_size; i++) {
        const xstr& element = options_->element(i);

        renderer.add_text_faded(
            r2::vec2(last_pos_.x + left_offset +
                member_animations_[i].hovered * hovered_offset,
                pos_y),
            style.text_accent().alpha(
                animation * member_animations_[i].selected * 0.5f),
            style.text_accent().transparent(),
            clip_rect_rightx - fade_width, clip_rect_rightx,
            element, 
            true /* blurred */
        );

        const auto text_color = style.text().interp(
            style.text_accent(), member_animations_[i].selected).alpha(
            animation * (0.7f + member_animations_[i].hovered * 0.3f) *
            (0.6f + member_animations_[i].selected * 0.4f)
        );

        renderer.add_text_faded(
            r2::vec2(last_pos_.x + left_offset +
                member_animations_[i].hovered * hovered_offset,
                pos_y),
            text_color,
            text_color.transparent(),
            clip_rect_rightx - fade_width, clip_rect_rightx,
            element
        );

        pos_y += row_height_;
    }
    if (options_size == 0u) {
        renderer.add_text(
            r2::vec2(last_pos_.x + left_offset,
                pos_y),
            style.text().alpha(animation * 0.7f * 0.6f),
            list_options::kNone
        );
    }

    renderer.pop_clip_rect();
}

input_response dropdown_overlay::input(const overlay_input& input)
{
    if (!input.is_opened(this))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if ((input.event().is_message(message_type::mouse_button_down) &&
         input.event().get_mouse_button() == mouse_button::left) ||
            input.event().is_message(message_type::mouse_move)) {
        if (!util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            if (input.event().is_message(message_type::mouse_button_down)) {
                input.clear_opened();
                return input_response::handled();
            }
        }

        else {
            std::int32_t selected = static_cast<std::int32_t>(
                (mouse_y - last_pos_.y) / row_height_
            );
            if (selected >= 0 &&
                selected < static_cast<std::int32_t>(options_->size()) &&
                selected < input_count()) {
                if (input.event().is_message(message_type::mouse_button_down))
                    *value_ = static_cast<std::size_t>(selected);
                else /* MouseMove */ {
                    input_owner_->input_get_input(input.event()).set_hovered(this, selected);

                    instance()->cursors().set_cursor(cursor::hand);
                }

                return input_response::handled();
            }
        }
    }

    return should_block_input(input);
}

void_end_