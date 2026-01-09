#include <void/contents/overlays/custom_overlay.h>
#include <void/void.h>
#include <util/resizing_type.h>


void_begin_

enum class overlay_selected_ids : std::int32_t {
    selected_move,
    selected_size,
    selected_max,
};

custom_overlay::custom_overlay(void_* instance, input_owner* input_owner, 
                               const custom_overlay_cfg& cfg, const custom_overlay_data& data)
    : vobj(instance),
      input_receiver(input_owner, 
          std::to_underlying(overlay_selected_ids::selected_max)),
      cfg_(cfg),
      data_(data)
{
}

void custom_overlay::update()
{
    r2::vec2 pos = get_pos();
    r2::vec2 size = get_size();

    const auto display_size = instance()->renderer().get_render_size();

    assert(cfg_.min_width.raw() <= cfg_.max_width.raw() && 
           cfg_.min_height.raw() <= cfg_.max_height.raw());

    // clamp size
    const float min_width = cfg_.min_width.get(instance()->scale());
    const float max_width = cfg_.max_width.get(instance()->scale());
    const float min_height = cfg_.min_height.get(instance()->scale());
    const float max_height = cfg_.max_height.get(instance()->scale());

    size.x = std::clamp(size.x, min_width, max_width);
    size.y = std::clamp(size.y, min_height, max_height);

    // clamp overlay to screen
    pos.x = std::clamp(pos.x, 0.f, display_size.x - size.x);
    pos.y = std::clamp(pos.y, 0.f, display_size.y - size.y);

    last_pos_.x = pos.x;
    last_pos_.y = pos.y;
    last_pos_.w = size.x;
    last_pos_.h = size.y;

    set_pos_scaled(pos);
    set_size_scaled(size);

    if (update_callback_ != nullptr)
        update_callback_(instance(), *this);
}

void custom_overlay::render()
{
    if (render_callback_ != nullptr)
        render_callback_(instance(), *this);
}

input_response custom_overlay::input(const input_base& input)
{
    if (!has_input_ ||
        !instance()->is_open())
        return input_response::empty();

    if (!input.nothing_selected() &&
        !input.is_selected(this))
        return input_response::empty();

    auto res = move_window(input);
    if (res.is_handled())
        return res;

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if ((input.event().is_message(message_type::mouse_move) ||
        input.event().is_message(message_type::mouse_button_down)) &&
        util::is_in_rect(mouse_x, mouse_y, last_pos_))
        return input_response::handled();

    return input_response::empty();
}

r2::vec2 custom_overlay::get_pos() const noexcept
{
    const auto display_size = instance()->renderer().get_render_size();
    return r2::vec2(
        std::round(data_.pos.x * display_size.x),
        std::round(data_.pos.y * display_size.y)
    );
}

r2::vec2 custom_overlay::get_size() const noexcept
{
    return r2::vec2(
        std::round(data_.pos.w * instance()->scale()),
        std::round(data_.pos.h * instance()->scale())
    );
}

void custom_overlay::set_pos_scaled(const r2::vec2& v)
{
    const auto display_size = instance()->renderer().get_render_size();
    set_pos(v / display_size);
}

void custom_overlay::set_size_scaled(const r2::vec2& v)
{
    set_size(v / r2::vec2(instance()->scale()));
}

e_resizing_type custom_overlay::get_resizing_type(float mouse_x, float mouse_y)
{
    const sfloat kResizeOffset = sfloat(6.f);
    const float resize_offset = kResizeOffset.get(instance()->scale());
    const float resize_offset2 = resize_offset * 2.f;

    e_resizing_type ret = e_resizing_type::none;

    if (util::is_in_rect(mouse_x, mouse_y,
        last_pos_.x - resize_offset, last_pos_.y - resize_offset, last_pos_.w + resize_offset2, resize_offset2)) // top
        ret |= e_resizing_type::top;
    else if (util::is_in_rect(mouse_x, mouse_y,
        last_pos_.x - resize_offset, last_pos_.y + last_pos_.h - resize_offset, last_pos_.w + resize_offset2, resize_offset2)) // bottom
        ret |= e_resizing_type::bottom;

    if (util::is_in_rect(mouse_x, mouse_y,
        last_pos_.x - resize_offset, last_pos_.y - resize_offset, resize_offset2, last_pos_.h + resize_offset2)) // Left
        ret |= e_resizing_type::left;
    else if (util::is_in_rect(mouse_x, mouse_y,
        last_pos_.x + last_pos_.w - resize_offset, last_pos_.y - resize_offset, resize_offset2, last_pos_.h + resize_offset2)) // Right
        ret |= e_resizing_type::right;

    return ret;
}

