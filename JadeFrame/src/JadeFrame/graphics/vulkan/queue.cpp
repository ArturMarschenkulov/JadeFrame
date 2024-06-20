#include "queue.h"
#include "physical_device.h"
#include "surface.h"

namespace JadeFrame {
namespace vulkan {

static auto to_format_string_queue_family(const QueueFamily& queue_family)
    -> std::string {
    std::string result = "{ ";
    if ((queue_family.m_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U) {
        result += "Graphics ";
    }
    if ((queue_family.m_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0U) {
        result += "Compute ";
    }
    if ((queue_family.m_properties.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0U) {
        result += "Transfer ";
    }
    if ((queue_family.m_properties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0U) {
        result += "SparseBinding ";
    }
    if ((queue_family.m_properties.queueFlags & VK_QUEUE_PROTECTED_BIT) != 0U) {
        result += "Protected ";
    }
    if (queue_family.m_present_support == VK_TRUE) { result += "Present "; }
    result += "}";
    return result;
}

static auto query_surface_support(
    const PhysicalDevice& physical_device,
    u32                   index,
    const Surface&        surface
) -> bool {
    VkBool32 present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        physical_device.m_handle, index, surface.m_handle, &present_support
    );
    return (bool)present_support;
}

auto QueueFamily::supports_present(const Surface& surface) const -> bool {
    auto present_support = query_surface_support(*m_physical_device, m_index, surface);
    return present_support;
}

auto QueueFamily::query_queues(const LogicalDevice& device, u32 index) const -> Queue {
    return Queue(device, m_index, index);
}

} // namespace vulkan
} // namespace JadeFrame