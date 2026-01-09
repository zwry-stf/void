#include <string>
#include <thread>
#include <atomic>
#include <format>
#include <utility>
#include <memory>
#include <format>
#include <resources/resources.h>


// platform dependent includes
#if defined(R2_PLATFORM_WINDOWS)
#define NOMINMAX
#endif

#if defined(R2_BACKEND_D3D11)
#include <backend/d3d11/texture2d.h>
#endif

#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#endif
#include <GLFW/glfw3.h>

#if defined(R2_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include <r2/renderer.h>
#include <r2/error.h>
#include <void/void.h>
#include <void/builder/builder.h>
#include <void/contents/overlays/custom_overlay.h>


// Global data
struct GlobalRenderData {
#if defined(R2_BACKEND_D3D11)
    d3d_pointer<IDXGISwapChain> swapchain;
    d3d_pointer<ID3D11Device> device;
    d3d_pointer<ID3D11DeviceContext> context;
#endif
};

struct WindowData
{
    int fb_width{};
    int fb_height{};
    GLFWwindow* window{};
    std::atomic<bool> needs_resize{ false };
    int pending_resize_width{};
    int pending_resize_height{};
};

struct GlobalData {
    std::atomic<bool> running{ false };
    WindowData window_data;
    GlobalRenderData render_data;
};

inline static GlobalData g_data;

inline static vo::void_* g_void = &vo::get();


// function def
bool create_window(const std::string& title);
bool initialize_backend();
void add_widgets();
template<typename... Args>
void show_error_and_exit(std::format_string<Args...> f, Args&&... args);
void on_resize(GLFWwindow* window, int width, int height);
void render_thread();
void destroy_backend();
void destroy_window();


// main
#if defined(R2_PLATFORM_WINDOWS)
int __stdcall WinMain(HINSTANCE /* instance */,
            HINSTANCE /* prev instance */,
            LPSTR /* cmd line */,
            int /* show/hide */)
{
#endif

    if (!create_window("void - v1"))
        show_error_and_exit("failed to create window");

    if (!initialize_backend())
        show_error_and_exit("failed to initialize backend");

    // void
#if defined(R2_BACKEND_D3D11)
    r2::backend_init_data binit(g_data.render_data.swapchain.get());
#elif defined(R2_BACKEND_OPENGL)
    r2::backend_init_data binit;
#endif

#if defined(R2_PLATFORM_WINDOWS)
    r2::platform_init_data pinit(glfwGetWin32Window(g_data.window_data.window));
#endif

    g_void->options().set<vo::options::option_FullFrameUpdate>(false);
    g_void->options().set<vo::options::option_UpdateFrameTime>(false);
    g_void->options().set<vo::options::option_MenuMSAA>(false);

    add_widgets();

    try {
        g_void->init(pinit, binit);
    }
    catch (const r2::error& e) {
        show_error_and_exit("renderer initialization failed: {}", e.to_string());
    }

    // run
    glfwShowWindow(g_data.window_data.window);

#if defined(R2_BACKEND_OPENGL)
    glfwMakeContextCurrent(nullptr);
#endif

    g_data.running.store(true, std::memory_order_release);

    std::thread t(render_thread);

    while (g_data.running.load(std::memory_order_acquire)) {
        glfwPollEvents();

        if (glfwWindowShouldClose(g_data.window_data.window))
            break;

        int cursor = g_void->cursors().get_cursor_glfw();
        
        static std::vector<std::pair<int, GLFWcursor*>> cursors;
        GLFWcursor* glfw_cursor = nullptr;
        for (auto& c : cursors) {
            if (c.first == cursor) {
                glfw_cursor = c.second;
                break;
            }
        }
        if (glfw_cursor == nullptr) {
            glfw_cursor = glfwCreateStandardCursor(cursor);
            cursors.emplace_back(cursor, glfw_cursor);
        }

        glfwSetCursor(g_data.window_data.window, glfw_cursor);
    }

    g_data.running.store(false, std::memory_order_release);

    if (t.joinable())
        t.join();

    g_void->renderer().destroy();

    destroy_backend();

    destroy_window();

    return 0;
}

