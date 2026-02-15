# Resources (icons/fonts/shaders)

Void ships a separate `resources` static library that provides:
- icons used by the UI (see `void_resources::...` in the sample)
- embedded fonts
- shaders needed for effects (blur, glass, etc.)

You normally do **not** need to interact with this directly—just link the library.

Public headers:
- [`resources/include/resources/resources.h`](../resources/include/resources/resources.h)
- [`void/accessors/resources.h`](../void/include/void/accessors/resources.h)
- [`void/accessors/icons.h`](../void/include/void/accessors/icons.h)
- [`void/accessors/fonts.h`](../void/include/void/accessors/fonts.h)

**Note**
Resources get compiled into a binary array.
You will need to rebuild this array when switching backends or platforms.

Usage (Windows):
`resources_(x64_or_x86) -compile`
> inside [`resources/internal/res`](../resources/internal/res/)