#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>
#include <void/contents/input/events.h>
#include <void/contents/input/input_response.h>


void_begin_

class scrollbar : protected vobj,
                  public input_receiver {
private:
    float scroll_{ 0.f };
    float des_scroll_{ 0.f };
    float scroll_speed_{ 0.f };
    float highest_pos_;

    r2::rectf last_pos_;

    struct scroll_bar_cache {
        r2::rectf last_pos;
        float last_height;
        float width;
        float offset;
    };
    scroll_bar_cache bar_cache_;

    float moving_off_;

public:
    scrollbar(void_* instance, input_owner* input_owner);

public:
    void update(const r2::rectf& pos, float highest);
    void render(float alpha);
    input_response input(const input_base& input);

    void on_scroll(float scroll);

    [[nodiscard]] float get_scroll() const noexcept {
        return scroll_;
    }
    [[nodiscard]] float get_width() const noexcept {
        return last_pos_.w;
    }

    void set_scroll(float v) noexcept {
        scroll_ = v;
    }
    void set_des_scroll(float v) noexcept {
        des_scroll_ = v;
    }
    void set_scroll_speed(float v) noexcept {
        scroll_speed_ = v;
    }
};

void_end_