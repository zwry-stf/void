#pragma once
#include <void/contents/overlays/overlay.h>
#include <void/contents/widgets/list_options.h>


void_begin_

class dropdown_overlay : public overlay,
                         public input_receiver {
private:
    std::size_t* const value_;
    std::unique_ptr<list_options> options_;

    float spacing_;
    float row_height_;

    struct member_animation {
        float hovered{ 0.f };
        float selected{ 0.f };
    };
    std::vector<member_animation> member_animations_;

public:
    dropdown_overlay(void_* instance, input_owner* input_owner,
                     input_owner_overlay* overlay_owner, std::unique_ptr<list_options>&& options, std::size_t* value);

public:
    virtual void update(const overlay_render_input& input) override;
    virtual void render() override;
    virtual input_response input(const overlay_input& input) override;


    [[nodiscard]] auto* selected() const noexcept {
        return value_;
    }
    [[nodiscard]] auto* options() const noexcept {
        return options_.get();
    }
};

void_end_