#pragma once
#include <vulkan/vulkan.h>


namespace JadeFrame {


namespace vulkan {
class LogicalDevice;

// synchronizes GPU-to-CPU tasks
class Fence {
public:
    Fence() = default;
    ~Fence();
    Fence(const Fence&) = delete;
    auto operator=(const Fence&) -> Fence& = delete;
    Fence(Fence&&);
    auto operator=(Fence&&) -> Fence&;

    Fence(const LogicalDevice& device, bool signaled);

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
    ~Semaphore();
    Semaphore(const Semaphore&) = delete;
    auto operator=(const Semaphore&) -> Semaphore& = delete;
    Semaphore(Semaphore&&);
    auto operator=(Semaphore&&) -> Semaphore&;

    Semaphore(const LogicalDevice& device);

    VkSemaphore          m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
};
} // namespace vulkan
} // namespace JadeFrame