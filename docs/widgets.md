# Widgets

This page documents the widgets visible in the public builder API, and the most common “option chains” you’ll use.

> Source: public headers in `void/include/void/builder/*` and sample usage in `TestRun/main.cpp`.

## Common widget modifiers

Most widgets share these modifiers:

- `disabled(bool on)` / `disabled_inverted(bool condition)`
- `condition(std::function<bool()>)` — only render/enable under a predicate
- `no_config()` — opt out of config persistence for that widget
- `faded(bool)` — render in a subdued state (used e.g. by `textfield` in the sample)

The exact set depends on the widget type; the sample demonstrates which ones are supported.

## Toggle

```cpp
group.toggle("Toggle", value)
    ->disabled(other)
    ->disabled_inverted(condition);
```

Toggles can have “child widgets” attached:

```cpp
group.toggle("Toggle 1", bool_value)
    ->colorpicker(color_value)
    ->multiselect(options, multiselect_value);
```

## Slider

```cpp
group.slider("Slider", float_value, min, max, "{:.1f}")
    ->decimal_count(1);
```

## Button

```cpp
group.button("Button", "Click", []() { /* ... */ });
```

## Dropdown

Dropdown expects a `vo::list_options` provider and a selected index (`std::size_t`):

```cpp
group.dropdown("Dropdown", vo::list_options::create_constant(kOptions), index);
```

For dynamic data:

```cpp
group.dropdown("Dropdown",
    vo::list_options::create_vector_dynamic(&options_dynamic),
    index);
```

See also: [List options](#list-options).

## Multi-select

```cpp
group.multiselect(options, multiselect_value);
```

Where `multiselect_value` is `std::vector<bool>` sized to match the options.

## Textfield

Textfield is UTF-32 based in its callback (works well with IME / unicode input).

```cpp
group.textfield("Textfield",
    [](const std::u32string& s) { /* ... */ })
  ->max_length(30)
  ->default_text("Default text")
  ->faded(true);
```

## Color picker

```cpp
group.colorpicker("Accent", style.accent())->no_config();
```

Optional variant:

```cpp
widget->optional_colorpicker(color_value, enabled_bool);
```

## Childwindow

A childwindow is a nested group-like container attached to a widget:

```cpp
group.childwindow("Childwindow")
    ->toggle("Toggle", value)
    ->slider("Slider", float_value, 0.f, 50.f);
```

To continue chaining back to the parent, the sample uses `last_childwindow()`.

## Keybind

Keybind widgets bind a `vo::keybind` instance.

```cpp
group.keybind(g_test_keybind)
    ->key(vo::key::f7)
    ->disabled(bool_value);
```

## List options

`vo::list_options` is an abstraction over “a list of strings”. It supports:

- constant arrays (`create_constant`)
- dynamic vectors (`create_vector_dynamic`)

Important lifetime rule:
- the option storage must outlive the widget and any open dropdown/multiselect using it.

Public header: [`void/contents/widgets/list_options.h`](../void/include/void/contents/widgets/list_options.h)
