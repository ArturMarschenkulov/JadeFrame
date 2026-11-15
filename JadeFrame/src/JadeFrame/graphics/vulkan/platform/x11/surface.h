#pragma once
#include <vulkan/vulkan.h>

#include "JadeFrame/graphics/graphics_shared.h"

namespace JadeFrame {
class Window;

namespace vulkan {
namespace x11 {

/// Fails with `CONTEXT_CREATION_FAILED` if `window_handle` has no native window of
/// this platform or if Vulkan cannot create a surface for it.
[[nodiscard]] auto create_surface(VkInstance instance, Window* window_handle)
    -> Result<VkSurfaceKHR, InitError>;

} // namespace x11
} // namespace vulkan
} // namespace JadeFrame