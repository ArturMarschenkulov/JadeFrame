#include "platform_shared.h"
#if defined(JF_PLATFORM_LINUX)
    #include "JadeFrame/platform/linux/linux_window.h"
#elif defined(JF_PLATFORM_WINDOWS)
    #include "JadeFrame/platform/windows/windows_window.h"
#else
    #error "Unsupported platform"
#endif

namespace JadeFrame {
Window::Window(const Window::Desc& desc) {
#if defined(JF_PLATFORM_LINUX)
    m_native_window = std::make_unique<X11_NativeWindow>(desc);
#elif defined(JF_PLATFORM_WINDOWS)
    m_native_window = std::make_unique<win32::Window>(desc);
#else
    #error "Unsupported platform"
#endif
}

auto Window::handle_events(bool& running) -> void {
    m_native_window->handle_events(running);
}

auto Window::get_window_state() const -> WINDOW_STATE { return m_window_state; }

auto Window::set_title(const std::string& title) -> void {
    m_native_window->set_title(title);
}

auto Window::get_title() const -> std::string { return m_native_window->get_title(); }

auto Window::get_size() const -> const v2u32& { return m_native_window->get_size(); }

} // namespace JadeFrame