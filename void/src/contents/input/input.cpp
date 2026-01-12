#include <void/contents/input/input.h>
#include <void/void.h>
#include "keybind_manager.h"

#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

#if defined(VOID_HAS_GLFW)
#include <GLFW/glfw3.h>
#endif


void_begin_

input::input(void_* instance) noexcept
    : vobj(instance),
      last_mouse_x_(0.0f),
      last_mouse_y_(0.0f)
{
    key_states_.reset();
    mouse_states_.reset();

    keybind_manager_ = std::make_unique<keybind_manager>(instance);
}

input::~input() = default;

input_response input::process_event(const message_event& event)
{
#if defined(_DEBUG)
    instance()->renderer().assert_render_thread();
#endif
    if (event.is_message(message_type::key_down)) {
        key_states_[static_cast<std::size_t>(event.get_key())] = true;
    }

    if (event.is_message(message_type::key_up)) {
        key_states_[static_cast<std::size_t>(event.get_key())] = false;
    }

    if (event.is_message(message_type::mouse_button_down)) {
        mouse_states_[static_cast<std::size_t>(event.get_mouse_button())] = true;
    }

    if (event.is_message(message_type::mouse_button_up)) {
        mouse_states_[static_cast<std::size_t>(event.get_mouse_button())] = false;
    }

    if (!instance()->is_initialized())
        return input_response::empty();

    if (event.is_message(message_type::none) ||
        event.is_message(message_type::ignore)) {
        return input_response::empty();
    }

    return instance()->on_input(event);
}

input_response input::push_event(const message_event& event)
{
    if (!event.is_message(message_type::ignore) &&
        !event.is_message(message_type::none)) {
        {
            std::lock_guard<std::mutex> lock(messages_mutex_);
            pending_messages_.push_back(event);
        }

        if (instance()->is_open() &&
            instance()->options().get<options::option_BlockInput>()) {
            return input_response::handled();
        }
    }

    return input_response::empty();
}

void input::process_pending_events()
{
    std::vector<message_event> local_events;
    {
        std::lock_guard<std::mutex> lock(messages_mutex_);
        pending_messages_.swap(local_events);
    }

    for (const auto& e : local_events) {
        process_event(e);
    }
}

void input::input_on_frame()
{
    assert(instance()->is_initialized());

    instance()->reset_hovered_state();
    instance()->cursors().set_cursor(cursor::arrow);

    if (instance()->is_open()) {
#if defined(R2_PLATFORM_WINDOWS)
        update_key_flags_win32();
        input_on_frame_win32();
#endif
    }

    if (instance()->is_open()) {
        process_pending_events();
    }
    else {
        std::vector<message_event> local_events;
        {
            std::lock_guard<std::mutex> lock(messages_mutex_);
            pending_messages_.swap(local_events);
        }
        for (auto& e : local_events) {
            if (e.is_message(message_type::key_down) &&
                e.get_key() == instance()->options().get<options::option_MenuKey>()) {
                instance()->toggle_menu(!instance()->is_open());
            }
        }
    }

    keybind_manager_->update();
}

keybind_owner* input::add_keybind(keybind* bind, key default_key, keybind_mode mode)
{
    return keybind_manager_->add_keybind(bind, default_key, mode);
}

void input::clear_queue()
{
    std::lock_guard<std::mutex> lock(messages_mutex_);
    pending_messages_.clear();
}

int input::get_scan_code(key key)
{
#if defined(R2_PLATFORM_WINDOWS)
    return get_scan_code_win32(key);
#else
    assert(false && "no implementation called");
    return 0;
#endif
}

#pragma region(Win32)
#if defined(R2_PLATFORM_WINDOWS)
input_response input::input_win32(std::uint32_t msg, uint64_t wparam, int64_t lparam)
{
    if (!instance()->is_initialized())
        return input_response::empty();

    message_event event = convert_message_win32(msg, wparam, lparam);
    return push_event(event);
}

