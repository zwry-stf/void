#pragma once
#include <void/contents/widgets/widget.h>


void_begin_

class input_owner_overlay;

enum class group_area : std::uint8_t {
    left,
    right,
    max
};

class group : protected vobj {
    const xstr name_;
    const group_area area_;

    std::vector<std::unique_ptr<widget>> widgets_;

    r2::rectf last_pos_;

    bool  text_width_calculated_{ false };
    float text_width_;

    bool occluded_{ false };

public:
    group(void_* instance, const xstr& name, group_area area);

public:
    float update(float x, float y, float w,
                 const render_input& input, float& substract_last_widget);
    void render(float alpha);
    input_response input(const input_base& input, bool selected_only = false);

    void on_activate();
    void on_scale_change();

    void search(const xstr& text);

    widget* add_widget(std::unique_ptr<widget>&& widget) {
        widgets_.push_back(std::move(widget));
        return widgets_.back().get();
    }

    [[nodiscard]] auto area() const noexcept {
        return area_;
    }

private:
    void render_outline(float alpha);
};

void_end_