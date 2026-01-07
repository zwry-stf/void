#pragma once
#include <void/util/vobj.h>
#include <r2/renderer_definitions.h>


void_begin_

class render_target : protected vobj {
private:
    std::unique_ptr<r2::textureview> main_rtv_;
    std::unique_ptr<r2::framebuffer> main_fbo_;

    std::unique_ptr<r2::texture2d> menu_tex_;
    std::unique_ptr<r2::textureview> menu_view_;
    std::unique_ptr<r2::framebuffer> menu_fbo_;
    std::unique_ptr<r2::shaderprogram> menu_shader_;
    std::unique_ptr<r2::buffer> menu_vb_;
    std::unique_ptr<r2::buffer> menu_cb_;

    friend class _background;

public:
    using vobj::vobj;

public:
    void init();
    void init_targets();
    void destroy();
    void destroy_targets();

    void bind_main() const noexcept;
    void clear_menu() const noexcept;
    void bind_menu() const noexcept;
    void draw_menu() noexcept;
    
public:
    [[nodiscard]] auto* main_rtv() const noexcept {
        return main_rtv_.get();
    }
    [[nodiscard]] auto* main_fbo() const noexcept {
        return main_fbo_.get();
    }
    [[nodiscard]] auto* menu_tex() const noexcept {
        return menu_tex_.get();
    }
};

void_end_