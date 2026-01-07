#include "clipboard.h"

#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif


bool clipboard::copy_to_clipboard(const std::wstring& s)
{
#if defined(R2_PLATFORM_WINDOWS)
    if (!OpenClipboard(nullptr))
        return false;

    if (!EmptyClipboard()) {
        CloseClipboard();
        return false;
    }

    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE,
        static_cast<SIZE_T>((s.length() + 1u) * sizeof(wchar_t)));
    if (!mem) {
        CloseClipboard();
        return false;
    }

    wchar_t* wbuf = static_cast<wchar_t*>(GlobalLock(mem));
    if (!wbuf) {
        GlobalFree(mem);
        CloseClipboard();
        return false;
    }

    std::memcpy(
        wbuf,
        s.c_str(),
        s.size() * sizeof(*wbuf)
    );
    wbuf[s.length()] = L'\0';

    GlobalUnlock(mem);

    if (!SetClipboardData(CF_UNICODETEXT, mem)) {
        GlobalFree(mem);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
#endif // R2_PLATFORM_WINDOWS
}

bool clipboard::get_clipboard(std::wstring& s)
{
#if defined(R2_PLATFORM_WINDOWS)
    if (!OpenClipboard(nullptr))
        return false;

    HANDLE mem = GetClipboardData(CF_UNICODETEXT);
    if (!mem) {
        CloseClipboard();
        return false;
    }

    wchar_t* text = static_cast<wchar_t*>(GlobalLock(mem));
    if (!text) {
        CloseClipboard();
        return false;
    }

    s.clear();

    while (*text != '\0') {
        s += *text;
        text++;
    }

    GlobalUnlock(mem);
    CloseClipboard();

    return true;
#endif // R2_PLATFORM_WINDOWS
}
