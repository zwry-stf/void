#pragma once
#include <void/contents/widgets/widget.h>
#include <void/contents/widgets/list_options.h>
#include <void/contents/widgets/textfield.h>
#include <void/contents/widgets/scrollbar.h>


void_begin_

class list : public widget {
private:
	const xstr name_;
	std::size_t* const value_;
	const int num_rows_;
	const bool has_search_;
	std::unique_ptr<list_options> options_;

	std::unique_ptr<textfield> text_field_;
	std::u32string last_search_;
	float text_field_spacing_;
	float search_offset_;

	std::unique_ptr<scrollbar> scrollbar_;

	struct list_member {
		bool skipped{ false };
		float hovered{ 0.f };
		float selected{ 0.f };
	};
	std::vector<list_member> member_states_;

	r2::rectf list_pos_;

	float spacing_;
	float row_height_;

	float animation_hovered_search_{ 0.f };

	bool no_results_width_calculated_{ false };
	float no_results_width_;
	bool render_no_results_;

	bool options_updated_{ false };

public:
	list(void_* instance, input_owner* input_owner, std::unique_ptr<list_options>&& options,
		 const xstr& name, std::size_t* selected, int num_rows, bool has_search);

public:
    virtual void update(float x, float y, float w, const render_input& input, bool occluded) override;
    virtual void render(float alpha) override;
    virtual input_response input(const input_base& input) override;
	virtual void on_activate() override;
	virtual void on_scale_change() override;
    virtual bool matches_search(const xstr& search) noexcept override;
};

void_end_