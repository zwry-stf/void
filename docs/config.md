# Config

Void has a built-in config system. The general idea:

- Widgets created through the builder can persist values automatically.
- Widgets can opt out with `->no_config()`.
- Overlays can have independent config namespaces via `.config("name")`.

![Config tab](assets/config_tab_menu.png)

## Built-in config tab

`tab.config_tab("Config")` inserts the built-in config UI.
In the sample:

```cpp
tab.config_tab("Config").icon(void_resources::config_png);
```

## Disabling persistence for a widget

```cpp
group.colorpicker("Background", style.background())
    ->no_config();
```

This is commonly used for “live tweak” UI that should not serialize into a profile.

## Overlay configs

Custom overlays can store their position/size and other state under a config key:

```cpp
b.overlay().config("test_overlay");
```

## Custom modules

There are accessors in `void/accessors/config.h` for adding custom config modules.
Use this when you need to serialize state that is not represented by a widget value.

Public header: [`void/include/void/accessors/config.h`](../void/include/void/accessors/config.h)
