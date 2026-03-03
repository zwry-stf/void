#pragma once
#include <void/contents/widgets/widget.h>
#include <string>
#include <format>


void_begin_

using format_condition = std::function<bool(float, std::string&)>;

class slider : public widget {
private:
    const xstr name_;
    float* const value_;
    float min_;
    float max_;
    const std::format_string<float> format_;
    float last_value_;
    std::string formatted_text_;

    int num_decimals_{ 4 };

    float animation_{ 0.f };
    float half_slider_height_;
    r2::rectf slider_pos_;

    bool text_width_calculated_{ false };
    float text_width_;

    std::vector<format_condition> conditions_;
    std::function<float()> get_min_callback_;
    std::function<float()> get_max_callback_;

public:
    slider(void_* instance, input_owner* input_owner,
           const xstr& name, float* value, float min, float max, const std::format_string<float>& format);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;
    virtual void set_pos(const r2::vec2& pos) override;

    virtual void on_activate() override;
    virtual void on_scale_change() override;
    virtual bool matches_search(const xstr& search) noexcept override;

    void set_decimal_count(int count) noexcept {
        num_decimals_ = count;
    }

    void add_format_condition(format_condition&& condition) {
        conditions_.emplace_back(std::move(condition));
    }

    void set_get_min_callback(std::function<float()>&& callback) {
        get_min_callback_ = std::move(callback);
    }
    void set_get_max_callback(std::function<float()>&& callback) {
        get_max_callback_ = std::move(callback);
    }

private:
    void update_value(float mouse_x);
    void set_value(float scale);
};

void_end_