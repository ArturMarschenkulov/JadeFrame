#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {
class IWindow;

namespace vulkan {
namespace win32 {

auto create_surface(VkInstance instance, const IWindow* window_handle) -> VkSurfaceKHR;

} // namespace win32
} // namespace vulkan
} // namespace JadeFrame