// implementations
bool create_window(const std::string& title)
{
    int res = glfwInit();
    if (res != GLFW_TRUE)
        return false;

#if defined(R2_BACKEND_D3D11)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#elif defined(R2_BACKEND_OPENGL)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    g_data.window_data.window = glfwCreateWindow(1400, 900, title.c_str(), nullptr, nullptr);
    if (g_data.window_data.window == nullptr)
        return false;

#if defined(R2_BACKEND_OPENGL)
    glfwMakeContextCurrent(g_data.window_data.window);
#endif

    glfwGetFramebufferSize(g_data.window_data.window,
        &g_data.window_data.fb_width, &g_data.window_data.fb_height);

    glfwSetFramebufferSizeCallback(g_data.window_data.window, on_resize);

    // input callbacks
    glfwSetCharCallback(g_data.window_data.window,
        [](GLFWwindow* /* window */, std::uint32_t codepoint) {
            g_void->input().input_glfw_char(codepoint);
        }
    );

    glfwSetKeyCallback(g_data.window_data.window,
        [](GLFWwindow* /* window */, int key, int scancode, int action, int mods) {
            g_void->input().input_glfw_key(key, scancode, action, mods);
        }
    );

    glfwSetMouseButtonCallback(g_data.window_data.window,
        [](GLFWwindow* /* window */, int button, int action, int mods) {
            g_void->input().input_glfw_mouse_button(button, action, mods);
        }
    );

    glfwSetScrollCallback(g_data.window_data.window,
        [](GLFWwindow* /* window */, double xoffset, double yoffset) {
            g_void->input().input_glfw_scroll(xoffset, yoffset);
        }
    );

    return true;
}

bool initialize_backend()
{
#if defined(R2_BACKEND_D3D11)
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.OutputWindow                       = glfwGetWin32Window(g_data.window_data.window);
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT create_device_flags = 0;
#ifdef _DEBUG
    create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_level_array[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        create_device_flags,
        feature_level_array,
        sizeof(feature_level_array) / sizeof(feature_level_array[0]),
        D3D11_SDK_VERSION,
        &sd,
        g_data.render_data.swapchain.address_of(),
        g_data.render_data.device.address_of(),
        &feature_level,
        g_data.render_data.context.address_of()
    );

    if (res == DXGI_ERROR_UNSUPPORTED) {
        res = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            create_device_flags,
            feature_level_array,
            sizeof(feature_level_array) / sizeof(feature_level_array[0]),
            D3D11_SDK_VERSION,
            &sd,
            g_data.render_data.swapchain.address_of(),
            g_data.render_data.device.address_of(),
            &feature_level,
            g_data.render_data.context.address_of()
        );
    }

    if (FAILED(res))
        return false;
#endif

    return true;
}

