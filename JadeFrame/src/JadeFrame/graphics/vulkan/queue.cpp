#include "queue.h"
#include "physical_device.h"
#include "surface.h"

namespace JadeFrame {
namespace vulkan {



auto QueueFamily::supports_present(const Surface& surface) const -> bool {
    VkBool32 present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        m_physical_device->m_handle, m_index, surface.m_handle, &present_support
    );
    return (bool)present_support;
}

auto QueueFamily::query_queues(const LogicalDevice& device, u32 index) const -> Queue {
    return Queue(device, m_index, index);
}

} // namespace vulkan
} // namespace JadeFrame