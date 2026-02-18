# Input routing

Void has an input layer that accepts events from your windowing system.

The sample uses GLFW callbacks and forwards events to:

- `void_::input().input_glfw_char(...)`
- `void_::input().input_glfw_key(...)`
- `void_::input().input_glfw_mouse_button(...)`
- `void_::input().input_glfw_scroll(...)`

See: [`TestRun/main.cpp`](../TestRun/main.cpp)

It also supports raw Win32 events
- `input_win32(msg, wparam, lparam)`

## Cursor integration

Void exposes a cursor selector; the sample maps that to GLFW standard cursors:

```cpp
int cursor = instance.cursors().get_cursor_glfw();
auto glfw_cursor = glfwCreateStandardCursor(cursor);
glfwSetCursor(window, glfw_cursor);
```

Or using Win32 api:
```cpp
LPCSTR cursor = reinterpret_cast<LPCSTR>(instance->cursors().get_cursor_win32());
auto hCursor = LoadCursorA(NULL, cursor);
SetCursor(hCursor);
```

See: [`void/accessors/cursors.h`](../void/include/void/accessors/cursors.h)

## Overlay input

Custom overlays can enable/disable input and can also implement an input callback:
- `custom_overlay::toggle_input(bool)`
- `overlay_builder::on_input(...)`
