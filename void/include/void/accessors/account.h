#pragma once
#include <void/util/vobj.h>
#include <void/util/time_point.h>
#include <void/util/xstr.h>
#include <void/accessors/icons.h>
#include <r2/renderer_definitions.h>
#include <void/contents/input/input_receiver.h>
#include <void/contents/input/input_response.h>
#include <void/contents/input/events.h>
#include <string>


void_begin_

class account : protected vobj,
                public input_receiver {
private:
    time_point expiration_time_;
    xstr display_name_;
    icons::icon_handle icon_{ icons::kInvalidHandle };

    std::string display_text_;
    bool was_expired_{ false };
    time_point last_expiration_time_;

    r2::rectf last_pos_;

    std::int32_t menu_options_;
    friend class menu_options;

    r2::rectf text_pos_;
    float name_animation_hovered_{ 0.f };
    bool name_text_width_calculated_{ false };
    float name_text_width_;

public:
    account(void_* instance);
    ~account();

public:
    void create_overlay();

public:
    void set_expiration_time(const time_point& t);
    void set_display_name(const xstr& name);
    void set_pfp(icons::icon_handle icon);

    [[nodiscard]] const auto& get_display_name() const noexcept {
        return display_name_;
    }

    void render(const render_input& input);
    input_response input(const input_base& input);
    void on_scale_changed();
};

void_end_