void add_widgets()
{
    g_void->watermark().add_avarage_component(
        "fps",
        []() -> float {
            return 1.f / std::clamp(g_void->delta_time(), 0.0001f, 1.f);
        },
        std::chrono::milliseconds(600)
    );

    auto mb = g_void->get_builder();

    static bool overlay_enabled = true;
    mb.overlay()
        .config("test_overlay")
        .liquid_glass(true)
        .make_resizable(true)
        .pos(0.8f, 0.4f)
        .size(200.f, 120.f)
        .min(100.f, 50.f)
        .max(400.f, 300.f)
        .on_render(
            [](vo::void_* instance, vo::custom_overlay& overlay) -> void
            {
                auto& style = instance->style();

                overlay.data().rounding_bottom =
                    overlay.data().rounding_top =
                    instance->style().rounding->get(instance->scale());
                overlay.data().border = instance->style().border();
                overlay.data().background = instance->style().overlay_background().transparent();

                overlay.cfg().liquid_glass_size.raw() = style.spacing->get(instance->scale()) * 2.f;
                overlay.data().liquid_glass_color = r2::color::black().alpha(0.1f);

                constinit static float overlay_animation = 1.f;
                overlay_animation = instance->util().lerp2(
                    overlay_animation, overlay_enabled
                );

                overlay.set_animation(overlay_animation);
            }
        )
        .on_update(
            [](vo::void_* instance, vo::custom_overlay& overlay) -> void
            {
                (void)instance;
                overlay.toggle_input(overlay_enabled);
            }
        );

    using vo::xstr;
    static constexpr xstr kOptions[] = {
        xstr("Option 1"),
        xstr("Option 2"),
        xstr("Option 3"),
        xstr("Option 4"),
        xstr("Option 5"),
        xstr("Option 6")
    };

    static std::vector<xstr> options_dynamic;
    static bool bool_value = false;
    static bool bool_value2 = false;
    static float float_value = 0.f;
    static std::size_t int_value = 0;
    static r2::color color_value = r2::color::cyan();
    static std::vector<bool> multiselect_value;


    mb.label("Label 1");
    auto tab = mb.tab("Tab 1");
    tab.set_icon(void_resources::color_png);

    // child 1
    {
        auto child = tab.child("Child 1");

        child.left_group("Group left")
            .toggle("Toggle 1", bool_value)
                ->disabled(bool_value2)
                ->colorpicker(color_value)
                ->multiselect(vo::list_options::create_constant(kOptions), multiselect_value)
            .toggle("Toggle 2", bool_value2)
                ->disabled_inverted(bool_value)
            .slider("Slider 1", float_value)
                ->decimal_count(1)
            .childwindow("Childwindow")
                ->toggle("Toggle 3", bool_value2)
                    .l()->disabled(bool_value)
                    .l()->dropdown(vo::list_options::create_constant(kOptions), int_value)
                ->slider("Slider 1", float_value, 0.f, 50.f)
                    .l()->decimal_count(0)
                ->button("Button 1", "Click", [](){})
            .colorpicker("Colorpicker 1", color_value)
                ->last_childwindow()
                ->colorpicker(color_value);

        child.right_group("Group right")
            .button("Button", "Add Option",
                []() -> void {
                    options_dynamic.push_back(xstr("Option"));
                })
            .spacing()
            ->condition([]()->bool { return options_dynamic.empty(); })
            .dropdown(
                "Dropdown",
                vo::list_options::create_vector_dynamic(&options_dynamic),
                int_value
            );
    }
}

template<typename ...Args>
void show_error_and_exit(std::format_string<Args...> f, Args && ...args)
{
    std::string msg = std::format(f, std::forward<Args>(args)...);

#if defined(R2_PLATFORM_WINDOWS)
    MessageBoxA(NULL, msg.c_str(), "void - error", MB_OK | MB_ICONERROR);
#endif

    std::abort();
}

void on_resize(GLFWwindow* window, int width, int height)
{
    if (window != g_data.window_data.window)
        return;

    if (g_data.window_data.fb_width == width &&
        g_data.window_data.fb_height == height)
        return;

    if (width == 0 &&
        height == 0)
        return;

    g_data.window_data.pending_resize_width = width;
    g_data.window_data.pending_resize_height = height;

    g_data.window_data.needs_resize.store(true, std::memory_order_release);
}

bool resize(int width, int height)
{
#if defined(R2_BACKEND_D3D11)
    g_data.render_data.context->ClearState();
    g_data.render_data.context->Flush();
#endif

    g_void->pre_resize();

#if defined(R2_BACKEND_D3D11)
    HRESULT hr = g_data.render_data.swapchain->ResizeBuffers(
        0u,
        static_cast<UINT>(width),
        static_cast<UINT>(height),
        DXGI_FORMAT_UNKNOWN,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
    );
    if (FAILED(hr))
        return false;
#endif

    g_void->post_resize();

    g_data.window_data.fb_width = width;
    g_data.window_data.fb_height = height;

    return true;
}

