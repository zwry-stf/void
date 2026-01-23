#pragma once
#include <void/contents/widgets/widget.h>


void_begin_

class dropdown : public widget {
protected:
    const xstr name_;
    const std::int32_t overlay_id_;
    input_owner_overlay* const parent_overlay_owner_;

    r2::rectf dropdown_pos_;

public:
    dropdown(void_* instance, input_owner* input_owner,
             input_owner_overlay* overlay_owner, std::int32_t overlay_id, const xstr& name);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;
    virtual void set_pos(const r2::vec2& pos) override;

    virtual bool matches_search(const xstr& search) noexcept override;

public:
    static void render_dropdown(void_* instance, const r2::rectf& pos, float animation_hovered, float animation_selected, 
                                float animation_disabled, float alpha, const xstr& text, bool is_multiselect);
};

void_end_