inline static key convert_key_win32(std::int32_t vk) {
    if (vk >= 'A' && vk <= 'Z') {
        int offset = vk - 'A';
        return static_cast<key>(static_cast<int>(key::a) + offset);
    }

    if (vk >= '0' && vk <= '9') {
        int offset = vk - '0';
        return static_cast<key>(static_cast<int>(key::n0) + offset);
    }

    if (vk >= VK_F1 && vk <= VK_F12) {
        int offset = vk - VK_F1;
        return static_cast<key>(static_cast<int>(key::f1) + offset);
    }

    switch (vk) {
    case VK_UP:    return key::up;
    case VK_DOWN:  return key::down;
    case VK_LEFT:  return key::left;
    case VK_RIGHT: return key::right;

    case VK_SHIFT: [[fallthrough]];
    case VK_LSHIFT:   return key::lshift;
    case VK_RSHIFT:   return key::rshift;
    case VK_CONTROL: [[fallthrough]];
    case VK_LCONTROL: return key::lctrl;
    case VK_RCONTROL: return key::rctrl;
    case VK_LMENU:    return key::lalt;
    case VK_RMENU:    return key::ralt;

    case VK_RETURN:  return key::enter;
    case VK_BACK:    return key::backspace;
    case VK_ESCAPE:  return key::escape;
    case VK_TAB:     return key::tab;
    case VK_SPACE:   return key::space;

    case VK_INSERT:  return key::insert;
    case VK_DELETE:  return key::del;
    case VK_HOME:    return key::home;
    case VK_END:     return key::end;
    }

    return key::none;
}

message_event input::convert_message_win32(std::uint32_t msg, uint64_t wparam, int64_t lparam)
{
    const r2::vec2 render_size = instance()->renderer().get_render_size();
    auto rwindow_size = instance()->util().get_window_size();
    if (!rwindow_size.has_value())
        return message_event::none();
    const r2::vec2 window_size = rwindow_size.value();

    const r2::vec2 render_window_scale = render_size / window_size;

    const std::int32_t mouse_x = static_cast<std::int32_t>(LOWORD(lparam) * render_window_scale.x);
    const std::int32_t mouse_y = static_cast<std::int32_t>(HIWORD(lparam) * render_window_scale.y);

    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        return message_event::from_mouse_button(
            msg == WM_LBUTTONDOWN,
            static_cast<float>(mouse_x),
            static_cast<float>(mouse_y),
            mouse_button::left
        );

    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        return message_event::from_mouse_button(
            msg == WM_RBUTTONDOWN,
            static_cast<float>(mouse_x),
            static_cast<float>(mouse_y), 
            mouse_button::right
        );

    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        return message_event::from_mouse_button(
            msg == WM_MBUTTONDOWN,
            static_cast<float>(mouse_x),
            static_cast<float>(mouse_y), 
            mouse_button::middle
        );

    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    {
        mouse_button btn = (HIWORD(wparam) == XBUTTON1) ? mouse_button::xbutton1 : mouse_button::xbutton2;
        return message_event::from_mouse_button(
            msg == WM_XBUTTONDOWN,
            static_cast<float>(mouse_x), 
            static_cast<float>(mouse_y), 
            btn
        );
    }

    case WM_MOUSEMOVE:
        return message_event::ignore();

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        return message_event::from_key(
            true,
            convert_key_win32(static_cast<std::int32_t>(wparam))
        );

    case WM_KEYUP:
    case WM_SYSKEYUP:
        return message_event::from_key(
            false,
            convert_key_win32(static_cast<std::int32_t>(wparam))
        );

    case WM_CHAR:
        return message_event::from_char(static_cast<std::uint32_t>(wparam));

    case WM_MOUSEWHEEL:
    {
        POINT point;
        point.x = static_cast<LONG>(LOWORD(lparam));
        point.y = static_cast<LONG>(HIWORD(lparam));

        HWND hwnd = instance()->renderer().context()->get_hwnd();
        if (ScreenToClient(hwnd, &point)) {
            float px = static_cast<float>(point.x) * render_window_scale.x;
            float py = static_cast<float>(point.y) * render_window_scale.y;

            const int16_t scroll = static_cast<int16_t>(HIWORD(wparam));
            return message_event::from_scroll(
                px, py,
                static_cast<float>(scroll) / float(WHEEL_DELTA)
            );
        }
        break;
    }
    }

    if (msg == WM_SYSCOMMAND && wparam == SC_KEYMENU && (lparam >> 16) <= 0) {
        return message_event::from_key(true, key::lalt);
    }

    return message_event::none();
}

