#include <void/void.h>
#include <background/background.h>
#include <background/background_overlay.h>
#include <void/util/error.h>
#include <contents/sidebar.h>
#include <util/resizing_type.h>
#include <config/config.h>
#include <theme/theme.h>
#include <void/builder/builder.h>
#include <contents/overlays/keybind_list/keybind_list.h>


#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif


void_begin_

enum class void_selected_ids : std::int32_t {
    selected_move,
    selected_size,
    selected_max,
};

void_::void_()
    : input_receiver(this, 
        std::to_underlying(void_selected_ids::selected_max))
{
    background_ = std::make_unique<_background>(this);
    background_overlay_ = std::make_unique<_background_overlay>(this);
    sidebar_ = std::make_unique<sidebar>(this);
    config_ = std::make_unique<_config>(this);
    theme_ = std::make_unique<_theme>(this);
    keybind_list_ = std::make_unique<keybind_list>(this);

    search_text_field_ = std::make_unique<textfield>(
        this, this,
        textfield_type::text, /* type */
        textfield_flags::mouse_in_rect | textfield_flags::stop_on_return |
            textfield_flags::movable_caret | textfield_flags::faded_text, /* flags */
        xstr("search..."), /* default text */
        xstr(), /* post text */
        30u /* max length */
    );
}

void_::~void_() = default;

void void_::init(const r2::platform_init_data& pinit, const r2::backend_init_data& binit)
{
    renderer_.init(pinit, binit);

#if defined(R2_BACKEND_OPENGL)
    renderer_.backup_render_state();
#endif // R2_BACKEND_OPENGL

    // scale
    scale_ = calculate_scale();

    // fonts
    fonts().create();

    // build fonts
    fonts().build();

    renderer_.create_font_texture();

    icons().init();

    // background
    render_target().init_targets();

    background_->init();
    background_overlay_->init(background_.get());

    // render_target
    render_target().init();

    // config
    config_->init();
    theme_->init();

#if defined(R2_BACKEND_OPENGL)
    renderer_.restore_render_state();
#endif // R2_BACKEND_OPENGL

    // menu pos
    pos_.w = std::round(
        options().get<options::option_DefaultSize>().x * scale_
    );
    pos_.h = std::round(
        options().get<options::option_DefaultSize>().y * scale_
    );

    const r2::vec2 render_size = renderer_.get_render_size();
    pos_.x = (render_size.x - pos_.w) * 0.5f;
    pos_.y = (render_size.y - pos_.h) * 0.5f;

    open_ = !options().get<options::option_StartMinimized>();
    animation_ = is_open() ? 1.f : 0.f;
    alpha_ = animation_;

    initialized_.store(true, std::memory_order_release);

    sidebar_->on_activate();
}

void void_::destroy()
{
    destroy_render();
    icons().destroy();

    config_->destroy();
    theme_->destroy();
}

void void_::init_render(const r2::platform_init_data& pinit, const r2::backend_init_data& binit)
{
    // renderer
    renderer_.init(pinit, binit);

#if defined(R2_BACKEND_OPENGL)
    renderer_.backup_render_state();
#endif // R2_BACKEND_OPENGL

    renderer_.create_font_texture();

    icons().init();

    // background
    render_target().init_targets();

    background_->init();
    background_overlay_->init(background_.get());

    // render_target
    render_target().init();

#if defined(R2_BACKEND_OPENGL)
    renderer_.restore_render_state();
#endif

    initialized_.store(true, std::memory_order_release);
}

void void_::destroy_render()
{
    initialized_.store(false, std::memory_order_release);

    render_target().destroy();

    background_->destroy();
    background_overlay_->destroy();

    renderer_.destroy_render();
}

void void_::pre_resize()
{
    background_->pre_resize();
    render_target().destroy_targets();

    renderer_.pre_resize();
}

void void_::post_resize()
{
    renderer_.post_resize();

    render_target().init_targets();
    background_->post_resize();
}

