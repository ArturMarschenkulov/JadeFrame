#pragma once
#include <vulkan/vulkan.h>


namespace JadeFrame {


namespace vulkan {
class LogicalDevice;
class Fence {
public:
    auto init(const LogicalDevice& device) -> void;
    auto deinit() -> void;
    auto wait_for_fences() -> void;
    auto reset() -> void;

    VkFence              m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
};

class Semaphore {
public:
    auto init(const LogicalDevice& device) -> void;
    auto deinit() -> void;

    VkSemaphore          m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
};
} // namespace vulkan
} // namespace JadeFrame