void input::input_on_frame_win32()
{
    assert(instance()->is_initialized());

    HWND hwnd = instance()->renderer().context()->get_hwnd();

    POINT pt;
    if (!GetCursorPos(&pt))
        return;

    if (!ScreenToClient(hwnd, &pt))
        return;

    const r2::vec2 render_size = instance()->renderer().get_render_size();
    auto rwindow_size = instance()->util().get_window_size();
    if (!rwindow_size.has_value())
        return;

    const r2::vec2 window_size = rwindow_size.value();
    const r2::vec2 scale = render_size / window_size;

    float mx = static_cast<float>(pt.x) * scale.x;
    float my = static_cast<float>(pt.y) * scale.y;

    RECT rect;
    bool in_window = false;
    if (GetClientRect(hwnd, &rect)) {
        in_window =
            pt.x >= 0 && pt.y >= 0 &&
            pt.x < rect.right && pt.y < rect.bottom;
    }

    last_mouse_x_ = mx;
    last_mouse_y_ = my;

    message_event ev = message_event::from_mousemove(mx, my, in_window);
    process_event(ev);
}

void input::update_key_flags_win32()
{
    for (std::size_t i = 0u; i < key_states_.count(); i++) {
        if (!key_states_[i])
            continue;

        int vk = get_virtual_key_win32(static_cast<key>(i));
        assert(vk != 0);

        key_states_[i] = GetAsyncKeyState(vk) & 0x8000;
    }

    for (std::size_t i = 0u; i < mouse_states_.count(); i++) {
        if (!mouse_states_[i])
            continue;

        int vk = get_virtual_key_win32(static_cast<mouse_button>(i));
        assert(vk != 0);

        mouse_states_[i] = GetAsyncKeyState(vk) & 0x8000;
    }
}

int input::get_scan_code_win32(key key)
{
    int vk = get_virtual_key_win32(key);
    if (vk == 0)
        return vk;

    return static_cast<int>(MapVirtualKeyA(static_cast<UINT>(vk), MAPVK_VK_TO_VSC));
}

int input::get_virtual_key_win32(key key)
{
    if (key == key::none)
        return 0;

    if (key >= key::a &&
        key <= key::z) {
        return 'A' + (static_cast<int>(key) - static_cast<int>(key::a));
    }

    if (key >= key::n0 &&
        key <= key::n9) {
        return '0' + (static_cast<int>(key) - static_cast<int>(key::n0));
    }

    if (key >= key::f1 &&
        key <= key::f12) {
        return VK_F1 + (static_cast<int>(key) - static_cast<int>(key::f1));
    }

    switch (key) {
        // Navigation
    case key::up:    return VK_UP;
    case key::down:  return VK_DOWN;
    case key::left:  return VK_LEFT;
    case key::right: return VK_RIGHT;

        // Modifiers
    case key::lshift: return VK_LSHIFT;
    case key::rshift: return VK_RSHIFT;
    case key::lctrl:  return VK_LCONTROL;
    case key::rctrl:  return VK_RCONTROL;
    case key::lalt:   return VK_LMENU;
    case key::ralt:   return VK_RMENU;

        // Confirm/cancel
    case key::enter:     return VK_RETURN;
    case key::backspace: return VK_BACK;
    case key::escape:    return VK_ESCAPE;
    case key::tab:       return VK_TAB;
    case key::space:     return VK_SPACE;

        // System/navigation
    case key::insert: return VK_INSERT;
    case key::del:    return VK_DELETE;
    case key::home:   return VK_HOME;
    case key::end:    return VK_END;
    }

    return 0;
}

int input::get_virtual_key_win32(mouse_button key)
{
    switch (key) {
    case mouse_button::left:     return VK_LBUTTON;
    case mouse_button::right:    return VK_RBUTTON;
    case mouse_button::middle:   return VK_MBUTTON;
    case mouse_button::xbutton1: return VK_XBUTTON1;
    case mouse_button::xbutton2: return VK_XBUTTON2;
    }

    return 0;
}

