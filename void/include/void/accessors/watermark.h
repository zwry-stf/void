#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>
#include <vector>
#include <void/util/xstr.h>
#include <functional>
#include <chrono>


void_begin_

class watermark_component {
protected:
    bool width_cached_{ false };
    float cached_width_;

public:
    watermark_component() = default;

public:
    virtual void update(void_* instance) = 0;
    virtual void render(void_* instance, const r2::rectf& pos, float fade_width, float animation) = 0;
    virtual void on_scale_change();

    [[nodiscard]] float cached_width() const noexcept {
        assert(width_cached_);
        return cached_width_;
    }
};

class watermark : protected vobj {
private:
    float animation_{ 0.f };

    bool  title_width_calculated_{ false };
    float title_width_;

    std::vector<std::unique_ptr<watermark_component>> components_;

public:
    using vobj::vobj;

public:
    void add_component(std::unique_ptr<watermark_component>&& component);
    void add_avarage_component(const xstr& title, std::function<float()>&& callback,
                               std::chrono::milliseconds update_time);
    void add_text_component(const xstr* text);

    void render();
    void on_scale_changed();

private:
    void add_border(float& x, float y, float h, float spacing);
};

void_end_