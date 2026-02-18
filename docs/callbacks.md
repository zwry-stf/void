# Callbacks

Void exposes several callback “hooks” through accessors and builder constructs.

## Menu callbacks

There is a general callbacks accessor in:
- [`void/accessors/callbacks.h`](../void/include/void/accessors/callbacks.h)

Example:
```cpp
instance->callbacks().set<vo::callbacks::callback_OnCreateFonts>(
    []() -> void {
        //
    }
);
instance->callbacks().set<vo::callbacks::callback_OnPostRender>(
    [this]() -> void {
        //
    }
);
instance->callbacks().set<vo::callbacks::callback_OnToggleMenu>(
    [this](bool /* is_open */) -> void {
        //
    }
);
```

## Overlay callbacks

Custom overlays support:
- `on_render(void_*, custom_overlay&)`
- `on_update(void_*, custom_overlay&)`
- `on_input(void_*, custom_overlay&, const input_base&) -> input_response`

See: [`void/builder/overlay.h`](../void/include/void/builder/overlay.h)