#include "surface.h"
#include "JadeFrame/utils/logger.h"
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
        if (m_handle != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_instance, m_handle, Instance::allocator());
        }
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_window_handle = std::exchange(other.m_window_handle, nullptr);
        m_instance = std::exchange(other.m_instance, nullptr);
    }
    return *this;
}

auto Surface::create(VkInstance instance, Window* window_handle)
    -> Result<Surface, InitError> {
#if _WIN32
    auto handle = win32::create_surface(instance, window_handle);
#elif __linux__
    auto handle = x11::create_surface(instance, window_handle);
#else
    auto handle = err<VkSurfaceKHR, InitError>(InitError{
        InitErrorCode::BACKEND_UNSUPPORTED,
        GRAPHICS_API::VULKAN,
        "the Vulkan renderer cannot create a surface on this platform",
    });
#endif
    if (handle.is_err()) { return err<Surface, InitError>(std::move(handle).error()); }

    Surface surface;
    surface.m_handle = handle.value();
    surface.m_window_handle = window_handle;
    surface.m_instance = instance;
    return ok<Surface, InitError>(std::move(surface));
}

Surface::~Surface() {
    if (m_handle != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_handle, Instance::allocator());
        m_handle = VK_NULL_HANDLE;
        m_instance = VK_NULL_HANDLE;
        m_window_handle = nullptr;
    }
}

//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame
