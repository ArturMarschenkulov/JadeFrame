#include "windows_window.h"
#include "JadeFrame/utils/logger.h"

#include <Windows.h>
#include <shellapi.h>

#include "windows_shared.h"
#include "windows_input_manager.h"
#include "windows_system_manager.h"

#include <cstddef>
#include <stdexcept>

namespace JadeFrame {

namespace win32 {

namespace {

constexpr wchar_t k_window_class_name[] = L"JadeFrame";

struct WindowClassState {
    HINSTANCE instance = nullptr;
    std::size_t users = 0;
    bool registered = false;
    bool owned = false;
};

WindowClassState g_window_class;

} // namespace

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
    if (window.m_platform_window != nullptr) {
        window.m_platform_window->m_queue.push(
            WindowEvent::make_resize_event(size.x, size.y)
        );
    }
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
    if (message == WM_DROPFILES) {
        // JadeFrame does not expose a file-drop event yet, but the HDROP handle
        // still belongs to the application and must be released.
        ::DragFinish(reinterpret_cast<::HDROP>(wParam));
        return 0;
    }

    const EventMessage& wm = {hWnd, message, wParam, lParam};

    auto* nat_window = reinterpret_cast<NativeWindow*>(::GetPropW(hWnd, L"Window"));
    if (nat_window == nullptr || nat_window->m_platform_window == nullptr) {
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }

    auto&         input_manager = nat_window->m_platform_window->m_queue;
    NativeWindow& current_window = *nat_window;

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
            input_manager.push(WindowEvent::make_close_event());
        } break;
        case WM_DESTROY: {
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }
        case WM_NCDESTROY: {
            ::RemovePropW(hWnd, L"Window");
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }

        default: {
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }

    return 0; // message handled
}

static auto get_style(const JadeFrame::Window::Desc& desc) -> ::DWORD {
    DWORD style = 0;

    style |=
        (0 | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX |
         WS_MAXIMIZEBOX); // WS_OVERLAPPEDWINDOW

    if (desc.visible == true) { style |= WS_VISIBLE; }

    return style;
}

static auto get_extended_style(const JadeFrame::Window::Desc& desc) -> ::DWORD {
    return desc.accept_drop_files ? WS_EX_ACCEPTFILES : 0;
}

static auto register_class(HINSTANCE instance) -> bool {
    if (g_window_class.registered) {
        if (g_window_class.instance != instance) {
            Logger::err("The JadeFrame window class was requested from another module.");
            return false;
        }
        ++g_window_class.users;
        return true;
    }

    ::WNDCLASSEXW window_class = {};
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
    window_class.lpszClassName = k_window_class_name;

    const ::ATOM result = ::RegisterClassExW(&window_class);
    if (result == 0) {
        const DWORD error = ::GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            Logger::err("Window registration failed: {}", error);
            return false;
        }

        // A previous library instance or an embedding application may already
        // own this class. Reuse it only when it is the compatible JadeFrame
        // class; never claim ownership of an unrelated class with the same name.
        ::WNDCLASSEXW existing = {};
        existing.cbSize = sizeof(existing);
        if (!::GetClassInfoExW(instance, k_window_class_name, &existing) ||
            existing.lpfnWndProc != window_procedure || existing.hInstance != instance) {
            Logger::err("An incompatible JadeFrame window class is already registered.");
            return false;
        }
        g_window_class.owned = false;
    } else {
        g_window_class.owned = true;
    }

    g_window_class.instance = instance;
    g_window_class.users = 1;
    g_window_class.registered = true;
    return true;
}

static auto release_class(HINSTANCE instance) -> void {
    if (!g_window_class.registered || g_window_class.instance != instance ||
        g_window_class.users == 0) {
        return;
    }

    --g_window_class.users;
    if (g_window_class.users != 0) { return; }

    if (!g_window_class.owned) {
        g_window_class = {};
        return;
    }

    if (!::UnregisterClassW(k_window_class_name, instance)) {
        Logger::warn(
            "Failed to unregister the JadeFrame window class: {}", ::GetLastError()
        );
        // Keep the state so a later window can reuse the class and retry the
        // unregister operation after any remaining native windows disappear.
        return;
    }

    g_window_class = {};
}

NativeWindow::NativeWindow(const JadeFrame::Window::Desc& desc) {
    const HINSTANCE instance = ::GetModuleHandleW(nullptr);
    const std::wstring window_title = from_string_to_wstring(desc.title);

    try {
        m_instance_handle = instance;
        if (!register_class(instance)) {
            throw std::runtime_error("Failed to register the JadeFrame window class");
        }
        m_class_reference = true;

        ::DWORD window_style = get_style(desc);
        m_window_handle = ::CreateWindowExW(
            get_extended_style(desc),                                  // window_ex_style,
            k_window_class_name,                                       // app_window_class,
            window_title.c_str(),                                      // app_window_title,
            window_style,
            (desc.position.x == -1) ? CW_USEDEFAULT : desc.position.x, // window_x,
            (desc.position.y == -1) ? CW_USEDEFAULT : desc.position.y, // window_y,
            static_cast<int32_t>(desc.size.x), // window_width, //CW_USEDEFAULT;
            static_cast<int32_t>(desc.size.y), // window_height, //CW_USEDEFAULT;
            NULL,                              // parent_window
            NULL,                              // menu
            instance,
            NULL // lpParam
        );
        if (m_window_handle == nullptr) {
            const DWORD error = ::GetLastError();
            throw std::runtime_error(
                "Failed to create the JadeFrame window (error " +
                std::to_string(error) + ")"
            );
        }

        ::RECT client_rect = {};
        if (!::GetClientRect(m_window_handle, &client_rect)) {
            const DWORD error = ::GetLastError();
            throw std::runtime_error(
                "Failed to query the JadeFrame client area (error " +
                std::to_string(error) + ")"
            );
        }
        ::RECT window_rect = {};
        if (!::GetWindowRect(m_window_handle, &window_rect)) {
            const DWORD error = ::GetLastError();
            throw std::runtime_error(
                "Failed to query the JadeFrame window bounds (error " +
                std::to_string(error) + ")"
            );
        }

        if (!::SetPropW(m_window_handle, L"Window", reinterpret_cast<HANDLE>(this))) {
            const DWORD error = ::GetLastError();
            throw std::runtime_error(
                "Failed to associate the JadeFrame window state (error " +
                std::to_string(error) + ")"
            );
        }

        m_title = desc.title;
        m_size = v2u32(client_rect.right, client_rect.bottom);
        m_position = v2u32(window_rect.left, window_rect.top);
        m_window_state = JadeFrame::Window::WINDOW_STATE::WINDOWED;
        if (desc.visible == true) {
            ::ShowWindow(m_window_handle, SW_SHOW);
            m_window_state = JadeFrame::Window::WINDOW_STATE::WINDOWED;
        }
    } catch (...) {
        reset();
        throw;
    }
}

auto NativeWindow::reset() noexcept -> void {
    if (m_window_handle != nullptr) {
        if (!::DestroyWindow(m_window_handle)) {
            Logger::warn("Failed to destroy the JadeFrame window: {}", ::GetLastError());
        }
        ::RemovePropW(m_window_handle, L"Window");
        m_window_handle = nullptr;
    }

    if (m_class_reference) {
        release_class(static_cast<HINSTANCE>(m_instance_handle));
        m_class_reference = false;
    }
    m_instance_handle = nullptr;
}

NativeWindow::~NativeWindow() { reset(); }

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

auto NativeWindow::set_window_state(const JadeFrame::Window::WINDOW_STATE window_state)
    -> void {
    m_window_state = window_state;
}

auto NativeWindow::get_window_state() const -> JadeFrame::Window::WINDOW_STATE {
    return m_window_state;
}

auto NativeWindow::query_client_size() const -> v2u64 {
    ::RECT rect = {};
    ::GetClientRect(m_window_handle, &rect);
    return v2u64(rect.right, rect.bottom);
}
} // namespace win32

} // namespace JadeFrame
