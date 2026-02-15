# Troubleshooting

## Nothing renders / menu not visible
- Ensure you call `void_::init(...)` successfully (catch `r2::error` like the sample).
- Confirm your backend init data matches your backend define (D3D11 swapchain for D3D11).
- Confirm you link both `void` and `resources`.

## Dropdown/multiselect shows garbage / crashes
`vo::list_options` can point at dynamic storage.
Make sure the underlying array/vector outlives the widget and any open popup.

## Textfield input doesn’t work
You must forward both:
- `input_glfw_char` (text input)
- `input_glfw_key` (key events)

Forwarding only key events will break normal text input.

## Overlay can’t be dragged/resized
Check that you didn’t call:
- `.make_movable(false)`
- `.make_resizable(false)`

## Blur/glass looks wrong or is disabled
Some builds can disable blur via options (see `option_NoBlur` and downsample-related options).
The sample also disables MSAA / full-frame update via options for testing.