void void_::render()
{
#if defined(_DEBUG)
    renderer_.assert_render_thread();
#endif

    input().input_on_frame();

    /// start frame
    if (options().get<options::option_UpdateFrameTime>()) {
        update_frame_time();
    }

    renderer_.reset_render_data();
    if (options().get<options::option_FullFrameUpdate>()) {
        renderer_.backup_render_state();
        renderer_.setup_render_state();
    }

    if (options().get<options::option_MenuMSAA>()) {
        renderer_.set_multisampled(true);
    }

    background_overlay_->reset_data();

    /// animation
    animation_ = util().lerp2(animation_, is_open());
    alpha_ = animation_;

    callbacks().invoke<callbacks::callback_OnPreRender>();

    /// render menu
    const bool menu_rendered = alpha_ >= util::g_min_alpha;
    if (menu_rendered)
        render_menu();

    render_target().bind_main();

    if (menu_rendered)
        render_target().draw_menu();

    if (options().get<options::option_MenuMSAA>()) {
        renderer_.set_multisampled(false);
    }

    if (menu_rendered)
        render_overlays();
    background_overlay_->reset_data();

    /// render custom overlays
    background_overlay_->render_custom_overlays(background_.get());
    background_->restore_render_states();

    /// misc rendering
    watermark().render();
    notifications().render();

    callbacks().invoke<callbacks::callback_OnPostRender>();

    renderer_.render();
    renderer_.reset_render_data();

    /// end frame
    renderer_.restore_render_state();

    cursors().update();
}

input_response void_::on_input(const message_event& event)
{
    auto input = input_get_input(event);

    if (open_) {
        /// custom overlay input
        auto res = background_overlay_->input(input);
        if (res.is_handled())
            return res;

        /// overlay input
        {
            auto overlay_input = input_get_overlay_input(event);

            // only input opened overlay
            if (!overlay_input.nothing_opened()) {
                for (auto& o : overlays_) {
                    if (overlay_input.is_opened(o.get())) {
                        res = o->input(overlay_input);
                        if (res.is_handled())
                            return res;
                    }
                }
            }
        }

        /// sidebar
        res = sidebar_->input(input);
        if (res.is_handled())
            return res;

        /// search
        res = input_search(input);
        if (res.is_handled())
            return res;

        /// account
        res = account().input(input);
        if (res.is_handled())
            return res;

        /// move/resize
        res = input_move(input);
        if (res.is_handled())
            return res;

        /// key
        if (event.is_message(message_type::key_down) &&
            event.get_key() == key::escape) {
            toggle_menu(false);
            return input_response::handled();
        }
    }

    if (event.is_message(message_type::key_down) &&
        event.get_key() == options().get<options::option_MenuKey>()) {
        toggle_menu(!is_open());
        return input_response::handled();
    }

    return input_response::empty();
}

void void_::toggle_menu(bool open)
{
#if defined(_DEBUG)
    renderer_.assert_render_thread();
#endif
    open_.store(open, std::memory_order_release);
    if (!open)
        input_reset_selected_state();
    input().clear_queue();

    callbacks().invoke<callbacks::callback_OnToggleMenu>(open);
}

void void_::set_scale(float scale)
{
    update_scale(scale);
}

void void_::set_scale_auto()
{
    update_scale(calculate_scale());
}

menu_builder void_::get_builder() noexcept
{
    return menu_builder(this);
}

float void_::calculate_scale() noexcept
{
    if (!options().get<options::option_DynamicSizing>())
        return 1.f;

#if defined(R2_PLATFORM_WINDOWS)
    constexpr float kDesignHeight = 1080.f;
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    float screen_scale = float(screen_height == 0 ? int(kDesignHeight) : screen_height) / kDesignHeight;

    return screen_scale;
#endif
}

