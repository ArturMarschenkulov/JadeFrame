#include "pch.h"
#include "vulkan_surface.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "platform/win32/surface.h"

#include <cassert>

namespace JadeFrame {
namespace vulkan {
// namespace win32 {

Surface::Surface(Surface&& other)
    : m_handle(other.m_handle)
    , m_window_handle(other.m_window_handle)
    , m_instance(other.m_instance) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_window_handle = nullptr;
    other.m_instance = nullptr;
}
auto Surface::operator=(Surface&& other) -> Surface& {
    if (this != &other) {
        m_handle = other.m_handle;
        m_window_handle = other.m_window_handle;
        m_instance = other.m_instance;

        other.m_handle = VK_NULL_HANDLE;
        other.m_window_handle = nullptr;
        other.m_instance = nullptr;
    }
    return *this;
}
Surface::Surface(VkInstance instance, const IWindow* window_handle) {
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

Surface::~Surface() {
    if (m_handle != VK_NULL_HANDLE) { vkDestroySurfaceKHR(m_instance, m_handle, nullptr); }
}

//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame