#include "JadeFrame/platform/window_event.h"
#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/logger.h"

#include "gui.h"

#include "JadeFrame/platform/window.h"
#include <imgui/imgui.h>
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "JadeFrame/platform/window.h"

#if defined(_WIN32)
    #include "imgui/backends/imgui_impl_win32.h"
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "imgui/backends/imgui_impl_x11.h"
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

/*
    The `imgui` repo does not have a linux/x11/wayland backend. One can use `glfw`,
    but that means that we'd have to integrate `glfw` into the engine, which is not
   optimal.


    This means that we'll have to write our own. There are some attempts in the wild
   already. For now, I will collect various links to such resources so that I can write my
   own x11/wayland backend. https://github.com/ocornut/imgui/pull/3372
   https://github.com/ocornut/imgui/issues/4224

*/

namespace JadeFrame {

static auto button_jadeframe_to_imgui(BUTTON button) -> ImGuiMouseButton {
    switch (button) {
        case BUTTON::LEFT: return ImGuiMouseButton_Left;
        case BUTTON::RIGHT: return ImGuiMouseButton_Right;
        case BUTTON::MIDDLE: return ImGuiMouseButton_Middle;
        case BUTTON::X1: return 3;
        case BUTTON::X2: return 4;
        case BUTTON::MAX: return ImGuiMouseButton_COUNT;
    }
    return ImGuiMouseButton_COUNT;
}

static auto key_jadeframe_to_imgui(KEY key) -> ImGuiKey {
    switch (key) {
        case KEY::SPACE: return ImGuiKey_Space;
        case KEY::ESCAPE: return ImGuiKey_Escape;
        case KEY::ENTER: return ImGuiKey_Enter;
        case KEY::TAB: return ImGuiKey_Tab;
        case KEY::BACKSPACE: return ImGuiKey_Backspace;
        case KEY::INSERT: return ImGuiKey_Insert;
        case KEY::DELET: return ImGuiKey_Delete;
        case KEY::RIGHT: return ImGuiKey_RightArrow;
        case KEY::LEFT: return ImGuiKey_LeftArrow;
        case KEY::DOWN: return ImGuiKey_DownArrow;
        case KEY::UP: return ImGuiKey_UpArrow;
        case KEY::PAGE_UP: return ImGuiKey_PageUp;
        case KEY::PAGE_DOWN: return ImGuiKey_PageDown;
        case KEY::HOME: return ImGuiKey_Home;
        case KEY::END: return ImGuiKey_End;
        case KEY::CAPS_LOCK: return ImGuiKey_CapsLock;
        case KEY::SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case KEY::NUM_LOCK: return ImGuiKey_NumLock;
        case KEY::PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case KEY::PAUSE: return ImGuiKey_Pause;
        case KEY::A: return ImGuiKey_A;
        case KEY::B: return ImGuiKey_B;
        case KEY::C: return ImGuiKey_C;
        case KEY::D: return ImGuiKey_D;
        case KEY::E: return ImGuiKey_E;
        case KEY::F: return ImGuiKey_F;
        case KEY::G: return ImGuiKey_G;
        case KEY::H: return ImGuiKey_H;
        case KEY::I: return ImGuiKey_I;
        case KEY::J: return ImGuiKey_J;
        case KEY::K: return ImGuiKey_K;
        case KEY::L: return ImGuiKey_L;
        case KEY::M: return ImGuiKey_M;
        case KEY::N: return ImGuiKey_N;
        case KEY::O: return ImGuiKey_O;
        case KEY::P: return ImGuiKey_P;
        case KEY::Q: return ImGuiKey_Q;
        case KEY::R: return ImGuiKey_R;
        case KEY::S: return ImGuiKey_S;
        case KEY::T: return ImGuiKey_T;
        case KEY::U: return ImGuiKey_U;
        case KEY::V: return ImGuiKey_V;
        case KEY::W: return ImGuiKey_W;
        case KEY::X: return ImGuiKey_X;
        case KEY::Y: return ImGuiKey_Y;
        case KEY::Z: return ImGuiKey_Z;
        case KEY::ZERO: return ImGuiKey_0;
        case KEY::ONE: return ImGuiKey_1;
        case KEY::TWO: return ImGuiKey_2;
        case KEY::THREE: return ImGuiKey_3;
        case KEY::FOUR: return ImGuiKey_4;
        case KEY::FIVE: return ImGuiKey_5;
        case KEY::SIX: return ImGuiKey_6;
        case KEY::SEVEN: return ImGuiKey_7;
        case KEY::EIGHT: return ImGuiKey_8;
        case KEY::NINE: return ImGuiKey_9;
        case KEY::F1: return ImGuiKey_F1;
        case KEY::F2: return ImGuiKey_F2;
        case KEY::F3: return ImGuiKey_F3;
        case KEY::F4: return ImGuiKey_F4;
        case KEY::F5: return ImGuiKey_F5;
        case KEY::F6: return ImGuiKey_F6;
        case KEY::F7: return ImGuiKey_F7;
        case KEY::F8: return ImGuiKey_F8;
        case KEY::F9: return ImGuiKey_F9;
        case KEY::F10: return ImGuiKey_F10;
        case KEY::F11: return ImGuiKey_F11;
        case KEY::F12: return ImGuiKey_F12;
        case KEY::LEFT_SHIFT: return ImGuiKey_LeftShift;
        case KEY::LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case KEY::LEFT_ALT: return ImGuiKey_LeftAlt;
        case KEY::LEFT_SUPER: return ImGuiKey_LeftSuper;
        case KEY::RIGHT_SHIFT: return ImGuiKey_RightShift;
        case KEY::RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case KEY::RIGHT_ALT: return ImGuiKey_RightAlt;
        default: return ImGuiKey_None;
    }
}

GUI::~GUI() { destroy(); }

auto GUI::init(Window* window, GRAPHICS_API api) -> void {
    JF_ASSERT(window != nullptr, "GUI requires a window");
    if (api != GRAPHICS_API::VULKAN && api != GRAPHICS_API::OPENGL) {
        Logger::err("{} is not supported", to_string(api));
        JF_ASSERT(false, "Right now only VULKAN and OPENGL are supported");
    }
    m_window = window;
    m_graphics_api = api;

    ImGui::CreateContext();
    m_has_context = true;
    if (m_window != nullptr) {
        m_window_event_callback_id =
            m_window->add_event_callback([this](const WindowEvent& event) {
                this->on_window_event(event);
            });
    }
#if _WIN32
    NativeWindow* native_window = window->m_native_window.get();
    auto*         native_window_win32 = dynamic_cast<win32::NativeWindow*>(native_window);
    JF_ASSERT(native_window_win32 != nullptr, "Expected a Win32 native window");
    m_platform_backend_initialized =
        ImGui_ImplWin32_Init(native_window_win32->m_window_handle);
#elif __linux__

    NativeWindow* native_window = window->m_native_window.get();
    auto*         native_window_x11 = dynamic_cast<X11_NativeWindow*>(native_window);
    JF_ASSERT(native_window_x11 != nullptr, "Expected an X11 native window");
    ::Display*    display = native_window_x11->m_display;
    ::Window      win = native_window_x11->m_window;
    m_platform_backend_initialized =
        ImGui_ImplX11_Init(display, reinterpret_cast<void*>(win));
#endif
    switch (api) {
        case GRAPHICS_API::OPENGL: {
            const char* glsl_version = "#version 450";
            m_renderer_backend_initialized = ImGui_ImplOpenGL3_Init(glsl_version);
            m_is_initialized =
                m_platform_backend_initialized && m_renderer_backend_initialized;
        } break;
        case GRAPHICS_API::VULKAN: {
            m_is_initialized = false;
            // const char*               glsl_version = "#version 450";
            // ImGui_ImplVulkan_InitInfo info;
            // info.Instance;
            // info.PhysicalDevice;
            // info.Device;
            // info.QueueFamily;
            // info.Queue;
            // info.PipelineCache;
            // info.DescriptorPool;
            // info.Subpass;
            // info.MinImageCount; // >= 2
            // info.ImageCount;    // >= MinImageCount
            // info.MSAASamples;   // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to
            //                     // VK_SAMPLE_COUNT_1_BIT)
            // info.Allocator;
            // info.CheckVkResultFn;

            // ImGui_ImplVulkan_Init(&info);
            // ImGui_ImplOpenGL3_Init(glsl_version);
        } break;
        default: assert(0);
    }
}

auto GUI::new_frame() -> void {

    if (m_graphics_api == GRAPHICS_API::OPENGL) {
        ImGui_ImplOpenGL3_NewFrame();
    } else if (m_graphics_api == GRAPHICS_API::VULKAN) {
        // ImGui_ImplVulkan_NewFrame();
    } else {
        assert(0);
    }
#if _WIN32
    ImGui_ImplWin32_NewFrame();
#elif __linux__
    ImGui_ImplX11_NewFrame();
#endif

    ImGui::NewFrame();
}

auto GUI::render() -> void {

    ImGui::Text("Hello, world %d", 123);
    ImGui::Render();

    if (m_graphics_api == GRAPHICS_API::OPENGL) {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    } else if (m_graphics_api == GRAPHICS_API::VULKAN) {
        // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), nullptr);
    } else {
        assert(0);
    }
}

auto GUI::on_window_event(const WindowEvent& event) -> void {
    if (!m_has_context) { return; }

    ImGuiIO& io = ImGui::GetIO();
    switch (event.type) {
        case WindowEvent::TYPE::KEY: {
            const KeyEvent key_event = event.key_event;
            const ImGuiKey imgui_key = key_jadeframe_to_imgui(key_event.key);
            if (imgui_key != ImGuiKey_None) {
                io.AddKeyEvent(imgui_key, key_event.type == INPUT_STATE::PRESSED);
            }
        } break;
        case WindowEvent::TYPE::BUTTON: {
            const ButtonEvent button_event = event.button_event;
            const ImGuiMouseButton imgui_button =
                button_jadeframe_to_imgui(button_event.button);
            if (imgui_button < ImGuiMouseButton_COUNT) {
                io.AddMouseButtonEvent(
                    imgui_button, button_event.type == INPUT_STATE::PRESSED
                );
            }
        } break;
        case WindowEvent::TYPE::MOUSE: {
            const MouseEvent mouse_event = event.mouse_event;
            io.AddMousePosEvent(
                static_cast<float>(mouse_event.m_x),
                static_cast<float>(mouse_event.m_y)
            );
        } break;
        case WindowEvent::TYPE::RESIZE:
        case WindowEvent::TYPE::CLOSE: break;
    }
}

auto GUI::destroy() -> void {
    if (!m_has_context) { return; }

    if (m_window != nullptr && m_window_event_callback_id != 0) {
        m_window->remove_event_callback(m_window_event_callback_id);
        m_window_event_callback_id = 0;
    }
    m_window = nullptr;

    switch (m_graphics_api) {
        case GRAPHICS_API::OPENGL: {
            if (m_renderer_backend_initialized) { ImGui_ImplOpenGL3_Shutdown(); }
        } break;
        case GRAPHICS_API::VULKAN: {
            // ImGui_ImplVulkan_Shutdown();
        } break;
        default: assert(0);
    }
#if _WIN32
    if (m_platform_backend_initialized) {
        ImGui_ImplWin32_Shutdown();
    }
#elif __linux__
    if (m_platform_backend_initialized) {
        ImGui_ImplX11_Shutdown();
    }
#endif
    ImGui::DestroyContext();
    m_is_initialized = false;
    m_platform_backend_initialized = false;
    m_renderer_backend_initialized = false;
    m_has_context = false;
}
} // namespace JadeFrame