void void_::render_menu()
{
    theme_->animate();

    /// clamp position
    {
        const sfloat kMinAreaInScreen = sfloat(100.f);
        const float min_area_in_screen = kMinAreaInScreen.get(scale_);

        const r2::vec2 display_size = renderer_.get_render_size();
        pos_.x = std::roundf(std::clamp(pos_.x,
            (std::min)(-pos_.w + min_area_in_screen, 0.f),
            (std::max)(display_size.x - min_area_in_screen, 0.f)));
        pos_.y = std::roundf(std::clamp(pos_.y,
            (std::min)(-pos_.h + min_area_in_screen, 0.f),
            (std::max)(display_size.y - min_area_in_screen, 0.f)));
        pos_.w = std::round((std::max)(pos_.w, style().min_x.get(scale_)));
        pos_.h = std::round((std::max)(pos_.h, style().min_y.get(scale_)));
    }

    /// clear menu
    render_target().clear_menu();

    /// background
    background_->render(); // calls render_target.bind_menu()
    background_->restore_render_states();

    /// sidebar
    sidebar_->render(input_get_render_input());

    /// search
    render_search();

    /// account
    account().render(input_get_render_input());

    /// overlay dim
    float highest_animation_value = 0.f;
    auto overlay_input = input_get_overlay_render_input();
    for (auto& overlay : overlays_) {
        overlay->update(overlay_input);

        if (overlay->alpha() > highest_animation_value)
            highest_animation_value = overlay->alpha();
    }

    renderer_.add_rect_filled(
        r2::vec2(pos_.x, pos_.y),
        r2::vec2(pos_.x + pos_.w, pos_.y + pos_.h),
        r2::color::black().alpha(highest_animation_value * 0.2f),
        style().rounding->get(scale_)
    );

    /// render
    renderer_.render();
    renderer_.reset_render_data();
}

void void_::render_overlays()
{
    callbacks().invoke<callbacks::callback_OnPreRenderOverlay>();

    /// render overlays
    for (auto& overlay : overlays_) {
        overlay->render();
    }

    background_overlay_->render(background_.get());
    background_->restore_render_states();

    renderer_.render();
    renderer_.reset_render_data();

    background_overlay_->reset_data();

    /// render child overlays
    for (auto& overlay : overlays_) {
        if (overlay->has_overlays())
            overlay->render_overlays();
    }

    background_overlay_->render(background_.get());
    background_->restore_render_states();

    callbacks().invoke<callbacks::callback_OnPostRenderOverlay>();

    renderer_.render();
    renderer_.reset_render_data();
}

void void_::update_frame_time()
{
    time_point now = std::chrono::steady_clock::now();

    if (frame_start_ != time_point()) {
        delta_time_ = std::chrono::duration<float>(
            now - frame_start_).count();
    }

    frame_start_ = now;
}

void void_::reset_hovered_state()
{
    input_reset_hovered_state();
}

void void_::render_search()
{
    auto input = input_get_render_input();

    auto& renderer = this->renderer();
    auto& util     = this->util();
    auto& style    = this->style();

    // Animations
    has_search_ = true;
    search_animation_         = util.lerp(search_animation_, has_search_);
    search_opened_animation_  = util.lerp(search_opened_animation_, search_opened_);
    search_hovered_animation_ = util.lerp(search_hovered_animation_, input.is_hovered(search_text_field_.get()));

    // Search icon
    const float spacing = style.spacing->get(scale_);

    const float offset = std::round(spacing * 0.25f);
    const float full_height = style.text_size_small.get(scale_) + offset * 2.f;
    const float full_size = full_height * 4.f;

    const r2::vec2 search_min = r2::vec2(
        pos_.x + pos_.w - spacing * 2.f - full_size * search_opened_animation_ - full_height,
        pos_.y + (style.top_bar_height.get(scale_) - full_height) * 0.5f
    );

    const r2::vec2 search_max = search_min + r2::vec2(full_height, full_height);

    search_pos_ = r2::vec4(
        search_min.x, search_min.y,
        search_max.x, search_max.y
    );

    const float animation  = (0.7f + search_opened_animation_ * 0.3f) * search_animation_;
    const float animation2 = search_opened_animation_ * search_animation_;

    if (animation < util::g_min_alpha)
        return;

    if (search_icon_ == icons::kInvalidHandle) {
        search_icon_ = icons().get_or_create_handle(void_resources::search_png);
    }
    
    const auto* icon = icons().get_or_create(search_icon_, full_height);

    renderer.add_image(
        icon->tex,
        search_min, search_max,
        style.icon().alpha(animation),
        icon->uv_min, icon->uv_max
    );

    // Text field
    if (search_opened_animation_ < 0.001f)
        return;

    const float rounding = style.rounding->get(scale_);

    const float rect_left = search_max.x;
    const float rect_right = pos_.x + pos_.w - spacing * 2.f;

    if (rect_right > rect_left) {
        renderer.add_rect_filled(
            r2::vec2(rect_left, search_min.y),
            r2::vec2(rect_right, search_max.y),
            style.group_background().alpha(animation2),
            rounding * 0.5f
        );

        renderer.add_rect(
            r2::vec2(rect_left, search_min.y),
            r2::vec2(rect_right, search_max.y),
            style.accent2().interp(style.accent(), (std::max)(search_hovered_animation_,
                search_text_field_->animation_selected())).alpha(animation2),
            style.border_size.get(scale_),
            rounding * 0.5f
        );
    }

    if (full_size * search_opened_animation_ - offset * 4.f > 0.2f) {
        renderer.push_clip_rect(
            r2::vec2(search_max.x, search_min.y),
            r2::vec2(pos_.x + pos_.w - spacing * 2.f, search_max.y)
        );

        const r2::rectf pos{
            search_max.x + offset * 2.f,
            search_min.y + offset,
            full_size * search_opened_animation_ - offset * 4.f,
            style.text_size_small.get(scale_)
        };

        search_text_field_->update(
            pos, 
            input, 
            false /* occluded */
        );

        search_text_field_->render(animation2, style.text());

        renderer.pop_clip_rect();
    }
}

