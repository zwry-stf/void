#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <r2/renderer_definitions.h>
#include <void/contents/input/events.h>
#include <void/contents/input/input_response.h>
#include <void/accessors/icons.h>


void_begin_

struct scaled_icon;
class scrollbar;
class input_owner_overlay;

class child_tab : protected vobj,
                  public input_receiver {
protected:
    const xstr name_;
    input_owner_overlay* const parent_overlay_owner_;
    const std::int32_t input_id_;

    r2::rectf last_pos_;

    bool  text_width_calculated_{ false };
    float text_width_;

    icons::icon_handle icon_{ icons::kInvalidHandle };
    float highest_pos_;
    float scrollbar_width_;

    float animation_selected_{ 0.f };
    float animation_hovered_{ 0.f };

    std::unique_ptr<scrollbar> scrollbar_;

public:
    child_tab(void_* instance, input_owner* input_owner, std::int32_t num_input, std::int32_t input_id,
              input_owner_overlay* overlay_owner, const xstr& name);
    ~child_tab();

    child_tab(const child_tab&) = delete;
    child_tab& operator=(const child_tab&) = delete;

public:
    virtual float update(float x, float y, bool selected, const render_input& input);
    virtual void render(float alpha, bool selected);
    virtual input_response input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id);

    virtual void on_activate(bool parent_change, bool first = false);
    virtual void on_scale_change();

    void set_icon(int resource_id);

    [[nodiscard]] const auto& last_pos() const noexcept {
        return last_pos_;
    }

protected:
    void update_scrollbar();
    void render_scrollbar(float alpha);

    input_response input_scrollbar(const input_base& input);
};

void_end_