void render_frame()
{
    g_void->renderer().reset_render_data();
    g_void->renderer().backup_render_state();
    g_void->renderer().setup_render_state();
    
    // test
    {
        auto& renderer = g_void->renderer();
        {
            renderer.push_clip_rect(
                r2::rect(
                    1000, 200,
                    1200,
                    static_cast<std::int32_t>(renderer.get_render_size().y)
                )
            );

            renderer.add_quad_filled_multicolor(
                r2::vec2{ 800.f, 100.f },
                r2::vec2{ 800.f, 500.f },
                r2::vec2{ 1300.f, 500.f },
                r2::vec2{ 1300.f, 100.f },
                r2::color::white(),
                r2::color::red(),
                r2::color::blue(),
                r2::color::blue()
            );

            renderer.pop_clip_rect();
        }

        renderer.add_line(r2::vec2(200.f, 200.f), r2::vec2(700.f, 600.f), r2::color::black(), 6.f);
        renderer.add_line(r2::vec2(200.f, 200.f), r2::vec2(700.f, 600.f), r2::color::white(), 4.f);

        renderer.add_line(r2::vec2(200.f, 200.f), r2::vec2(200.f, 600.f), r2::color::black(), 3.f);
        renderer.add_line(r2::vec2(200.f, 200.f), r2::vec2(200.f, 600.f), r2::color::white(), 1.f);

        const r2::vec2 points[] = {
            r2::vec2(500.f, 500.f),
            r2::vec2(800.f, 450.f),
            r2::vec2(780.f, 200.f),
            r2::vec2(600.f, 130.f),
            r2::vec2(450.f, 240.f),
        };

        renderer.add_convex_filled(
            points, sizeof(points) / sizeof(points[0]),
            r2::color::cyan().interp(r2::color::black(), 0.5f).interp(r2::color::white(), 0.3f)
        );

        auto vtx_index = renderer.vertex_ptr();
        renderer.add_shadow_rect_filled(
            r2::vec2(600.f, 400.f),
            r2::vec2(900.f, 600.f),
            r2::color::white(),
            20.f
        );
        renderer.shade_vertices_col(
            vtx_index,
            renderer.vertex_ptr(),
            r2::vec2(500.f, 300.f),
            r2::vec2(1000.f, 700.f),
            r2::color::white(),
            r2::color::blue(),
            r2::color::cyan(),
            r2::color::purple()
        );

        vtx_index = renderer.vertex_ptr();
        renderer.add_rect(
            r2::vec2(600.f, 700.f),
            r2::vec2(900.f, 900.f),
            r2::color::white().interp(r2::color::black(), 0.5f),
            2.f,
            20.f,
            r2::e_rounding_flags::rounding_top | r2::e_rounding_flags::rounding_bottomright
        );
        renderer.shade_vertices_col(
            vtx_index,
            renderer.vertex_ptr(),
            r2::vec2(500.f, 600.f),
            r2::vec2(1000.f, 1000.f),
            r2::color::white(),
            r2::color::blue(),
            r2::color::cyan(),
            r2::color::purple()
        );

        renderer.add_quad_filled(
            r2::vec2(300.f, 300.f),
            r2::vec2(400.f, 700.f),
            r2::vec2(1000.f, 800.f),
            r2::vec2(900.f, 400.f),
            (r2::color::green() + r2::color::blue()).interp(r2::color::black(), 0.5f).alpha(0.2f)
        );

        auto test_str = std::u8string_view(u8"Hello World! abcikawhfioawhf");
        float width = renderer.get_text_width(test_str);
        (void)width;
        renderer.add_text_faded(
            r2::vec2(500.f, 300.f),
            r2::color::blue().interp(r2::color::white(), 0.4f).interp(r2::color::green(), 0.3f),
            r2::color::red(),
            500.f, 800.f,
            test_str,
            true
        );

        renderer.add_text(
            r2::vec2(300.f, 300.f),
            r2::color::blue().interp(r2::color::white(), 0.4f).interp(r2::color::green(), 0.3f),
            std::u8string_view(u8"Ä*+**''Ä")
        );
    }

    // fps 
    {
        constexpr float kUpdateTime = 0.5f;
        static int counted = 0;
        static float current = 0.f;
        static std::chrono::steady_clock::time_point last_update;

        auto time_now = std::chrono::steady_clock::now();
        const float elapsed = std::chrono::duration<float>(time_now - last_update).count();
        if (elapsed > kUpdateTime) {
            current = static_cast<float>(counted) / elapsed;

            counted = 0;
            last_update = time_now;
        }

        counted++;

        auto s = std::format("{:.2f}", current);

        g_void->renderer().add_text(
            r2::vec2(10.f, 10.f),
            r2::color::cyan(),
            s
        );
    }

    if (GetAsyncKeyState(VK_F7) & 0x8000) {
        g_void->renderer().add_text(
            r2::vec2(750.f, 200.f),
            r2::color::yellow(),
            std::u8string_view(u8"→😭😂🤓😘→")
        );

        g_void->set_scale(2.f);
    }

    if (GetAsyncKeyState(VK_F8) & 0x8000) {
        g_void->renderer().add_text(
            r2::vec2(750.f, 200.f),
            r2::color::yellow(),
            std::u8string_view(u8"💔💔🤑🐒")
        );

        g_void->set_scale_auto();

        g_void->notifications().create_note() << vo::xstr("Test") << r2::color::red() << vo::xstr("Test Red");
    }

    g_void->renderer().render();
    g_void->update_frame_time();
    g_void->render();
    g_void->renderer().update_fonts_on_frame();
}

