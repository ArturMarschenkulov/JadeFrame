#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_logical_device.h"
#include "JadeFrame/utils/option.h"

#include "JadeFrame/prelude.h"

#include <optional>
#include <vector>

namespace JadeFrame {

namespace vulkan {
class Instance;
class Surface;

using QueueFamilyIndex = u32;

class QueueFamilyIndices {
public:
    std::optional<QueueFamilyIndex> m_graphics_family;
    std::optional<QueueFamilyIndex> m_present_family;

    auto is_complete() const -> bool {
        return m_graphics_family.has_value() && m_present_family.has_value();
    }
};

class QueueFamily {
public:
    QueueFamilyIndex        m_index;
    VkQueueFamilyProperties m_properties;
    u32                     m_queue_amount;
    VkBool32                m_present_support;
};

class PhysicalDevice {
private:

public:
    auto init(Instance& instance, const Surface& surface) -> void;
    auto check_extension_support(const std::vector<const char*>& extensions) -> bool;
    auto find_queue_families(
        const std::vector<QueueFamily>& queue_families,
        const Surface&                  surface
    ) -> QueueFamilyIndices;
    auto find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const -> u32;

    auto query_memory_properties() -> VkPhysicalDeviceMemoryProperties;
    auto query_properties() -> VkPhysicalDeviceProperties;
    auto query_features() -> VkPhysicalDeviceFeatures;
    auto query_queue_families() -> std::vector<QueueFamily>;
    auto query_surface_capabilities(const Surface& surface) const
        -> VkSurfaceCapabilitiesKHR;
    auto query_surface_formats(const Surface& surface) const
        -> std::vector<VkSurfaceFormatKHR>;
    auto query_surface_present_modes(const Surface& surface) const
        -> std::vector<VkPresentModeKHR>;

    auto query_extension_properties() -> std::vector<VkExtensionProperties>;

    auto query_limits() const -> VkPhysicalDeviceLimits;

    auto create_logical_device() -> LogicalDevice;

public:
    VkPhysicalDevice m_handle;
    Instance*        m_instance_p = nullptr;

    VkPhysicalDeviceProperties       m_properties = {};
    VkPhysicalDeviceFeatures         m_features = {};
    VkPhysicalDeviceMemoryProperties m_memory_properties = {};

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