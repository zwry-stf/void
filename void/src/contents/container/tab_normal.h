#pragma once
#include <void/contents/container/tab.h>
#include <void/accessors/icons.h>


void_begin_

class input_owner_overlay;

class tab_normal : public tab,
                   public input_receiver {
private:
    const xstr name_;

    r2::rectf last_pos_;

    float animation_selected_{ 0.f };
    float animation_hovered_{ 0.f };

    bool text_width_calculated_{ false };
    float text_width_;

    icons::icon_handle icon_{ icons::kInvalidHandle };

    std::int32_t selected_child_{ 0 };
    std::vector<std::unique_ptr<class child_tab>> child_tabs_;

public:
    tab_normal(void_* instance, input_owner* input_owner, const xstr& name,
               input_owner_overlay* overlay_owner);
    ~tab_normal();

public:
    virtual float update(float x, float y, float w,
        const render_input& input, bool selected) override;
    virtual void render(bool selected) override;
    virtual input_response input(const input_base& input, std::int32_t& selected_tab, std::int32_t tab_id) override;

    virtual void on_activate(bool first = false) override;
    virtual void on_scale_change() override;

    void add_child_tab(std::unique_ptr<child_tab>&& child_tab);

private:
    void update_content(const render_input& input);
    void render_header();
    void render_content();
};
                   

void_end_