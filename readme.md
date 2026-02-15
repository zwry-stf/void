# Void

> A C++ menu + overlay framework built around a fluent **builder API** (tabs, child tabs, groups, widgets) with **config + themes**, **notifications**, **keybinds**, and **top-level overlays** (including a liquid-glass style effect).

---

## Gallery

![Fullscreen preview](docs/assets/fullscreen_example_tab.png)

**Config tab (with different theme)**
![Config preview](docs/assets/config_tab_menu.png)

**Overlay backgrounds**
- Default background  
  ![Overlay default](docs/assets/overlay_default.png)
- Liquid glass effect  
  ![Overlay glass](docs/assets/overlay_glass.png)

---

## Features

### Menu & overlays
- Full menu system with **tabs**, **child tabs**, left/right **groups**, and a rich set of widgets
- **Menu-level overlays** and **top-level overlays**
- Overlay background styles:
  - Default overlay background
  - Liquid glass effect

➡️ **Overlays documentation:** see [`docs/overlays.md`](docs/overlays.md)

### Config system
- Builder-created widgets automatically persist (no manual wiring for typical UI state)
- Supports custom config modules (fixed-size and dynamic modules)

### Themes
- Theme loading/saving
- Theme transition animations (if enabled by options)

### Widgets / components
- Standard input widgets (toggle, slider, dropdown, multi-select, list, textfield, button, colorpicker, etc.)
- **Child widgets** attached to widgets, supports unlimited amount
- **Child windows** attached to widgets
- **Keybinds** + optional **keybind list**
- **Notifications** (colored text components)
- **Account display** (pfp + name)
- **Watermark system**
  - Text components
  - “Average” components (prefix + value from user function, refreshed on interval)
  - Support for custom components

---

## Getting started

### 1) Add the library to your project

- Add as a submodule + include in your build

**Using premake**:
```lua
workspace ""
    -- your options here...

    local void_dir = --
    local r2_dir = -- 
    local used_backend = "d3d11"
    local build_root = --
    local int_root = --

    local configurations = {
        debug = "Debug",
        release = "Release
    }

include (void_dir .. "/premake/void.lua")
include (r2_dir .. "/premake/r2.lua")

group "void"
void.add_projects(
    {
        base = void_dir,
        build_root = build_root,
        int_root = int_root,
        r2_dir = r2_dir,
        backend = used_backend,
    },
    configurations
)
group "void/deps"
r2.add_projects(
    {
        base = r2_dir,
        build_root = build_root,
        int_root = int_root,
        backend = used_backend,
    },
    configurations
)
group ""

-- integrate into your project
project ""
    -- your options here...

    -- integrate renderer if needed
    r2.set_common_project_settings(configurations) // define _DEBUG, NDEBUG

    r2.use { base = r2_dir, backend = used_backend, include_impl = true } // add r2 include dirs
    r2.set_project_backend_defines( used_backend )

    -- add void to your includes and link
    
    includedirs {
        void_dir .. "/void/include",
        void_dir .. "/void/src",
        void_dir .. "/resources/include",
    }
    
    dependson { "void" }
    links     { "void", "resources" }


```

### 2) Minimal initialization flow
This is the **recommended order**:
1. Create the instance
2. Register callbacks (fonts, toggle hook, config/theme hooks, etc.)
3. Set options (must match “mutable after init” rules)
4. Optional: set account info, watermark, etc.
5. Call `init(platform_init_data, backend_init_data)`
6. Build UI using the menu builder

> The exact platform/backend init structs depend on your renderer integration.

### 3) Minimal menu + overlay example
This example demonstrates the core concepts:
- Create a menu with a tab + child tab
- Add left/right groups with widgets
- Add a top-level overlay with render/update/input callbacks

```cpp
#include <void/void.h>
#include <void/builder/builder.h>

static std::unique_ptr<vo::void_> g_ui;

static bool g_enabled = true;
static float g_value = 0.5f;
static std::size_t g_selected = 0;

void create_ui() {
    auto mb = g_ui->get_builder();

    mb.label("Example");

    // Tab + child tab
    auto tab = mb.tab("Main");
    auto child = tab.child("General");

    // Groups + widgets
    child.left_group("Controls")
        .toggle("Enabled", g_enabled)
        .slider("Value", g_value, 0.0f, 1.0f)
            ->decimal_count(2);

    child.right_group("Selection")
        .dropdown(
            "Mode",
            vo::list_options::create_constant(/* list_options* */),
            g_selected
        );

    // Top-level overlay
    mb.overlay(/* in_menu_layer = */ false)
        .config("example_overlay")
        .make_movable(true)
        .make_resizable(false)
        .clamp_in_window(false)
        .pos(0.02f, 0.2f)
        .size(200.0f, 80.0f)
        .on_render([](vo::void_* instance, vo::custom_overlay& overlay) {
            (void)instance; (void)overlay;
            // TODO: render overlay content
        })
        .on_update([](vo::void_* instance, vo::custom_overlay& overlay) {
            (void)instance; (void)overlay;
            // TODO: update overlay state
        })
        // resize, move etc. will be handled internally
        .on_input([](vo::void_* instance, vo::custom_overlay& overlay, const vo::input_base& input) -> vo::input_response {
            (void)instance; (void)overlay; (void)input;
            // TODO: consume input if needed
            return vo::input_response{};
        });

    // See overlays documentation for details:
    // docs/overlays.md
}

void init_ui(/* platform + backend args here */) {
    g_ui = std::make_unique<vo::void_>();

    // Callbacks
    g_ui->callbacks().set<vo::callbacks::callback_OnCreateFonts>([]() {
        // TODO: create/register fonts here
    });

    g_ui->callbacks().set<vo::callbacks::callback_OnToggleMenu>([](bool open) {
        (void)open;
        // TODO: react to menu toggle if desired
    });

    // Options
    // respect “mutable after init” constraints.
    // g_ui->options().set<vo::options::option_MenuMSAA>(false);
    // g_ui->options().set<vo::options::option_NoBlur>(false);

    // Optional: account + watermark usage (fill in if you expose these APIs publicly)
    // g_ui->account().set_display_name("user");
    // g_ui->watermark().add_text_component(&some_string);
    // g_ui->watermark().add_avarage_component("fps", [](){ return ...; }, std::chrono::milliseconds(500));

    // Initialize platform/backend
    // TODO: create your platform_init_data and backend_init_data then:
    // g_ui->init(platform_init_data, backend_init_data);

    create_ui();
}
```

### Documentation
[`docs/overlays.md`](docs/index.md)