void render_thread()
{
#if defined(R2_BACKEND_OPENGL)
    glfwMakeContextCurrent(g_data.window_data.window);
#endif 

#if defined(_DEBUG)
    g_void->renderer().set_render_thread(std::this_thread::get_id());
#endif

    while (g_data.running.load(std::memory_order_acquire)) {
        // resize
        if (g_data.window_data.needs_resize.load(std::memory_order_acquire)) {
            bool ret = resize(
                g_data.window_data.pending_resize_width,
                g_data.window_data.pending_resize_height
            );

            if (!ret) {
                g_data.running.store(false, std::memory_order_release);
                return;
            }

            g_data.window_data.needs_resize.store(false, std::memory_order_release);
        }

        // viewport
        g_void->render_target().bind_main();

        g_void->renderer().context()->set_viewport({
            0.f, 0.f,
            static_cast<float>(g_data.window_data.fb_width),
            static_cast<float>(g_data.window_data.fb_height)
            }
        );
        
        // clear
        g_void->renderer().context()->clear_framebuffer(
            g_void->render_target().main_fbo());

        // render
        render_frame();

        // present
#if defined(R2_BACKEND_D3D11)
        g_data.render_data.swapchain->Present(0u, DXGI_PRESENT_ALLOW_TEARING);
#elif defined(R2_BACKEND_OPENGL)
        glfwSwapBuffers(g_data.window_data.window);
#endif
    }

    g_data.running.store(false, std::memory_order_release);
}

void destroy_backend()
{
#if defined(R2_BACKEND_D3D11)
    if (g_data.render_data.context) {
        g_data.render_data.context->ClearState();
        g_data.render_data.context->Flush();

        g_data.render_data.context.reset();
    }
    g_data.render_data.device.reset();
    g_data.render_data.swapchain.reset();
#endif
}

void destroy_window()
{
    if (g_data.window_data.window != nullptr) {
        glfwDestroyWindow(g_data.window_data.window);

        g_data.window_data.window = nullptr;
    }

    glfwTerminate();
}