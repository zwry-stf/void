# Getting started

This project is designed to be embedded as a static library into your application.
The reference integration is in [`TestRun/main.cpp`](../TestRun/main.cpp).

## Build & link (Premake)

Void ships with a premake module: [`premake/void.lua`](../premake/void.lua). It creates two static libs:

- `void` – the UI/menu/overlay framework
- `resources` – icons, fonts, shaders, etc. (Void depends on this)

Void also depends on `r2` (renderer abstraction) and one of its backends (`d3d11` or `opengl`).

Example premake wiring pattern:

```lua
-- prerequisites:
--   include("<path-to-r2>/premake/r2.lua")
--   include("<path-to-void>/premake/void.lua")

workspace ""
    -- your options here...

    local void_dir = -- your path to void 
    local r2_dir = -- your path to r2
    local used_backend = "d3d11" -- or "opengl"
    local build_root = -- your build folder
    local int_root = -- your intermediate build folder

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

Your executable should link against `void` and `resources` (and whatever `r2` projects you use).

## Runtime integration checklist

You need to provide three things:

### 1) Platform init data
On Windows, the sample uses the Win32 window handle:
- `r2::platform_init_data pinit(hwnd)`

### 2) Backend init data
Depends on the backend:

- **D3D11**: provide the swapchain: `r2::backend_init_data binit(swapchain)`
- **OpenGL**: default construct: `r2::backend_init_data binit;`

### 3) Feed input events
Void has a small input layer. In the sample, GLFW callbacks forward events into Void:

- `input_glfw_char(codepoint)`
- `input_glfw_key(key, scancode, action, mods)`
- `input_glfw_mouse_button(button, action, mods)`
- `input_glfw_scroll(xoffset, yoffset)`

It also supports raw Win32 events
- input_win32(msg, wparam, lparam);

See: [`void/contents/input/input.h`](../void/include/void/contents/input/input.h)

## Minimal boot sequence

High-level flow (mirrors `TestRun/main.cpp`):

1. Create `vo::void_`.
2. Configure options if needed: `instance.options().set<option_X>(...)`.
3. Build UI widgets via the builder: `auto b = instance.get_builder(); ...`.
4. Call `instance.init(pinit, binit)`.
5. Each frame:
   - feed input events (or forward your own events)
   - call renderer / present (Void renders inside its own render step; see the integration in TestRun)

## Multi-threading notes

The sample renders on a dedicated render thread, while the window loop polls events on the main thread.
That pattern works as long as:
- input forwarding is safe for your threading model
- resize is handled carefully (see the sample’s `needs_resize` flag)

If you do not need a render thread, you can render from the main thread as well.
