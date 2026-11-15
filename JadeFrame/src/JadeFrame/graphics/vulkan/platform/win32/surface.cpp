#include "surface.h"
#include "../../context.h"
#include "../../shared.h"

#include "JadeFrame/platform/windows/windows_window.h"

namespace JadeFrame {
namespace vulkan {
namespace win32 {
static auto surface_error(std::string message) -> Result<VkSurfaceKHR, InitError> {
    return err<VkSurfaceKHR, InitError>(InitError{
        InitErrorCode::CONTEXT_CREATION_FAILED,
        GRAPHICS_API::VULKAN,
        std::move(message),
    });
}

auto create_surface(VkInstance instance, Window* window_handle)
    -> Result<VkSurfaceKHR, InitError> {
    const auto* win = window_handle == nullptr
                          ? nullptr
                          : dynamic_cast<const JadeFrame::win32::NativeWindow*>(
                                window_handle->m_native_window.get()
                            );
    if (win == nullptr) { return surface_error("the window has no Win32 native window"); }

    const VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = win->m_instance_handle,
        .hwnd = win->m_window_handle,
    };
    VkSurfaceKHR handle = VK_NULL_HANDLE;

    VkResult result =
        vkCreateWin32SurfaceKHR(instance, &create_info, Instance::allocator(), &handle);
    if (result != VK_SUCCESS) {
        return surface_error(
            std::string("vkCreateWin32SurfaceKHR failed: ") + to_string(result)
        );
    }
    return ok<VkSurfaceKHR, InitError>(handle);
    // { Logger::info("Created Win32 surface {} at {}", fmt::ptr(this),
    // fmt::ptr(m_handle)); }
}
} // namespace win32
} // namespace vulkan
} // namespace JadeFrame