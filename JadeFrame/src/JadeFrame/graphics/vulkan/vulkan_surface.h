#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

namespace JadeFrame {
class IWindow;
namespace vulkan {
// namespace win32 {
class Surface {
private:
public:
    auto init(VkInstance instance, const IWindow* window_handle) -> void;
    auto deinit() -> void;

public:
    VkSurfaceKHR   m_handle = VK_NULL_HANDLE;
    const IWindow* m_window_handle = nullptr;
    VkInstance     m_instance = nullptr;
};
//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame
