#pragma once
#include <void/contents/widgets/widget.h>
#include <void/contents/widgets/textfield.h>


void_begin_

class textfield_widget : public widget {
private:
    const xstr name_;
    std::function<void(const std::u32string&)> callback_;

    std::unique_ptr<textfield> text_field_;

    r2::rectf text_pos_;

public:
    textfield_widget(void_* instance, input_owner* input_owner, 
                     const xstr& name, std::function<void(const std::u32string&)>&& callback,
                     textfield_type type, textfield_flags flags, const xstr& default_text, std::size_t max_length);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;

    virtual void on_activate() override;
    virtual void on_scale_change() override;

    virtual bool matches_search(const xstr& search) noexcept override;

    void change_textfield(std::unique_ptr<textfield>&& textfield);
};

void_end_