#include <void/builder/overlay.h>
#include <void/builder/builder.h>
#include <void/void.h>
#include <void/contents/overlays/custom_overlay.h>
#include <config/config.h>


void_begin_

overlay_builder::overlay_builder(void_* instance, custom_overlay* overlay_instance)
	: vobj(instance),
	  overlay_instance_(overlay_instance)
{
}

overlay_builder::~overlay_builder()
{
	if (!config_path_.empty()) {
		xstr config_path = xstr("overlay");
		config_path.append_safe(config_path_);

		instance()->config().add_module(
			std::make_unique<default_config_module<r2::rectf>>(
				config_path,
				&overlay_instance_->data().pos
			)
		);
	}
}

overlay_builder& overlay_builder::pos(float x, float y)
{
	overlay_instance_->set_pos(
		r2::vec2{
			x,
			y
		}
	);

	return *this;
}

overlay_builder& overlay_builder::size(float w, float h)
{
	overlay_instance_->set_size(
		r2::vec2{
			w,
			h
		}
	);

	return *this;
}

overlay_builder& overlay_builder::min(float w, float h)
{
	overlay_instance_->cfg().min_width.raw() = w;
	overlay_instance_->cfg().min_height.raw() = h;

	return *this;
}

overlay_builder& overlay_builder::max(float w, float h)
{
	overlay_instance_->cfg().max_width.raw() = w;
	overlay_instance_->cfg().max_height.raw() = h;

	return *this;
}

overlay_builder& overlay_builder::make_resizable(bool value)
{
	overlay_instance_->cfg().resizable = value;

	return *this;
}

overlay_builder& overlay_builder::liquid_glass(bool value)
{
	overlay_instance_->cfg().liquid_glass = value;

	return *this;
}

overlay_builder& overlay_builder::on_render(std::function<void(void_* instance, custom_overlay&)>&& callback)
{
	overlay_instance_->set_render_callback(std::move(callback));

	return *this;
}

overlay_builder& overlay_builder::on_update(std::function<void(void_* instance, custom_overlay&)>&& callback)
{
	overlay_instance_->set_update_callback(std::move(callback));

	return *this;
}

overlay_builder& overlay_builder::config(const xstr& path)
{
	config_path_ = path;

	return *this;
}

void_end_