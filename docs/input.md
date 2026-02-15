# Input routing

Void has an input layer that accepts events from your windowing system.

The sample uses GLFW callbacks and forwards events to:

- `void_::input().input_glfw_char(...)`
- `void_::input().input_glfw_key(...)`
- `void_::input().input_glfw_mouse_button(...)`
- `void_::input().input_glfw_scroll(...)`

See: [`TestRun/main.cpp`](../TestRun/main.cpp)

## Cursor integration

Void exposes a cursor selector; the sample maps that to GLFW standard cursors:

```cpp
int cursor = instance.cursors().get_cursor_glfw();
glfwSetCursor(window, glfw_cursor);
```

See: [`void/accessors/cursors.h`](../void/include/void/accessors/cursors.h)

## Overlay input

Custom overlays can enable/disable input and can also implement an input callback:
- `custom_overlay::toggle_input(bool)`
- `overlay_builder::on_input(...)`
