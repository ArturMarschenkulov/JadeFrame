#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_logical_device.h"


#include "JadeFrame/prelude.h"

#include <optional>
#include <vector>

namespace JadeFrame {

class VulkanInstance;
namespace vulkan {
class Surface;

using QueueFamilyIndex = u32;
class QueueFamilyIndices {
public:
    std::optional<QueueFamilyIndex> m_graphics_family;
    std::optional<QueueFamilyIndex> m_present_family;
    auto is_complete() -> bool { return m_graphics_family.has_value() && m_present_family.has_value(); }
};
class QueueFamily {
public:
    QueueFamilyIndex        m_index;
    VkQueueFamilyProperties m_properties;
    u32                     m_queue_amount;
    VkBool32                m_present_support;
};
struct SurfaceSupportDetails {
    VkSurfaceCapabilitiesKHR        m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR>   m_present_modes;
};
class PhysicalDevice {
private:
public:
    auto init(VulkanInstance& instance, const Surface& surface) -> void;
    auto check_extension_support(const std::vector<const char*>& extensions) -> bool;
    auto find_queue_families(const std::vector<QueueFamily>& queue_families) -> QueueFamilyIndices;
    auto find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const -> u32;
    auto query_queue_families(const Surface& surface) -> std::vector<QueueFamily>;

    auto create_logical_device() -> LogicalDevice;

public:
    VkPhysicalDevice m_handle;
    VulkanInstance*  m_instance_p = nullptr;

    VkPhysicalDeviceProperties       m_properties = {};
    VkPhysicalDeviceFeatures         m_features = {};
    VkPhysicalDeviceMemoryProperties m_memory_properties = {};
    // Surface
    SurfaceSupportDetails m_surface_support_details;

    // Queue stuff
    std::vector<QueueFamily>             m_queue_families;
    std::vector<VkQueueFamilyProperties> m_queue_family_properties;
    QueueFamilyIndices                   m_queue_family_indices;

    // Extension stuff
    std::vector<VkExtensionProperties> m_extension_properties;
    std::vector<const char*>           m_device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    bool m_extension_support;
};

} // namespace vulkan
} // namespace JadeFrame