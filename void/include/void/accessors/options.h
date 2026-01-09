#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <r2/renderer_definitions.h>
#include <void/contents/input/message_event.h>


void_begin_

#define _new_void_option(_name, _option_type, _default_value, _after_init) private: \
    _option_type option_##_name##_ = _default_value; \
public: \
    struct option_##_name {}; \
private: \
    template <> struct option_type<option_##_name> { \
        using type = _option_type; \
        static constexpr bool allow_after_init = _after_init; \
        static constexpr auto member = &options::option_##_name##_; \
    }

class options : protected vobj {
public:
    using vobj::vobj;

private:
    template <typename T> struct option_type;
    

    //                -- Name --   -- Type --   -- Default --          -- After Init --

    // Update the delta time and frame-start timestamp during menu rendering.
    // Type: bool
    // Default: true
    // Mutable after init: no
    _new_void_option(UpdateFrameTime, bool,         true,                  false);

    // Perform a full "frame start" automatically, instead of requiring callers
    // to invoke full updates themselves beforehand.
    // Type: bool
    // Default: true
    // Mutable after init: no
    _new_void_option(FullFrameUpdate, bool,         true,                  false);

    // Name of the configuration folder (inside the user's Documents).
    // Type: xstr
    // Default: "void-v2"
    // Mutable after init: no
    _new_void_option(ConfigName,      xstr,         "void-v2",             false);

    // Human-readable project name (used e.g. by notifications).
    // Type: xstr
    // Default: "void"
    // Mutable after init: no
    _new_void_option(ProjectName,     xstr,         "void",                false);

    // Default menu window size in unscaled pixels.
    // Type: ui_vec2
    // Default: (800.f, 550.f)
    // Mutable after init: no
    _new_void_option(DefaultSize,     r2::vec2,     r2::vec2(800.f, 550.f), false);

    // Start the menu minimized the first time it opens.
    // Type: bool
    // Default: false
    // Mutable after init: no
    _new_void_option(StartMinimized,  bool,         false,                 false);
    
    // Disable all blur effects (improves performance).
    // Type: bool
    // Default: false
    // Mutable after init: no
    _new_void_option(NoBlur,          bool,         false,                 false);

    // Enable msaa of the menu's render target
    // Type: bool
    // Default: true
    // Mutable after init: no
    _new_void_option(MenuMSAA,        bool,         true,                 false);

    // Downsample factor used by blur passes (performance/quality tradeoff).
    // Supported values: 1.0, 0.5, 0.25, 0.125
    // Type: float
    // Default: 0.25f
    // Mutable after init: no
    _new_void_option(DownsampleValue, float,        0.25f,                 false);

    // Allow the menu window to be resized.
    // Type: bool
    // Default: true
    // Mutable after init: yes
    _new_void_option(AllowResizing,   bool,         true,                  true);

    // Scale the menu relative to the screen size (not the window size).
    // Type: bool
    // Default: true
    // Mutable after init: no
    _new_void_option(DynamicSizing,   bool,         true,                  false);

    // Maximum size difference a rendered icon can have the loaded icons size
    // type: int32
    // default: 4
    // Mutable after init: no
    _new_void_option(MaxIconSizeDiff, std::int32_t, 4,                     false);
    
    // Enable Notifications.
    // Type: bool
    // Default: true
    // Mutable after init: true
    _new_void_option(Notifications,   bool,         true,                  true);
    
    // Enable Watermark.
    // Type: bool
    // Default: true
    // Mutable after init: true
    _new_void_option(Watermark,       bool,         true,                  true);

    // Show user account info in the UI.
    // Type: bool
    // Default: true
    // Mutable after init: no
    _new_void_option(UserAccount,     bool,         true,                  false);

    // Set the key used to toggle the menu
    // Type: key (enum)
    // Default: key::insert
    // Mutable after init: yes
    _new_void_option(MenuKey,         key,          key::insert,           true);

    // Block application/game input while the menu is open.
    // Type: bool
    // Default: true
    // Mutable after init: yes
    _new_void_option(BlockInput,      bool,         true,                  true);

    // Invoke the toggle-menu callback during initialization.
    // Type: bool
    // Default: true
    // Mutable after init: no
    _new_void_option(ToggleCallbackOnInit, bool,    true,                  false);

    // Show the keybind indicator list.
    // Type: bool
    // Default: true
    // Mutable after init: yes
    _new_void_option(IndicatorList,   bool,         true,                  true);

    // Hide "Always On" keybinds in the indicator list.
    // Type: bool
    // Default: true
    // Mutable after init: yes
    _new_void_option(HideAlwaysOn,    bool,         true,                  true);

    // Animate the loading of themes (transitions).
    // Type: bool
    // Default: true
    // Mutable after init: yes
    _new_void_option(AnimateThemes,   bool,         true,                  true);

    // Icon resource/index used by the menu’s sidebar. (-1 = default)
    // Type: int
    // Default: -1
    // Mutable after init: no
    _new_void_option(SidebarIcon,     int,          -1,                    false);

public:
    template <typename T>
    void set(const typename option_type<T>::type& value) noexcept {
        this->*(option_type<T>::member) = value;
    }

    template <typename T>
    [[nodiscard]] typename option_type<T>::type& get() noexcept {
        return this->*(option_type<T>::member);
    }

    template <typename T>
    [[nodiscard]] const typename option_type<T>::type& get() const noexcept {
        if constexpr (option_type<T>::allow_after_init)
            assert_noinit();
        return this->*(option_type<T>::member);
    }

private:
    void assert_noinit();
};

#undef _new_void_option

void_end_