input_response custom_overlay::move_window(const input_base& input)
{
    if (!input.nothing_selected() &&
        !input.is_range_selected(this,
            overlay_selected_ids::selected_move,
            overlay_selected_ids::selected_size))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);
    
    if (input.event().is_message(message_type::mouse_move)) {
        // Move
        if (input.is_selected(this, overlay_selected_ids::selected_move)) {
            last_pos_.x = mouse_x - move_pos_.x;
            last_pos_.y = mouse_y - move_pos_.y;

            set_pos_scaled(
                r2::vec2{
                    last_pos_.x,
                    last_pos_.y
                }
            );

            return input_response::handled();
        }

        auto resizing_type = e_resizing_type::invalid;
        // Resize
        if (input.is_selected(this, overlay_selected_ids::selected_size)) {
            r2::vec4 new_rect = move_pos_.to_vec4();

            const float min_width = cfg_.min_width.get(instance()->scale());
            const float max_width = cfg_.max_width.get(instance()->scale());
            const float min_height = cfg_.min_height.get(instance()->scale());
            const float max_height = cfg_.max_height.get(instance()->scale());
            const auto display_size = instance()->renderer().get_render_size();

            if (resizing_type_ & e_resizing_type::top) {
                new_rect.y = mouse_y;

                if (new_rect.w - new_rect.y > max_height)
                    new_rect.y = new_rect.w - max_height;
                if (new_rect.w - new_rect.y < min_height)
                    new_rect.y = new_rect.w - min_height;
                if (new_rect.y < 0.f)
                    new_rect.y = 0.f;
            }
            else if (resizing_type_ & e_resizing_type::bottom) {
                new_rect.w = mouse_y;

                if (new_rect.w - new_rect.y > max_height)
                    new_rect.w = new_rect.y + max_height;
                if (new_rect.w - new_rect.y < min_height)
                    new_rect.w = new_rect.y + min_height;
                if (new_rect.w > display_size.y)
                    new_rect.w = display_size.y;
            }

            if (resizing_type_ & e_resizing_type::left) {
                new_rect.x = mouse_x;

                if (new_rect.z - new_rect.x > max_width)
                    new_rect.x = new_rect.z - max_width;
                if (new_rect.z - new_rect.x < min_width)
                    new_rect.x = new_rect.z - min_width;
                if (new_rect.x < 0.f)
                    new_rect.x = 0.f;
            }
            else if (resizing_type_ & e_resizing_type::right) {
                new_rect.z = mouse_x;

                if (new_rect.z - new_rect.x > max_width)
                    new_rect.z = new_rect.x + max_width;
                if (new_rect.z - new_rect.x < min_width)
                    new_rect.z = new_rect.x + min_width;
                if (new_rect.z > display_size.x)
                    new_rect.z = display_size.x;
            }

            last_pos_ = r2::rectf{
                new_rect.x,
                new_rect.y,
                new_rect.z - new_rect.x,
                new_rect.w - new_rect.y
            };

            set_pos_scaled(
                r2::vec2{
                    last_pos_.x,
                    last_pos_.y 
                }
            );
            set_size_scaled(
                r2::vec2{
                    last_pos_.w,
                    last_pos_.h
                }
            );

            resizing_type = resizing_type_;
        }
        else {
            resizing_type = get_resizing_type(mouse_x, mouse_y);
        }

        // Resize Cursor
        if (resizing_type == e_resizing_type::none ||
            resizing_type == e_resizing_type::invalid)
            return input_response::empty();

        switch (resizing_type) {
        case e_resizing_type::top:
        case e_resizing_type::bottom:
            instance()->cursors().set_cursor(cursor::size_ns);
            break;
        case e_resizing_type::left:
        case e_resizing_type::right:
            instance()->cursors().set_cursor(cursor::size_ew);
            break;
        case e_resizing_type::top_left:
        case e_resizing_type::bottom_right:
            instance()->cursors().set_cursor(cursor::size_nwse);
            break;
        case e_resizing_type::top_right:
        case e_resizing_type::bottom_left:
            instance()->cursors().set_cursor(cursor::size_nesw);
            break;
        default:
            assert(false);
        }

        return input_response::handled();
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (input.is_range_selected(this,
                overlay_selected_ids::selected_move,
                overlay_selected_ids::selected_size)) {
            input.clear_selected();

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        
        e_resizing_type resizing_type = get_resizing_type(mouse_x, mouse_y);
        if (resizing_type != e_resizing_type::none) {
            resizing_type_ = resizing_type;

            move_pos_ = last_pos_;

            input.set_selected(this, overlay_selected_ids::selected_size);
            return input_response::handled();
        }

        // Moving
        if (util::is_in_rect(mouse_x, mouse_y, last_pos_)) {
            move_pos_.x = mouse_x - last_pos_.x;
            move_pos_.y = mouse_y - last_pos_.y;

            input.set_selected(this, overlay_selected_ids::selected_move);
            return input_response::handled();
        }
    }

    return input_response::empty();
}

void_end_