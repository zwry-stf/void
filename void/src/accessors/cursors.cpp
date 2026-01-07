#include <void/accessors/cursors.h>

#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif // R2_PLATFORM_WINDOWS
#if defined(VOID_HAS_GLFW)
#include <GLFW/glfw3.h>
#endif // VOID_HAS_GLFW


void_begin_

cursors::cursors(void_* instance)
    : vobj(instance),
      current_(cursor::arrow),
      current_synced_(current_)
{
}

#if defined(VOID_HAS_GLFW)
int cursors::get_cursor_glfw() const noexcept
{
    auto c = get_cursor();

    switch (c) {
    case cursor::arrow:
        return GLFW_ARROW_CURSOR;
    case cursor::hand:
        return GLFW_HAND_CURSOR;
    case cursor::text:
        return GLFW_IBEAM_CURSOR;
    case cursor::size_ns:
        return GLFW_RESIZE_NS_CURSOR;
    case cursor::size_ew:
        return GLFW_RESIZE_EW_CURSOR;
    case cursor::size_nesw:
        return GLFW_RESIZE_NESW_CURSOR;
    case cursor::size_nwse:
        return GLFW_RESIZE_NWSE_CURSOR;
    case cursor::size_all:
        return GLFW_RESIZE_ALL_CURSOR;
    }
    return 0;
}
#endif // VOID_HAS_GLFW

#if defined(R2_PLATFORM_WINDOWS)
wchar_t* cursors::get_cursor_win32() const noexcept
{
    auto c = get_cursor();

    switch (c) {
    case cursor::arrow:
        return IDC_ARROW;
    case cursor::hand:
        return IDC_HAND;
    case cursor::text:
        return IDC_IBEAM;
    case cursor::size_ns:
        return IDC_SIZENS;
    case cursor::size_ew:
        return IDC_SIZEWE;
    case cursor::size_nesw:
        return IDC_SIZENESW;
    case cursor::size_nwse:
        return IDC_SIZENWSE;
    case cursor::size_all:
        return IDC_SIZEALL;
    }
    return 0;
}
#endif // R2_PLATFORM_WINDOWS

void_end_