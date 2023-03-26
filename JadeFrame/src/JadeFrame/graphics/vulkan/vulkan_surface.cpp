#include "pch.h"
#include "vulkan_surface.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "platform/win32/surface.h"

#include <cassert>

namespace JadeFrame {
namespace vulkan {
// namespace win32 {
auto Surface::init(VkInstance instance, const IWindow* window_handle) -> void {
    Logger::trace("Surface::init start");
    m_window_handle = window_handle;

    VkResult result;
#if _WIN32
    m_handle = win32::create_surface(instance, window_handle);
#else
    assert(false && "not implemented yet");
#endif
    Logger::trace("Surface::init end");
}

auto Surface::deinit() -> void { vkDestroySurfaceKHR(m_instance, m_handle, nullptr); }

//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame