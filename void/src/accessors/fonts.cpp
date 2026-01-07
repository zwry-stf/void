#include <void/accessors/fonts.h>
#include <void/void.h>
#include <resources/resources.h>


void_begin_

void fonts::create()
{
    auto& renderer = instance()->renderer();
    auto& style = instance()->style();

    if (font_small_ != nullptr) {
        renderer.remove_font(font_small_);
        font_small_ = nullptr;
    }
    if (font_large_ != nullptr) {
        renderer.remove_font(font_large_);
        font_large_ = nullptr;
    }

    r2::font_cfg fcfg{};
    fcfg.offset_y = -1;
    fcfg.oversample_h = 2;
    fcfg.oversample_v = 2;
    fcfg.glow_strength = 2.f;

    // small font
    fcfg.size = static_cast<std::uint32_t>(
        style.text_size_small.get(instance()->scale())
    );
    fcfg.glow_radius = static_cast<std::uint32_t>(
        style.text_size_small.get(instance()->scale() * 0.2f)
    );
    font_small_ = renderer.add_font(fcfg);

    // large font
    fcfg.size = static_cast<std::uint32_t>(
        style.text_size_large.get(instance()->scale())
    );
    fcfg.glow_radius = static_cast<std::uint32_t>(
        style.text_size_large.get(instance()->scale() * 0.2f)
    );
    font_large_ = renderer.add_font(fcfg);

    // add fonts
    const vo::loaded_resource font1 = vo::get().resources().load_resource(
        void_resources::NotoSans_Medium_ttf
    );
    font_small_->add_font(font1.data(), font1.size());
    font_large_->add_font(font1.data(), font1.size());

    const vo::loaded_resource font2 = vo::get().resources().load_resource(
        void_resources::MPLUSRounded1c_Medium_ttf
    );
    font_small_->add_font(font2.data(), font2.size());
    font_large_->add_font(font2.data(), font2.size());

    const vo::loaded_resource font3 = vo::get().resources().load_resource(
        void_resources::NotoEmoji_Medium_ttf
    );
    font_small_->add_font(font3.data(), font3.size());
    font_large_->add_font(font3.data(), font3.size());
}

void fonts::build()
{
    assert(font_small_ != nullptr);
    assert(font_large_ != nullptr);

    auto& renderer = instance()->renderer();
    if (renderer.is_initialized()) {
        font_small_->build();
        font_large_->build();
    }
    else {
        renderer.build_fonts();
    }
}

void fonts::bind_font_small() const
{
    instance()->renderer().set_current_font(font_small_);
}

void fonts::bind_font_large() const
{
    instance()->renderer().set_current_font(font_large_);
}

void_end_