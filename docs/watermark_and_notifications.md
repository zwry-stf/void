# Watermark & notifications

## Watermark

The watermark system supports “components” that render text in a compact line.

The sample shows two components:

### Average component (refreshed on an interval)

```cpp
instance.watermark().add_avarage_component(
    "fps",
    []() -> float { return 1.f / std::clamp(instance.delta_time(), 0.0001f, 1.f); },
    std::chrono::milliseconds(600)
);
```

### Text component

```cpp
static vo::xstr text_value = "Text";
instance.watermark().add_text_component(&text_value);
```

Accessor header: [`void/accessors/watermark.h`](../void/include/void/accessors/watermark.h)

## Notifications

Notifications are exposed via:
- [`void/accessors/notifications.h`](../void/include/void/accessors/notifications.h)

They support colored segments (useful for highlighting status words).

Example:
```cpp
instance->notifications().create_note()
    << style.text() << xstr("Text");
    
instance->notifications().create_error()
    << style.text() << xstr("Error");
```