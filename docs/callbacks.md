# Callbacks

Void exposes several callback “hooks” through accessors and builder constructs.

## Overlay callbacks

Custom overlays support:
- `on_render(void_*, custom_overlay&)`
- `on_update(void_*, custom_overlay&)`
- `on_input(void_*, custom_overlay&, const input_base&) -> input_response`

See: [`void/builder/overlay.h`](../void/include/void/builder/overlay.h)

## Render lifecycle callbacks

There is a general callbacks accessor in:
- [`void/accessors/callbacks.h`](../void/include/void/accessors/callbacks.h)

Use this when you want to attach work to Void’s internal update/render phases rather than using a custom overlay.
