#include "windows_window.h"

#include <Windows.h>

#include "windows_message_map.h"
#include "windows_shared.h"

#include "JadeFrame/base_app.h" // for the singleton

#include <tuple>
#include <unordered_map>
#include <cassert>

namespace JadeFrame {

namespace win32 {

static auto window_resize_callback(NativeWindow& window, const EventMessage& wm) -> void {

    switch (wm.wParam) {
        case SIZE_MAXIMIZED:
            window.set_window_state(JadeFrame::Window::WINDOW_STATE::MAXIMIZED);
            break;
        case SIZE_MINIMIZED:
            window.set_window_state(JadeFrame::Window::WINDOW_STATE::MINIMIZED);
            break;
        case SIZE_RESTORED:
            window.set_window_state(JadeFrame::Window::WINDOW_STATE::WINDOWED);
            break;
        case SIZE_MAXHIDE: break;
        case SIZE_MAXSHOW: break;
    }

    window.set_size(v2u32(LOWORD(wm.lParam), HIWORD(wm.lParam)));

    const v2u32& size = window.get_size();
    auto&        renderer =
        Instance::get_singleton()->m_current_app_p->m_render_system.m_renderer;
    renderer->set_viewport(0, 0, size.x, size.y);
}

static auto window_move_callback(NativeWindow& window, const EventMessage& wm) -> void {
    // NOTE: wParam is not used

    window.set_position(v2u32(LOWORD(wm.lParam), HIWORD(wm.lParam)));
}

static auto window_focus_callback(NativeWindow& window, bool should_focus) {
    window.has_focus = should_focus;
}

static auto CALLBACK
window_procedure(::HWND hWnd, ::UINT message, ::WPARAM wParam, ::LPARAM lParam)
    -> ::LRESULT {
    const EventMessage& wm = {hWnd, message, wParam, lParam};

    BaseApp* app = Instance::get_singleton()->m_current_app_p;
    if (app == nullptr) {
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    } else {
    }

    auto* nat_window = reinterpret_cast<NativeWindow*>(::GetPropW(hWnd, L"Window"));
    auto& input_manager = nat_window->m_platform_window->m_queue;
    i32           current_window_id = -1;
    for (auto const& [window_id, window] : app->m_windows) {
        // if (window->get() == hWnd) { 
            current_window_id = window_id; 
        //}
    }
    NativeWindow& current_window =
        *reinterpret_cast<NativeWindow*>(app->m_windows[current_window_id]);

    switch (message) {
        case WM_SETFOCUS:
        case WM_KILLFOCUS: {
            bool should_focus = message == WM_SETFOCUS ? true : false;
            window_focus_callback(current_window, should_focus);
        } break;
        case WM_SIZE: {
            window_resize_callback(current_window, wm);

        } break;
        case WM_MOVE: {
            window_move_callback(current_window, wm);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            key_callback(wm, &input_manager);
        } break;
        case WM_CHAR: {
            char_callback(wm);
        } break;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEMOVE: {
            mouse_button_callback(wm, &input_manager);
        } break;

        /*
            Since WM_CLOSE, WM_DESTROY and WM_QUIT get frequently confused, I will explain
           them here.
            - WM_CLOSE: When one does something with the intention of closing a window.
           Like pressing X or Alt+F4.
            - WM_DESTROY: When window is about to be destroyed.
            - WM_NCDESTROY: When window is destroyed.

            - WM_QUIT is sent when the application is closed. Usually called after
           WM_DESTROY.
        */
        case WM_CLOSE: {
            // TODO: This code needs to be moved to WM_DESTROY.


            app->m_windows.erase(current_window_id);

            Logger::trace("\tLOG: Window Nr {} closing", app->m_windows.size());
            if (app->m_windows.empty() == true) {
                Logger::trace("\tLOG: All Windows were closed");
                ::PostQuitMessage(0);
            } else if (app->m_windows.contains(0) == false) {
                Logger::trace("\tLOG: Main Window was closed thus every other as well");
                ::PostQuitMessage(0);
                app->m_is_running = false;
            }

            // TODO: Add code which deals with ImGui!

        } break;
        case WM_DESTROY: {
            //::PostQuitMessage(0);
            return DefWindowProc(hWnd, message, wParam, lParam);
        } break;
        case WM_QUIT: {
            return DefWindowProc(hWnd, message, wParam, lParam);
            //::PostQuitMessage(0);
        } break;

        default: {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return 0; // message handled
}

static auto get_style(const JadeFrame::Window::Desc& desc) -> ::DWORD {
    DWORD style = 0;

    style |=
        (0 | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX |
         WS_MAXIMIZEBOX); // WS_OVERLAPPEDWINDOW

    if (desc.visable == true) { style |= WS_VISIBLE; }

    if (desc.accept_drop_files == true) { style |= WM_DROPFILES; }

    return style;
}

static auto register_class(HINSTANCE instance) -> ::WNDCLASSEX {
    static bool is_window_class_registered = false;
    if (is_window_class_registered == true) {
        Logger::err("window is already registered. This should not be possible!!");
        assert(!"should not be here");
    }

    ::WNDCLASSEX window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = window_procedure;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO); // IDI_APPLICATION
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground =
        reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)); // nullptr;
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = L"JadeFrame"; //"L"JadeFrame Window";

