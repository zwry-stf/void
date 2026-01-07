#pragma once
#include <void/util/vobj.h>
#include <void/contents/input/overlay_events.h>
#include <void/contents/input/input_owner.h>
#include <void/contents/input/input_response.h>
#include <r2/renderer_definitions.h>


void_begin_

class overlay : protected vobj,
                public input_owner,
                public input_receiver_overlay {
protected:
    const bool has_overlays_;
    const bool is_childwindow_;

    r2::rectf last_pos_;
    float des_animation_{ 0.f };
    float animation_{ 0.f };
    float alpha_{ 0.f };

    r2::vec2 set_pos_;
    r2::vec2 parent_size_;
    bool pos_changed_{ false };

public:
    overlay(void_* instance, input_owner_overlay* input_owner,
            bool has_overlays = false, bool is_childwindow = false);

    overlay(const overlay&) = delete;
    overlay& operator=(const overlay&) = delete;

public:
    virtual void update(const overlay_render_input& input);
    virtual void render() = 0;
    virtual input_response input(const overlay_input& input) = 0;
    virtual void render_overlays() {};
    virtual void on_scale_changed() {};

    void set_pos(const r2::vec2& pos, const r2::vec2& parent_size = r2::vec2());

    [[nodiscard]] float alpha() const noexcept {
        return alpha_;
    }
    [[nodiscard]] bool is_childwindow() const noexcept {
        return has_overlays_;
    }
    [[nodiscard]] bool has_overlays() const noexcept {
        return is_childwindow_;
    }

protected:
    input_response should_block_input(const overlay_input& input);
};

void_end_