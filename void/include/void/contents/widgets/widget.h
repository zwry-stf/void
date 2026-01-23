#pragma once
#include <void/util/vobj.h>
#include <void/contents/input/events.h>
#include <void/contents/input/input_response.h>
#include <r2/renderer_definitions.h>
#include <void/util/xstr.h>
#include <functional>
#include "widget_child.h"


void_begin_

class widget : protected vobj,
               public input_receiver {
protected:
    r2::rectf last_pos_;

    float animation_hovered_{ 0.f };
    float animation_selected_{ 0.f };
    float animation_disabled_{ 0.f };

    bool was_occluded_{ false };

    std::function<bool()> disabled_callback_;
    std::function<bool()> visible_callback_;
    bool skipped_{ false };

    std::vector<std::unique_ptr<widget_child>> child_widgets_;
    float child_widget_left_pos_{ 0.f };

public:
    explicit widget(void_* instance, input_owner* input_owner, std::int32_t input_num) noexcept;
    virtual ~widget();

    widget(const widget&) = delete;
    widget& operator=(const widget&) = delete;

public:
    // update function called every frame
    virtual void update(float x, float y, float w, const render_input& input, bool occluded);
    // render function called every frame
    virtual void render(float alpha) = 0;
    // input function
    virtual input_response input(const input_base& input) = 0;

    // Initializes things like animation when switching to active tab/window 
    virtual void on_activate();
    // callback for when global scale changes, reset cached text sizes etc. in here
    virtual void on_scale_change() {};

    // return true if search query found in widget
    virtual bool matches_search(const xstr& search) noexcept;

public:
    // check whether the widget should be disabled
    [[nodiscard]] bool is_disabled() const {
        return disabled_callback_ && disabled_callback_();
    }
    // check if widget should be visible
    [[nodiscard]] bool is_visible() const {
        return !visible_callback_ || visible_callback_();
    }
    // get the last cached render height
    [[nodiscard]] float get_height() const noexcept {
        return last_pos_.h;
    }
    // returns whether the widget is currently skipped in search
    [[nodiscard]] bool is_skipped() const noexcept {
        return skipped_;
    }
    // returns whether the widget is selected (including child widgets)
    [[nodiscard]] bool is_selected(const input_base& input) const {
        if (input.is_selected(this))
            return true;
        for (auto& c : child_widgets_) {
            if (input.is_selected(c.get()))
                return true;
        }

        return false;
    }

    // allows you to update the widgets position after update has been called without any affecting any other states
    virtual void set_pos(const r2::vec2& pos) {
        last_pos_.x = pos.x;
        last_pos_.y = pos.y;
    }

    // add a child widget to the current widget
    void add_child(std::unique_ptr<widget_child>&& child_widget) {
        child_widgets_.push_back(std::move(child_widget));
    }

    // set the disabled callback for the widget
    void set_disabled_callback(std::function<bool()>&& callback) {
        disabled_callback_ = std::move(callback);
    }

    // set the visible callback for the widget
    void set_visible_callback(std::function<bool()>&& callback) {
        visible_callback_ = std::move(callback);
    }

    // set whether widget should be skipped by search
    void set_skipped(bool state) noexcept { 
        skipped_ = state;
    }

protected:
    [[nodiscard]] bool can_receive_input_this(const input_base& input) const noexcept;
    [[nodiscard]] bool can_receive_input_simple(const input_base& input) const noexcept;
    void update_child_widgets(float right_x, const render_input& input);
    void render_child_widgets(float alpha);
    input_response input_child_widgets(const input_base& input);
    void set_child_widget_pos(float delta_x, float delta_y);
};

void_end_