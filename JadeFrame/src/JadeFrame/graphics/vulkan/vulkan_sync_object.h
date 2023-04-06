#pragma once
#include <vulkan/vulkan.h>


namespace JadeFrame {


namespace vulkan {
class LogicalDevice;

// synchronizes GPU-to-CPU tasks
class Fence {
public:
    Fence() = default;
    ~Fence() = default;
    Fence(const Fence&) = delete;
    auto operator=(const Fence&) -> Fence& = delete;
    Fence(Fence&&) = default;
    auto operator=(Fence&&) -> Fence& = default;

    Fence(const LogicalDevice& device, bool signaled);

    auto deinit() -> void;
    auto wait_for_fences() -> void;
    auto reset() -> void;
    auto is_signaled() -> bool;

    VkFence              m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
};

// synchronizes GPU tasks.
class Semaphore {
public:
    Semaphore() = default;
    ~Semaphore() = default;
    Semaphore(const Semaphore&) = delete;
    auto operator=(const Semaphore&) -> Semaphore& = delete;
    Semaphore(Semaphore&&) = default;
    auto operator=(Semaphore&&) -> Semaphore& = default;

    Semaphore(const LogicalDevice& device);

    auto deinit() -> void;

    VkSemaphore          m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
};
} // namespace vulkan
} // namespace JadeFrame