# Themes

Void supports theming (colors + key style scalars). The style and theme accessors are:

- `instance.style()`
- `instance.theme()`

The built-in theme tab is inserted with:

```cpp
tab.theme_tab("Theme").icon(void_resources::theme_png);
```

## What a theme affects

At minimum:
- color palette (accent, highlight, border, text, etc.)
- scalar values (rounding, spacing, overlay size)
- background settings may also be driven via theme depending on your config

## Live tweaking

The sample builds an “Advanced” tab that directly edits style values and background settings,
but marks them `no_config()` so they do not persist unless you explicitly save via config mechanisms.
