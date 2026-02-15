# Getting started

This project is designed to be embedded as a static library into your application.
The reference integration is in [`TestRun/main.cpp`](../TestRun/main.cpp) and is the best “truth source” for wiring.

## Build & link (Premake)

Void ships with a premake module: [`premake/void.lua`](../premake/void.lua). It creates two static libs:

- `void` – the UI/menu/overlay framework
- `resources` – icons, fonts, shaders, etc. (Void depends on this)

Void also depends on `r2` (renderer abstraction) and one of its backends (`d3d11` or `opengl`).

Minimal premake wiring pattern:

```lua
-- prerequisites:
--   include("<path-to-r2>/premake/r2.lua")
--   include("<path-to-void>/premake/void.lua")

group "thirdparty"
r2.add_projects({
  base = r2_dir,
  backend = "d3d11",         -- or "opengl"
  build_root = build_root,
  int_root = int_root,
})

group "void"
void.add_projects({
  base = void_dir,
  r2_dir = r2_dir,
  backend = "d3d11",         -- must match r2 backend
  build_root = build_root,
  int_root = int_root,
})
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
