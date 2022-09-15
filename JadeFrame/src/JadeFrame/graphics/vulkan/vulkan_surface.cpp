#include "pch.h"
#include "vulkan_surface.h"

#include <cassert>

namespace JadeFrame {
auto VulkanSurface::init(VkInstance instance, HWND window_handle) -> void {
    Logger::trace("VulkanSurface::init start");
    m_window_handle = window_handle;

    VkResult result;
#if _WIN32
    const VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .hinstance = ::GetModuleHandleW(NULL),
        .hwnd = window_handle,
    };

    result = vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) {
        assert(false);
        throw std::runtime_error("failed to create window surface!");
    }
#else
    assert(false && "not implemented yet");
#endif
    Logger::trace("VulkanSurface::init end");
}

auto VulkanSurface::deinit() -> void { vkDestroySurfaceKHR(m_instance, m_handle, nullptr); }




} // namespace JadeFrame