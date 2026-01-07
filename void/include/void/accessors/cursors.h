#pragma once
#include <void/util/vobj.h>
#include <atomic>


void_begin_

enum class cursor : std::uint8_t {
    arrow,
    hand,
    text,
    size_ns,
    size_ew,
    size_nesw,
    size_nwse,
    size_all
};

class cursors : protected vobj {
private:
    cursor current_;
    std::atomic<cursor> current_synced_;

public:
    cursors(void_* instance);

public:
    void set_cursor(cursor c) noexcept {
        current_ = c;
    }
    void update() noexcept {
        current_synced_.store(current_, std::memory_order_release);
    }
    [[nodiscard]] cursor get_cursor() const noexcept {
        return current_synced_.load(std::memory_order_acquire);
    }

#if defined(VOID_HAS_GLFW)
    [[nodiscard]] int get_cursor_glfw() const noexcept;
#endif // VOID_HAS_GLFW
#if defined(R2_PLATFORM_WINDOWS)
    [[nodiscard]] wchar_t* get_cursor_win32() const noexcept;
#endif // R2_PLATFORM_WINDOWS
};

void_end_