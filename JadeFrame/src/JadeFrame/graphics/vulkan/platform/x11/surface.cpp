#include "surface.h"
#include "../../context.h"
#include "../../shared.h"
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan.h>
#include <X11/Xlib.h>
#include "vulkan/vulkan_xlib.h"

#include "JadeFrame/platform/linux/linux_window.h"
#include "JadeFrame/platform/platform_shared.h"

namespace JadeFrame {
namespace vulkan {
namespace x11 {
static auto surface_error(std::string message) -> Result<VkSurfaceKHR, InitError> {
    return err<VkSurfaceKHR, InitError>(InitError{
        InitErrorCode::CONTEXT_CREATION_FAILED,
        GRAPHICS_API::VULKAN,
        std::move(message),
    });
}

auto create_surface(VkInstance instance, Window* window_handle)
    -> Result<VkSurfaceKHR, InitError> {
#undef linux
    const auto* win = window_handle == nullptr
                          ? nullptr
                          : dynamic_cast<const JadeFrame::X11_NativeWindow*>(
                                window_handle->m_native_window.get()
                            );
    if (win == nullptr || win->m_display == nullptr || win->m_window == 0) {
        return surface_error("the window has no X11 display connection or window");
    }

    const VkXlibSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .dpy = win->m_display,
        .window = win->m_window,
    };
    VkSurfaceKHR handle = VK_NULL_HANDLE;

    VkResult result =
        vkCreateXlibSurfaceKHR(instance, &create_info, Instance::allocator(), &handle);
    if (result != VK_SUCCESS) {
        return surface_error(
            std::string("vkCreateXlibSurfaceKHR failed: ") + to_string(result)
        );
    }
    return ok<VkSurfaceKHR, InitError>(handle);
    // { Logger::info("Created Win32 surface {} at {}", fmt::ptr(this),
    // fmt::ptr(m_handle)); }
}
} // namespace x11
} // namespace vulkan
} // namespace JadeFrame