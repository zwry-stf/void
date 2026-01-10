#include <void/accessors/watermark.h>
#include <void/void.h>


void_begin_

/// components

void watermark_component::on_scale_change()
{
	width_cached_ = false;
}

class avarage_watermark_component : public watermark_component {
private:
    const xstr name_;
    const std::function<float()> get_callback_;
    const std::chrono::milliseconds update_time_;

    bool  title_width_calculated_{ false };
    float title_width_;

    std::string text_;
    float text_width_animation_{ 0.f };

    int counted_{ 0 };
    float total_{ 0.f };
    float current_{ 0.f };

    time_point last_update_;

public:
    avarage_watermark_component(const xstr& name, auto&& fn, auto update_time) 
        : name_(name),
          get_callback_(std::move(fn)),
          update_time_(update_time) {
        assert(name.find('\n') == xstr::npos);
    }

public:
    virtual void on_scale_change() override {
        watermark_component::on_scale_change();
        title_width_calculated_ = false;
    }
    virtual void update(void_* instance) override {
        auto& renderer = instance->renderer();
        auto& style = instance->style();
        auto& util = instance->util();

        const float text_spacing = std::round(style.spacing->get(instance->scale()) * 0.3f);

        // get title width
        if (!title_width_calculated_) {
            if (renderer.get_text_width_strict(name_, title_width_))
                title_width_calculated_ = true;
            else
                title_width_ = renderer.get_text_width(name_);
            title_width_ = std::ceil(title_width_);
            text_width_animation_ = -1.f;
        }

        // update value
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            instance->frame_start() - last_update_
        );
        const bool needs_update = last_update_ == time_point() ||
            elapsed >= update_time_;
        if (needs_update) {
            last_update_ = instance->frame_start();

            if (counted_ > 0)
                current_ = total_ / static_cast<float>(counted_);
            else
                current_ = 0.f;

            counted_ = 0;
            total_ = 0.f;
        }

        counted_++;
        total_ += get_callback_();

        // to string
        text_ = std::format("{:.0f}", current_);

        // animation
        const float text_width = std::ceil(renderer.get_text_width(text_));
        if (text_width_animation_ <= 0.f)
            text_width_animation_ = text_width;
        else
            text_width_animation_ = util.lerp(text_width_animation_, text_width, 2.f);

        width_cached_ = true;
        cached_width_ = title_width_ + text_spacing + text_width_animation_;
    }
    virtual void render(void_* instance, const r2::rectf& pos, float fade_width, float animation) override {
        auto& renderer = instance->renderer();
        auto& style = instance->style();

        const float text_spacing = std::round(style.spacing->get(instance->scale()) * 0.4f);
        const float text_spacing_y = std::round(
            (pos.h - style.text_size_small.get(instance->scale())) * 0.5f
        );

        renderer.add_text(
            r2::vec2(pos.x, pos.y + text_spacing_y),
            style.text().alpha(animation * 0.7f),
            name_
        );

        renderer.add_text_faded(
            r2::vec2(pos.x + title_width_ + text_spacing,
                pos.y + text_spacing_y),
            style.text_accent().alpha(animation), style.text_accent().transparent(),
            pos.x + pos.w, pos.x + pos.w + fade_width,
            text_
        );
    }
};

class text_watermark_component : public watermark_component
{
    const xstr* const text_;

    float text_width_animation_;

public:
    text_watermark_component(const xstr* text)
        : text_(text) { }

private:
    virtual void update(void_* instance) override {
        auto& renderer = instance->renderer();
        auto& util = instance->util();

        assert(text_->find('\n') == xstr::npos);

        const auto text_width = std::ceil(renderer.get_text_width(*text_));
        if (!width_cached_) {
            width_cached_ = true;
            text_width_animation_ = text_width;
        }
        else
            text_width_animation_ = util.lerp(text_width_animation_, text_width, 2.f);

        cached_width_ = text_width_animation_;
    }
    virtual void render(void_* instance, const r2::rectf& pos, float fade_width, float animation) override {
        auto& renderer = instance->renderer();
        auto& style = instance->style();

        const float text_spacing_y = std::round(
            (pos.h - style.text_size_small.get(instance->scale())) * 0.5f
        );

        renderer.add_text_faded(
            r2::vec2(pos.x, pos.y + text_spacing_y),
            style.text_accent().alpha(animation), style.text_accent().transparent(),
            pos.x + pos.w, pos.x + pos.w + fade_width,
            *text_
        );
    }
};

