# Style & background

Two central accessors drive look-and-feel:

- `instance.style()` — color palette + scalar style values
- `instance.background()` — blur/noise/background blending controls

The sample builds a UI for these in the “Advanced” child tab.

## Typical pattern

```cpp
auto& style = instance.style();
auto& background = instance.background();

group.colorpicker("Accent", style.accent())->no_config();
group.toggle("Background Blur", background.blur_enabled())->no_config();
group.slider("Blend Amount", background.blend_amount())->no_config();
```

## Scalable floats (`sfloat`)

Many values in style/background use a “scalable float” type so the UI can scale with DPI.
You will often see `.raw()` access to the underlying float for binding a slider.

Header: [`void/util/scalable_float.h`](../void/include/void/util/scalable_float.h)
