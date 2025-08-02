#include "surface.h"
#include "../../context.h"
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include <X11/Xlib.h>
#include "vulkan/vulkan_xlib.h"

#include "JadeFrame/platform/linux/linux_window.h"
#include "JadeFrame/platform/platform_shared.h"

namespace JadeFrame {
namespace vulkan {
namespace x11 {
auto create_surface(VkInstance instance, Window* window_handle) -> VkSurfaceKHR {
#undef linux
    const auto* win = dynamic_cast<const JadeFrame::X11_NativeWindow*>(
        window_handle->m_native_window.get()
    );

    const VkXlibSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .dpy = win->m_display,
        .window = win->m_window,
    };
    VkSurfaceKHR handle;

    VkResult result =
        vkCreateXlibSurfaceKHR(instance, &create_info, Instance::allocator(), &handle);
    if (result != VK_SUCCESS) {
        assert(false);
        throw std::runtime_error("failed to create window surface!");
    }
    return handle;
    // { Logger::info("Created Win32 surface {} at {}", fmt::ptr(this),
    // fmt::ptr(m_handle)); }
}
} // namespace x11
} // namespace vulkan
} // namespace JadeFrame