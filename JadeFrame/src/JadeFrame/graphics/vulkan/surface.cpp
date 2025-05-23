#include "surface.h"
#include "context.h"
#if defined(_WIN32)
    #include "JadeFrame/platform/windows/windows_window.h"
    #include "platform/win32/surface.h"
#elif defined(__linux__)
    #include "platform/x11/surface.h"
#endif

namespace JadeFrame {
namespace vulkan {
// namespace win32 {

Surface::Surface(Surface&& other) noexcept
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_window_handle(std::exchange(other.m_window_handle, nullptr))
    , m_instance(std::exchange(other.m_instance, nullptr)) {}

auto Surface::operator=(Surface&& other) noexcept -> Surface& {
    if (this != &other) {
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_window_handle = std::exchange(other.m_window_handle, nullptr);
        m_instance = std::exchange(other.m_instance, nullptr);
    }
    return *this;
}

Surface::Surface(VkInstance instance, const Window* window_handle)
    : m_window_handle(window_handle) {
    Logger::trace("Surface::init start");

#if _WIN32
    m_handle = win32::create_surface(instance, window_handle);
#elif __linux__
    m_handle = x11::create_surface(instance, window_handle);
#else
    assert(false && "not implemented yet");
#endif
    Logger::trace("Surface::init end");
}

Surface::~Surface() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_handle, Instance::allocator());
    }
}

//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame