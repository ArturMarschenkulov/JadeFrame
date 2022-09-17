#pragma once
#include <vulkan/vulkan.h>


namespace JadeFrame {

class VulkanLogicalDevice;

namespace vulkan {
class Fence {
public:
    auto init(const VulkanLogicalDevice& device) -> void;
    auto deinit() -> void;
    auto wait_for_fences() -> void;
    auto reset() -> void;

    VkFence                    m_handle = VK_NULL_HANDLE;
    const VulkanLogicalDevice* m_device = nullptr;
};

class Semaphore {
public:
    auto init(const VulkanLogicalDevice& device) -> void;
    auto deinit() -> void;

    VkSemaphore                m_handle = VK_NULL_HANDLE;
    const VulkanLogicalDevice* m_device = nullptr;
};
} // namespace vulkan
} // namespace JadeFrame