e_resizing_type void_::get_resizing_type(float mouse_x, float mouse_y) const noexcept
{
    const sfloat kResizeOffset = sfloat(6.f);
    const float resize_offset = kResizeOffset.get(scale_);
    const float resize_offset2 = resize_offset * 2.f;

    e_resizing_type ret = e_resizing_type::none;

    if (util::is_in_rect(mouse_x, mouse_y,
        pos_.x - resize_offset, pos_.y - resize_offset, pos_.w + resize_offset2, resize_offset2)) // top
        ret |= e_resizing_type::top;
    else if (util::is_in_rect(mouse_x, mouse_y,
        pos_.x - resize_offset, pos_.y + pos_.h - resize_offset, pos_.w + resize_offset2, resize_offset2)) // bottom
        ret |= e_resizing_type::bottom;

    if (util::is_in_rect(mouse_x, mouse_y,
        pos_.x - resize_offset, pos_.y - resize_offset, resize_offset2, pos_.h + resize_offset2)) // Left
        ret |= e_resizing_type::left;
    else if (util::is_in_rect(mouse_x, mouse_y,
        pos_.x + pos_.w - resize_offset, pos_.y - resize_offset, resize_offset2, pos_.h + resize_offset2)) // Right
        ret |= e_resizing_type::right;

    return ret;
}