    ::ATOM res = ::RegisterClassEx(&window_class);
    if (!res) { Logger::err("Window Registration Failed! {}", ::GetLastError()); }

    is_window_class_registered = true;
    return window_class;
}

NativeWindow::NativeWindow(const JadeFrame::Window::Desc& desc) {
    ::HMODULE inst = ::GetModuleHandleW(NULL);
    ::WNDCLASSEX wc = register_class(inst);

    ::DWORD window_style = get_style(desc);

    ::HWND window_handle = ::CreateWindowExW(
        0,                                                          // window_ex_style,
        L"JadeFrame",                                               // app_window_class,
        to_wide_char(static_cast<const char*>(desc.title.c_str())), // app_window_title,
        window_style,
        (desc.position.x == -1) ? CW_USEDEFAULT : desc.position.x, // window_x,
        (desc.position.y == -1) ? CW_USEDEFAULT : desc.position.y, // window_y,
        static_cast<int32_t>(desc.size.x), // window_width, //CW_USEDEFAULT;
        static_cast<int32_t>(desc.size.y), // window_height, //CW_USEDEFAULT;
        NULL,                              // parent_window
        NULL,                              // menu
        inst,
        NULL // lpParam
    );
    if (window_handle == NULL) {
        Logger::err("win32_create_window error: {}", ::GetLastError());
        assert(false);
    }

    ::RECT client_rect = {};
    ::GetClientRect(window_handle, &client_rect);
    ::RECT window_rect = {};
    ::GetWindowRect(window_handle, &window_rect);


    ::SetPropW(window_handle, L"Window", reinterpret_cast<HANDLE>(this));

    m_title = desc.title;
    m_size = v2u32(client_rect.right, client_rect.bottom);
    m_position = v2u32(window_rect.left, window_rect.top);
    m_window_handle = window_handle;
    m_window_state = JadeFrame::Window::WINDOW_STATE::WINDOWED;
    if (desc.visable == true) {
        ::ShowWindow(window_handle, SW_SHOW);
        m_window_state = JadeFrame::Window::WINDOW_STATE::WINDOWED;
    } else {
        //::ShowWindow(window_handle, SW_SHOW);
        // m_window_state = WINDOW_STATE::MINIMIZED;
    }
}

NativeWindow::~NativeWindow() { ::DestroyWindow(m_window_handle); }

auto NativeWindow::handle_events(bool& is_running) -> void {
    // TODO: Abstract the Windows code away
    ::MSG message;
    while (::PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            is_running = false;
            return;
        }
        ::TranslateMessage(&message);
        ::DispatchMessageW(&message);
    }
}

auto NativeWindow::set_title(const std::string& title) -> void {
    m_title = title;
    ::SetWindowTextA(m_window_handle, m_title.c_str());
}

auto NativeWindow::get_title() const -> std::string { return m_title; }

auto NativeWindow::set_size(const v2u32& size) -> void { m_size = size; }

auto NativeWindow::get_size() const -> const v2u32& { return m_size; }

auto NativeWindow::set_position(const v2u32& position) -> void { m_position = position; }

auto NativeWindow::get_position() const -> const v2u32& { return m_position; }

auto NativeWindow::set_window_state(const JadeFrame::Window::WINDOW_STATE window_state) -> void {
    m_window_state = window_state;
}

auto NativeWindow::get_window_state() const -> JadeFrame::Window::WINDOW_STATE { return m_window_state; }

auto NativeWindow::query_client_size() const -> v2u64 {
    ::RECT rect = {};
    ::GetClientRect(m_window_handle, &rect);
    return v2u64(rect.right, rect.bottom);
}
} // namespace win32

} // namespace JadeFrame