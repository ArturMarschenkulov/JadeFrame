#pragma once

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
    #define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

#include "JadeFrame/graphics/graphics_shared.h"

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

    /// Creates the surface of `window_handle`. There is no failing constructor: a
    /// `Surface` either owns a valid handle or is empty.
    [[nodiscard]] static auto create(VkInstance instance, Window* window_handle)
        -> Result<Surface, InitError>;

public:
    VkSurfaceKHR m_handle = VK_NULL_HANDLE;
    Window*      m_window_handle = nullptr;
    VkInstance   m_instance = VK_NULL_HANDLE;
};

//} // namespace win32
} // namespace vulkan
} // namespace JadeFrame
