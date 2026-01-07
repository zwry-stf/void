#pragma once
#include <string>


namespace clipboard {
    bool copy_to_clipboard(const std::wstring& s);
    bool get_clipboard(std::wstring& s);
}