#endif // R2_PLATFORM_WINDOWS
#pragma endregion

#pragma region(Glfw)
#if defined(VOID_HAS_GLFW)

inline static key convert_key_glfw(int key) {
    if (key >= GLFW_KEY_A &&
        key <= GLFW_KEY_Z) {
        int offset = key - GLFW_KEY_A;
        return static_cast<vo::key>(static_cast<int>(key::a) + offset);
    }

    if (key >= GLFW_KEY_0 && 
        key <= GLFW_KEY_9) {
        int offset = key - GLFW_KEY_0;
        return static_cast<vo::key>(static_cast<int>(key::n0) + offset);
    }

    if (key >= GLFW_KEY_F1 &&
        key <= GLFW_KEY_F12) {
        int offset = key - GLFW_KEY_F1;
        return static_cast<vo::key>(static_cast<int>(key::f1) + offset);
    }

    switch (key) {
    case GLFW_KEY_UP:    return key::up;
    case GLFW_KEY_DOWN:  return key::down;
    case GLFW_KEY_LEFT:  return key::left;
    case GLFW_KEY_RIGHT: return key::right;

    case GLFW_KEY_LEFT_SHIFT:    return key::lshift;
    case GLFW_KEY_RIGHT_SHIFT:   return key::rshift;
    case GLFW_KEY_LEFT_CONTROL:  return key::lctrl;
    case GLFW_KEY_RIGHT_CONTROL: return key::rctrl;
    case GLFW_KEY_LEFT_ALT:      return key::lalt;
    case GLFW_KEY_RIGHT_ALT:     return key::ralt;

    case GLFW_KEY_ENTER:     return key::enter;
    case GLFW_KEY_BACKSPACE: return key::backspace;
    case GLFW_KEY_ESCAPE:    return key::escape;
    case GLFW_KEY_TAB:       return key::tab;
    case GLFW_KEY_SPACE:     return key::space;

    case GLFW_KEY_INSERT:  return key::insert;
    case GLFW_KEY_DELETE:  return key::del;
    case GLFW_KEY_HOME:    return key::home;
    case GLFW_KEY_END:     return key::end;
    }

    return key::none;
}

inline static mouse_button convert_mouse_button_glfw(int button) {
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:   return mouse_button::left;
    case GLFW_MOUSE_BUTTON_RIGHT:  return mouse_button::right;
    case GLFW_MOUSE_BUTTON_MIDDLE: return mouse_button::middle;
    default:
        return (button == GLFW_MOUSE_BUTTON_4) ? mouse_button::xbutton1 : mouse_button::xbutton2;
    }
}

input_response input::input_glfw_key(int key, int /* scancode */, int action, int /* mods */)
{
    if (action != GLFW_PRESS && action != GLFW_RELEASE && action != GLFW_REPEAT)
        return input_response::empty();

    bool down = (action == GLFW_PRESS || action == GLFW_REPEAT);
    vo::key k = convert_key_glfw(key);

    message_event ev = message_event::from_key(down, k);
    return push_event(ev);
}

input_response input::input_glfw_char(std::uint32_t codepoint)
{
    message_event ev = message_event::from_char(static_cast<std::uint32_t>(codepoint));
    return push_event(ev);
}

input_response input::input_glfw_mouse_button(int button, int action, int mods)
{
    (void)mods;

    bool down = (action == GLFW_PRESS);
    mouse_button mb = convert_mouse_button_glfw(button);

    message_event ev = message_event::from_mouse_button(down, last_mouse_x_, last_mouse_y_, mb);
    return push_event(ev);
}

input_response input::input_glfw_scroll(double /* xoffset */, double yoffset)
{
    if (yoffset == 0.0)
        return input_response::empty();

    float mx = last_mouse_x_;
    float my = last_mouse_y_;

    message_event ev = message_event::from_scroll(mx, my, static_cast<float>(yoffset));
    return push_event(ev);
}

#endif // VOID_HAS_GLFW
#pragma endregion

void_end_