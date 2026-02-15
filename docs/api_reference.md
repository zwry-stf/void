# API reference (public surface)

This is a compact map of the public headers you typically include. For details, read the headers directly.

## Main entry

- [`void/void.h`](../void/include/void/void.h)
- [`void/void.inline.inl`](../void/include/void/void.inline.inl)

Primary type:
- `vo::void_`

Common methods visible in the sample:
- `init(platform_init_data, backend_init_data)`
- `renderer()`
- `get_builder()`
- `options()`, `style()`, `background()`, `theme()`
- `input()`, `cursors()`
- `watermark()`, `notifications()`
- `delta_time()`, `scale()`, `pos()`, `alpha()`

## Builder headers

- [`void/builder/builder.h`](../void/include/void/builder/builder.h)
- [`void/builder/tab.h`](../void/include/void/builder/tab.h)
- [`void/builder/group.h`](../void/include/void/builder/group.h)
- [`void/builder/overlay.h`](../void/include/void/builder/overlay.h)

## Accessors

Located in: `void/include/void/accessors/*`

- `account.h` — account display data (name/pfp)
- `background.h` — blur/noise/blend controls
- `callbacks.h` — lifecycle callbacks
- `config.h` — config loading/saving + modules
- `cursors.h` — cursor selection helpers
- `fonts.h` / `icons.h` / `resources.h` — resource access
- `notifications.h` — notification system
- `options.h` — feature flags / perf toggles
- `render_target.h` — render target access
- `style.h` — colors + scalar style values
- `theme.h` — theme load/save
- `util.h` — lerp/time helpers, etc.
- `watermark.h` — watermark components

## Widgets

Widgets and helper types:
- [`void/contents/widgets/widget.h`](../void/include/void/contents/widgets/widget.h)
- [`void/contents/widgets/textfield.h`](../void/include/void/contents/widgets/textfield.h)
- [`void/contents/widgets/list_options.h`](../void/include/void/contents/widgets/list_options.h)