input_response void_::input_move(const input_base& input)
{
    if (!input.nothing_selected() &&
        !input.is_range_selected(this,
            void_selected_ids::selected_move,
            void_selected_ids::selected_size))
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);
    
    if (input.event().is_message(message_type::mouse_move)) {
        // Move
        if (input.is_selected(this, void_selected_ids::selected_move)) {
            pos_.x = mouse_x - move_pos_.x;
            pos_.y = mouse_y - move_pos_.y;

            return input_response::handled();
        }

        auto resizing_type = e_resizing_type::invalid;
        // Resize
        if (input.is_selected(this, void_selected_ids::selected_size)) {
            r2::vec4 new_rect = move_pos_.to_vec4();

            const float min_width = style().min_x.get(scale_);
            const float min_height = style().min_y.get(scale_);

            if (resizing_type_ & e_resizing_type::top) {
                new_rect.y = mouse_y;

                if (new_rect.w - new_rect.y < min_height)
                    new_rect.y = new_rect.w - min_height;
            }
            else if (resizing_type_ & e_resizing_type::bottom) {
                new_rect.w = mouse_y;

                if (new_rect.w - new_rect.y < min_height)
                    new_rect.w = new_rect.y + min_height;
            }

            if (resizing_type_ & e_resizing_type::left) {
                new_rect.x = mouse_x;

                if (new_rect.z - new_rect.x < min_width)
                    new_rect.x = new_rect.z - min_width;
            }
            else if (resizing_type_ & e_resizing_type::right) {
                new_rect.z = mouse_x;

                if (new_rect.z - new_rect.x < min_width)
                    new_rect.z = new_rect.x + min_width;
            }

            pos_ = r2::rectf{
                new_rect.x,
                new_rect.y,
                new_rect.z - new_rect.x,
                new_rect.w - new_rect.y 
            };

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
            cursors().set_cursor(cursor::size_ns);
            break;
        case e_resizing_type::left:
        case e_resizing_type::right:
            cursors().set_cursor(cursor::size_ew);
            break;
        case e_resizing_type::top_left:
        case e_resizing_type::bottom_right:
            cursors().set_cursor(cursor::size_nwse);
            break;
        case e_resizing_type::top_right:
        case e_resizing_type::bottom_left:
            cursors().set_cursor(cursor::size_nesw);
            break;
        default:
            assert(false);
        }

        return input_response::handled();
    }

    else if (input.event().is_message(message_type::mouse_button_up) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (input.is_range_selected(this,
                void_selected_ids::selected_move,
                void_selected_ids::selected_size)) {
            input.clear_selected();

            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        
        e_resizing_type resizing_type = get_resizing_type(mouse_x, mouse_y);
        if (resizing_type != e_resizing_type::none) {
            resizing_type_ = resizing_type;

            move_pos_ = pos_;

            input.set_selected(this, void_selected_ids::selected_size);
            return input_response::handled();
        }

        // Moving
        if (util::is_in_rect(mouse_x, mouse_y, pos_)) {
            move_pos_.x = mouse_x - pos_.x;
            move_pos_.y = mouse_y - pos_.y;

            input.set_selected(this, void_selected_ids::selected_move);
            return input_response::handled();
        }
    }

    return input_response::empty();
}

input_response void_::input_search(const input_base& input)
{
    if (!has_search_)
        return input_response::empty();

    if (search_opened_) {
        auto res = search_text_field_->input(input);
        if (res.is_handled())
            return res;
    }

    if (!input_nothing_selected())
        return input_response::empty();

    float mouse_x, mouse_y;
    input.event().get_cursor_pos(mouse_x, mouse_y);

    if (input.event().is_message(message_type::mouse_move)) {
        if (util::is_in_quad(mouse_x, mouse_y, search_pos_)) {
            input.set_hovered(search_text_field_.get());
            cursors().set_cursor(cursor::hand);
            return input_response::handled();
        }
    }

    else if (input.event().is_message(message_type::mouse_button_down) &&
             input.event().get_mouse_button() == mouse_button::left) {
        if (util::is_in_quad(mouse_x, mouse_y, search_pos_)) {
            search_opened_ = !search_opened_;

            if (search_opened_)
                search_text_field_->start_typing();
            else {
                if (input.is_selected(search_text_field_.get()))
                    input.clear_selected();
                search_text_field_->set_string(std::u32string());
            }

            return input_response::handled();
        }
    }

    return input_response::empty();
}

void void_::update_scale(float scale)
{
#if defined(_DEBUG)
    renderer_.assert_render_thread();
#endif
    if (scale == scale_)
        return;
    
    assert(scale > 0.f);

    const float ratio = scale / scale_;
    const float old_w = pos_.w;
    const float old_h = pos_.h;
    pos_.w *= ratio;
    pos_.h *= ratio;

    pos_.x += (old_w - pos_.w) * 0.5f;
    pos_.y += (old_h - pos_.h) * 0.5f;

    scale_ = scale;

    // notify
    icons().on_scale_changed();
    sidebar_->on_scale_change();
    watermark().on_scale_changed();
    notifications().on_scale_changed();
    account().on_scale_changed();

    for (auto& o : overlays_)
        o->on_scale_changed();

    search_text_field_->on_scale_changed();

    // rebuild fonts
    fonts().create();
    fonts().build();

    callbacks().invoke<callbacks::callback_OnScaleChange>(scale_);
}

void_end_