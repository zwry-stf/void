# Builder overview

Void’s UI is built by describing a tree of content with a fluent API:

- **labels** (top-level separators / labels)
- **tabs**
  - **child tabs**
    - **groups** (left/right)
      - **widgets**

The canonical example is in [`TestRun/main.cpp`](../TestRun/main.cpp) (`add_widgets()` function).

![Fullscreen preview](assets/fullscreen_example_tab.png)

## Entry point

```cpp
auto b = instance.get_builder();
b.label("Label 1");

auto tab = b.tab("Tab 1");
tab.icon(void_resources::color_png);

auto child = tab.child("Child 1");
child.left_group("Group left")
    .toggle("Toggle 1", some_bool)
    .slider("Slider 1", some_float);
```

## Tabs

`builder::tab` creates a top-level tab.
You can also add the built-in system tabs:

```cpp
tab.config_tab("Config").icon(void_resources::config_png);
tab.theme_tab("Theme").icon(void_resources::theme_png);
```

## Child tabs

Child tabs are the content pages inside a tab.
They can also have icons:

```cpp
auto child = tab.child("Advanced");
child.icon(void_resources::child_png);
```

## Groups

Each child tab has two columns: `left_group()` and `right_group()`.

Groups are the “layout containers” that widgets are appended to.

```cpp
child.left_group("Background")
    .colorpicker("Background", style.background())->no_config();
```

## Fluent chaining model

Most widget creation calls return a widget pointer (or a small proxy) so you can configure behavior:

```cpp
group.toggle("Toggle 1", bool_value)
    ->disabled(other_bool)
    ->colorpicker(color_value);
```

Chaining can also “jump” into the last created nested container (childwindow/keybind/etc.)
The sample uses helper calls like `last_childwindow()` or `last_keybind()` for that.
