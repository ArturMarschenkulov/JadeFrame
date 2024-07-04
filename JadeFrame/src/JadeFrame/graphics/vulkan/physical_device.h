#pragma once
#include <vulkan/vulkan.h>
#include "logical_device.h"
#include "queue.h"
#include "JadeFrame/utils/option.h"

#include "JadeFrame/prelude.h"

#include <optional>
#include <vector>

namespace JadeFrame {

namespace vulkan {
class Instance;
class Surface;
class PhysicalDevice;

class QueueFamilyPointers {
public:
    QueueFamily* m_graphics_family = nullptr;
    QueueFamily* m_compute_family = nullptr;
    QueueFamily* m_transfer_family = nullptr;
    QueueFamily* m_present_family = nullptr;

    [[nodiscard]] auto is_complete() const -> bool {
        return m_graphics_family != nullptr && m_present_family != nullptr;
    }
};

class PhysicalDevice {
private:

public:
    auto               init(Instance& instance, const Surface& surface) -> void;
    [[nodiscard]] auto check_extension_support(const std::vector<const char*>& extensions
    ) const -> bool;
    [[nodiscard]] static auto choose_fitting_queue_families(
        std::vector<QueueFamily>& queue_families,
        const Surface&            surface
    ) -> QueueFamilyPointers;
    [[nodiscard]] auto
    find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const -> u32;

    [[nodiscard]] auto query_memory_properties() const
        -> VkPhysicalDeviceMemoryProperties;
    [[nodiscard]] auto query_properties() const -> VkPhysicalDeviceProperties;
    [[nodiscard]] auto query_features() const -> VkPhysicalDeviceFeatures;

    [[nodiscard]] auto query_queue_family_properties() const
        -> std::vector<VkQueueFamilyProperties>;
    [[nodiscard]] auto query_queue_families() const -> std::vector<QueueFamily>;
    [[nodiscard]] auto query_surface_capabilities(const Surface& surface) const
        -> VkSurfaceCapabilitiesKHR;
    [[nodiscard]] auto query_surface_formats(const Surface& surface) const
        -> std::vector<VkSurfaceFormatKHR>;
    [[nodiscard]] auto query_surface_present_modes(const Surface& surface) const
        -> std::vector<VkPresentModeKHR>;

    [[nodiscard]] auto query_extension_properties() const
        -> std::vector<VkExtensionProperties>;

    [[nodiscard]] auto limits() const -> VkPhysicalDeviceLimits;

    [[nodiscard]] auto find_depth_format() const -> VkFormat;

    auto create_logical_device() -> LogicalDevice;

public:
    VkPhysicalDevice m_handle;
    Instance*        m_instance_p = nullptr;

    VkPhysicalDeviceProperties       m_properties = {};
    VkPhysicalDeviceFeatures         m_features = {};
    VkPhysicalDeviceMemoryProperties m_memory_properties = {};

    // Queue stuff
    std::vector<QueueFamily> m_queue_families;
    QueueFamilyPointers      m_chosen_queue_family_pointers;

    // Extension stuff
    std::vector<VkExtensionProperties> m_extension_properties;
    std::vector<const char*>           m_device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    bool m_extension_support;
};

} // namespace vulkan
} // namespace JadeFrame