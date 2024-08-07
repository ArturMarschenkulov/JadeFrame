#pragma once

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
    #define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

namespace JadeFrame {
class Window;

namespace vulkan {
// namespace win32 {
class Surface {
private:

public:
    Surface() = default;
    ~Surface();
    Surface(const Surface&) = delete;
    auto operator=(const Surface&) -> Surface& = delete;
    Surface(Surface&& other) noexcept;
    auto operator=(Surface&& other) noexcept -> Surface&;

    Surface(VkInstance instance, const Window* window_handle);

public:
    VkSurfaceKHR  m_handle = VK_NULL_HANDLE;
    const Window* m_window_handle = nullptr;
    VkInstance    m_instance = nullptr;
};

//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame
