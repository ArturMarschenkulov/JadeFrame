#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {
class Window;

namespace vulkan {
namespace win32 {

auto create_surface(VkInstance instance, const Window* window_handle) -> VkSurfaceKHR;

} // namespace win32
} // namespace vulkan
} // namespace JadeFrame