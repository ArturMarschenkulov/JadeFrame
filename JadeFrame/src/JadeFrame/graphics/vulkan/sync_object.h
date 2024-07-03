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
    Fence(Fence&&) noexcept;
    auto operator=(Fence&&) noexcept -> Fence&;

    Fence(LogicalDevice& device, bool signaled);

    auto               wait_for_fences() -> void;
    auto               reset() -> void;
    [[nodiscard]] auto is_signaled() const -> bool;

    VkFence        m_handle = VK_NULL_HANDLE;
    LogicalDevice* m_device = nullptr;
};

// synchronizes GPU tasks.
class Semaphore {
public:
    Semaphore() = default;
    ~Semaphore();
    Semaphore(const Semaphore&) = delete;
    auto operator=(const Semaphore&) -> Semaphore& = delete;
    Semaphore(Semaphore&&) noexcept;
    auto operator=(Semaphore&&) noexcept -> Semaphore&;

    explicit Semaphore(LogicalDevice& device);

    VkSemaphore    m_handle = VK_NULL_HANDLE;
    LogicalDevice* m_device = nullptr;
};
} // namespace vulkan
} // namespace JadeFrame