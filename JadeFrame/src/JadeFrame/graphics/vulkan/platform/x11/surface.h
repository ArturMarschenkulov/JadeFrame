#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {
class Window;

namespace vulkan {
namespace x11 {

auto create_surface(VkInstance instance, const Window* window_handle) -> VkSurfaceKHR;

} // namespace x11
} // namespace vulkan
} // namespace JadeFrame