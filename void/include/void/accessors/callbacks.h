#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <r2/renderer_definitions.h>
#include <void/contents/input/message_event.h>
#include <functional>


void_begin_

#define _new_void_callback(_name, _callback_type) private: \
    std::function<void##_callback_type> callback_##_name##_{}; \
public: \
    struct callback_##_name {}; \
private: \
    template <> struct callback_type<callback_##_name> { \
        using type = std::function<void##_callback_type>; \
        static constexpr auto member = &callbacks::callback_##_name##_; \
    }

class callbacks : protected vobj {
public:
    using vobj::vobj;

private:
    template <typename T> struct callback_type;
    
    
    // Callback after the menu was toggled
    // args: bool menuOpen
    _new_void_callback(OnToggleMenu,        (bool));
    // Callback after the menu created its fonts, add your fonts here
    // usage: Vane::fonts().addFont(SOME_RESOURCE, outFont, font_size, font_cfg, ranges);
    _new_void_callback(OnCreateFonts,       ());
    // Render Callback before the menu starts rendering
    _new_void_callback(OnPreRender,         ());
    // Render Callback after the menu has rendered
    _new_void_callback(OnPostRender,        ());
    // Render Callback before overlay rendering
    _new_void_callback(OnPreRenderOverlay,  ());
    // Render Callback after overlay rendering
    _new_void_callback(OnPostRenderOverlay, ());
    // Load Callback after loading a config
    _new_void_callback(OnLoadConfig,        ());
    // Load Callback after loading a theme
    _new_void_callback(OnLoadTheme,         ());
    // Save Callback after loading a config
    _new_void_callback(OnSaveConfig,        ());
    // Save Callback after loading a theme
    _new_void_callback(OnSaveTheme,         ());

public:
    template <typename T>
    void set(typename callback_type<T>::type&& value) noexcept {
        this->*(callback_type<T>::member) = std::move(value);
    }

    template <typename T, typename... Args>
        requires (
            requires(typename callback_type<T>::type& callback, Args&&... args) 
            { callback(std::forward<Args>(args)...); }
        )
    void invoke(Args&&... args) const noexcept {
        auto& fn = this->*(callback_type<T>::member);
        if (fn)
            fn(std::forward<Args>(args)...);
    }
};

#undef _new_void_callback

void_end_