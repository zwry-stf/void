#include <void/contents/container/child_tab.h>
#include <void/void.h>
#include <void/contents/widgets/scrollbar.h>


void_begin_

child_tab::child_tab(void_* instance, input_owner* input_owner, std::int32_t num_input, std::int32_t input_id,
                     input_owner_overlay* overlay_owner, const xstr& name)
    : vobj(instance),
      input_receiver(input_owner, num_input),
      parent_overlay_owner_(overlay_owner),
      name_(name),
      input_id_(input_id)
{
    scrollbar_ = std::make_unique<scrollbar>(instance, input_owner);
}

child_tab::~child_tab() = default;

void child_tab::set_icon(int resource_id)
{
    icon_ = instance()->icons().get_or_create_handle(resource_id);
}

float child_tab::update(float x, float y, bool selected, const render_input& input)
{
    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();
    const auto& menu_pos = instance()->pos();

    if (!text_width_calculated_) {
        instance()->fonts().bind_font_large();
        if (renderer.get_text_width_strict(name_, text_width_))
            text_width_calculated_ = true;
        else
            text_width_ = renderer.get_text_width(name_);
        text_width_ = std::ceil(text_width_);
    }

    animation_selected_ = util.lerp(animation_selected_, selected, 3.5f);
    animation_hovered_ = util.lerp(animation_hovered_, input.is_hovered(this, input_id_));

    const float top_bar_height = style.top_bar_height.get(instance()->scale());
    const float text_size = style.text_size_large.get(instance()->scale());
    const bool has_icon = icon_ != icons::kInvalidHandle;
    const float icon_offset = has_icon ? text_size : 0.f;

    last_pos_.x = x;
    last_pos_.y = y + std::round((top_bar_height - text_size) * 0.5f);
    last_pos_.w = text_width_ + icon_offset;
    last_pos_.h = text_size;

    const float border_size = style.border_size.get(instance()->scale());
    const float spacing = style.spacing->get(instance()->scale());
    scrollbar_->update(
        r2::rectf{
            menu_pos.x + menu_pos.w - scrollbar_width_ - border_size,
            menu_pos.y + top_bar_height + spacing * 2.f,
            scrollbar_width_,
            menu_pos.h - top_bar_height - spacing * 4.f
        },
        highest_pos_
    );

    return last_pos_.w;
}

void child_tab::render(float alpha, bool)
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    instance()->fonts().bind_font_large();

    const float border_size = style.border_size.get(instance()->scale());
    const bool has_icon = icon_ != icons::kInvalidHandle;

    if (has_icon) {
        const float icon_offset = border_size * 2.f;
        const float icon_size = last_pos_.h - icon_offset * 2.f;
        const auto* icon = instance()->icons().get_or_create(icon_, icon_size);
        renderer.add_image(
            icon->tex,
            r2::vec2(last_pos_.x + icon_offset, last_pos_.y + icon_offset),
            r2::vec2(last_pos_.x + icon_offset + icon_size, last_pos_.y + icon_offset + icon_size),
            style.icon().interp(style.text_accent(), animation_selected_).alpha(alpha),
            icon->uv_min, icon->uv_max
        );
    }

    const float icon_offset = has_icon ?
        style.text_size_large.get(instance()->scale()) : 0.f;

    const float spacing = style.spacing->get(instance()->scale());
    const float fade_width = spacing * 2.f;
    const float fade_end = instance()->search_pos().x - spacing;
    const float fade_start = fade_end - fade_width;

    // Text
    const auto text_color = style.text().interp(style.text_accent(), animation_selected_);
    renderer.add_text_faded(
        r2::vec2(last_pos_.x + icon_offset, last_pos_.y),
        text_color.alpha(alpha * (0.4f + animation_selected_ * 0.6f)),
        text_color.transparent(),
        fade_start, fade_end, name_, true
    );

    renderer.add_text_faded(
        r2::vec2(last_pos_.x + icon_offset, last_pos_.y),
        text_color.alpha(alpha), text_color.transparent(),
        fade_start, fade_end, name_
    );

    instance()->fonts().bind_font_small();

    // Underline
    const float text_size = style.text_size_large.get(instance()->scale());

    renderer.add_rect_filled_faded(
        r2::vec2(last_pos_.x, last_pos_.y + text_size - border_size),
        r2::vec2(last_pos_.x + last_pos_.w, last_pos_.y + text_size),
        style.text().alpha(alpha * 0.3f),
        style.text().transparent(), 
        fade_start, fade_end
    );

    const float _anim = (1.f - std::sqrt(animation_selected_)) * 0.5f;
    const auto animated_color = style.text().interp(style.text_accent(), animation_selected_).alpha(0.8f - _anim * 1.6f);

    renderer.add_rect_filled_faded(
        r2::vec2(last_pos_.x + last_pos_.w * _anim, 
            last_pos_.y + text_size - border_size),
        r2::vec2(last_pos_.x + last_pos_.w - last_pos_.w * _anim,
            last_pos_.y + text_size),
        animated_color.alpha(alpha),
        animated_color.transparent(),
        fade_start, fade_end
    );
}

input_response child_tab::input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id)
{
    if (!input.nothing_selected() &&
        !input.is_selected(this, input_id_))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    // Tab Input
    if (input.event().is_message(message_type::mouse_button_down) &&
        input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            if (selected_tab != tab_id)
                on_activate(false);

            selected_tab = tab_id;

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_move)) {
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            input.set_hovered(this, input_id_);
            instance()->cursors().set_cursor(cursor::hand);
            return input_response::handled();
        }
    }

    return input_response::empty();
}

void child_tab::on_activate(bool parent_change, bool first)
{
    (void)parent_change;
    auto& style = instance()->style();

    if (!first) {
        const float spacing = style.spacing->get(instance()->scale());

        scrollbar_->set_scroll(0.f);
        scrollbar_->set_des_scroll(-spacing * 2.f);
        scrollbar_->set_scroll_speed(spacing * 4.f);
    }
}

void child_tab::on_scale_change()
{
    text_width_calculated_ = false;
}

void child_tab::render_scrollbar(float alpha)
{
    scrollbar_->render(alpha);
}

input_response child_tab::input_scrollbar(const input_base& input)
{
    auto ret = scrollbar_->input(
        input
    );

    if (!input.nothing_selected() &&
        !input.is_selected(this, input_id_))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (ret.is_handled())
        return ret;

    // scroll
    if (input.event().is_message(message_type::scroll)) {
        auto& style = instance()->style();

        r2::rectf rect = instance()->pos();
        rect.x += style.sidebar_width.get(instance()->scale());
        rect.y += style.top_bar_height.get(instance()->scale());

        if (util::is_in_rect(mouse_x, mouse_y, rect)) {
            scrollbar_->on_scroll(input.event().get_scroll());
            return input_response::handled();
        }
    }

    return input_response::empty();
}

void_end_