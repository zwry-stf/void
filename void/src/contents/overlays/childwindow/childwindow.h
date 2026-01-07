#pragma once
#include <void/contents/overlays/overlay.h>
#include <void/contents/widgets/widget.h>
#include <void/contents/input/input_owner_overlay.h>


void_begin_

class childwindow : public overlay,
                    public input_owner_overlay {
private:
    const xstr name_;

    std::vector<std::unique_ptr<widget>> widgets_;
    std::vector<std::unique_ptr<overlay>> overlays_;

public:
    childwindow(void_* instance, input_owner_overlay* input_owner, const xstr& name);

public:
    virtual void update(const overlay_render_input& input) override;
    virtual void render() override;
    virtual input_response input(const overlay_input& input) override;
    virtual void render_overlays() override;
    virtual void on_scale_changed() override;

    void add_widget(std::unique_ptr<widget>&& widget) {
        widgets_.push_back(std::move(widget));
    }
};

void_end_