/// watermark
void watermark::add_component(std::unique_ptr<watermark_component>&& component)
{
    components_.push_back(std::move(component));
}

void watermark::add_avarage_component(const xstr& title, std::function<float()>&& callback, 
                                      std::chrono::milliseconds update_time)
{
    components_.push_back(
        std::make_unique<avarage_watermark_component>(title, std::move(callback), update_time)
    );
}

void watermark::add_text_component(const xstr* text)
{
    components_.push_back(
        std::make_unique<text_watermark_component>(text)
    );
}

void watermark::render()
{
    auto& util = instance()->util();
    auto& style = instance()->style();
    auto& renderer = instance()->renderer();

    animation_ = util.lerp(animation_,
        instance()->options().get<options::option_Watermark>()
    );
    if (animation_ < util::g_min_alpha)
        return;

    instance()->fonts().bind_font_small();

    const xstr& title = instance()->options().get<options::option_ProjectName>();
    assert(title.find('\n') == xstr::npos);
    if (!title_width_calculated_) {
        if (renderer.get_text_width_strict(title, title_width_))
            title_width_calculated_ = true;
        else
            title_width_ = renderer.get_text_width(title);
        title_width_ = std::ceil(title_width_);
    }

    // calc full width
    const float spacing = style.spacing->get(instance()->scale());
    const float spacing_x = std::round(spacing * 0.6f);

    const float full_border_size = spacing_x + style.border_size.get(instance()->scale()) + spacing_x;

    float full_width = spacing_x + title_width_;
    for (auto& c : components_) {
        full_width += full_border_size;

        c->update(instance());

        full_width += c->cached_width();
    }

    full_width += spacing_x;

    // background
    const float screen_spacing_x = spacing * 2.f;
    const float screen_spacing_y = std::round(screen_spacing_x * 0.8f);

    const auto display_size = renderer.get_render_size();

    const float text_size = style.text_size_small.get(instance()->scale());
    const float top_spacing = std::round(spacing_x * 0.6f);
    const float full_height = text_size + top_spacing * 2.f;

    const auto background_min =
        r2::vec2(display_size.x - screen_spacing_x - full_width,
            screen_spacing_y);
    const auto background_max =
        r2::vec2(display_size.x - screen_spacing_x,
            screen_spacing_y + full_height);

    const float border_rounding = std::round(style.rounding->get(instance()->scale()) * 0.7f);

    renderer.add_shadow_rect_filled(
        background_min, background_max,
        style.background().opague().alpha(animation_ * 0.6f),
        border_rounding,
        full_height
    );

    renderer.add_rect_filled(
        background_min, background_max,
        style.background().opague().alpha(animation_),
        border_rounding
    );

    // text
    float xpos = background_min.x + spacing_x;
    const float ypos = background_min.y + top_spacing;

    // title
    renderer.add_text(
        r2::vec2(xpos, ypos),
        style.accent().alpha(animation_),
        title
    );

    xpos += title_width_;

    // components
    for (auto& c : components_) {
        add_border(xpos, ypos, text_size, spacing_x);

        c->render(
            instance(),
            r2::rectf{
                xpos, ypos,
                c->cached_width(),
                text_size
            },
            spacing_x, /* fade width */
            animation_
        );

        xpos += c->cached_width();
    }
}

void watermark::on_scale_changed()
{
    title_width_calculated_ = false;

    for (auto& c : components_)
        c->on_scale_change();
}

void watermark::add_border(float& x, float y, float h, float spacing)
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    const float border_size = style.border_size.get(instance()->scale());

    const r2::color_u32 col = style.text_accent().alpha(animation_ * 0.7f);
    const r2::color_u32 transparent = style.text_accent().transparent();

    renderer.add_rect_filled_multicolor(
        r2::vec2(x + spacing, y),
        r2::vec2(x + spacing + border_size, y + h * 0.5f),
        transparent, transparent,
        col, col
    );

    renderer.add_rect_filled_multicolor(
        r2::vec2(x + spacing, y + h * 0.5f),
        r2::vec2(x + spacing + border_size, y + h),
        col, col,
        transparent, transparent
    );

    x += spacing + border_size + spacing;
}

void_end_