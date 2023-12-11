#include "platform_shared.h"
#if defined(JF_PLATFORM_LINUX)
    #include "JadeFrame/platform/linux/linux_window.h"
#elif defined(JF_PLATFORM_WINDOWS)
    #include "JadeFrame/platform/windows/windows_window.h"
#else
    #error "Unsupported platform"
#endif

namespace JadeFrame {
IWindow::IWindow(const IWindow::Desc& desc) {
#if defined(JF_PLATFORM_LINUX)
    m_native_window = std::make_unique<Linux_Window>(desc);
#elif defined(JF_PLATFORM_WINDOWS)
    m_native_window = std::make_unique<win32::Window>(desc);
#else
    #error "Unsupported platform"
#endif
}

auto IWindow::handle_events(bool& running) -> void {
    m_native_window->handle_events(running);
}

auto IWindow::get_window_state() const -> WINDOW_STATE {
    return (WINDOW_STATE)m_native_window->get_window_state();
}

auto IWindow::set_title(const std::string& title) -> void {
    m_native_window->set_title(title);
}

auto IWindow::get_title() const -> std::string { return m_native_window->get_title(); }

auto IWindow::get_size() const -> const v2u32& { return m_native_window->get_size(); }

} // namespace JadeFrame