#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

struct HWND__;
typedef HWND__* HWND;

namespace JadeFrame {
namespace vulkan {
// namespace win32 {
class Surface {
private:
public:
    auto init(VkInstance instance, HWND window_handle) -> void;
    auto deinit() -> void;

public:
    VkSurfaceKHR m_handle = VK_NULL_HANDLE;
    HWND         m_window_handle = nullptr;
    VkInstance   m_instance = nullptr;
};
//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame
