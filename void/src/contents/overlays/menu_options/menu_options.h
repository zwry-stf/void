#pragma once
#include <void/contents/overlays/overlay.h>
#include <void/contents/widgets/widget.h>
#include <void/contents/input/input_owner_overlay.h>
#include <void/accessors/icons.h>


void_begin_

class menu_options : public overlay,
                     public input_receiver,
                     public input_owner_overlay {
private:
    std::vector<std::unique_ptr<widget>> widgets_;

    icons::icon_handle icon_{ icons::kInvalidHandle };
    float header_height_;
    r2::rectf account_offset_;

    std::size_t selected_scale_{ 0u };
    std::size_t last_selected_scale_{ 0u };

public:
    menu_options(void_* instance, input_owner* input_owner,
                 input_owner_overlay* overlay_owner);

public:
    virtual void update(const overlay_render_input& input) override;
    virtual void render() override;
    virtual input_response input(const overlay_input& input) override;
    virtual void render_overlays() override;
    virtual void on_scale_changed() override;
};

void_end_