#include "surface.h"
#include "../../vulkan_context.h"

#include "JadeFrame/platform/windows/windows_window.h"

namespace JadeFrame {
namespace vulkan {
namespace win32 {
auto create_surface(VkInstance instance, const Window* window_handle) -> VkSurfaceKHR {
    auto win = static_cast<const JadeFrame::win32::Window*>(window_handle);

    const VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = win->m_instance_handle,
        .hwnd = win->m_window_handle,
    };
    VkSurfaceKHR handle;

    VkResult result =
        vkCreateWin32SurfaceKHR(instance, &create_info, Instance::allocator(), &handle);
    if (result != VK_SUCCESS) {
        assert(false);
        throw std::runtime_error("failed to create window surface!");
    }
    return handle;
    // { Logger::info("Created Win32 surface {} at {}", fmt::ptr(this),
    // fmt::ptr(m_handle)); }
}
} // namespace win32
} // namespace vulkan
} // namespace JadeFrame