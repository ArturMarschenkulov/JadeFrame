#pragma once

#include <utility>

#include <vulkan/vulkan.h>

#include "JadeFrame/types.h"

namespace JadeFrame {
namespace vulkan {
using QueueFamilyIndex = u32;
class Surface;
class PhysicalDevice;
class LogicalDevice;
class CommandBuffer;
class Semaphore;
class Fence;
class Swapchain;

class Queue {
public:
    Queue() = default;
    ~Queue() = default;
    Queue(const Queue&) = delete;
    auto operator=(const Queue&) -> Queue& = delete;

    Queue(Queue&& other) noexcept
        : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE)) {}

    auto operator=(Queue&& other) noexcept -> Queue& {
        if (this != &other) { m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE); }
        return *this;
    }

    Queue(const LogicalDevice& device, u32 queue_family_index, u32 queue_index);

public: // submit methods
    auto submit(const CommandBuffer& cmd_buffer) const -> void;
    auto submit(
        const CommandBuffer& cmd_buffer,
        const Semaphore*     wait_semaphore,
        const Semaphore*     signal_semaphore,
        const Fence*         p_fence
    ) const -> void;

public:
    auto               wait_idle() const -> void;
    [[nodiscard]] auto present(VkPresentInfoKHR info) const -> VkResult;
    auto present(const u32& index, const Swapchain& swapchain, const Semaphore* semaphore)
        const -> VkResult;

public:
    VkQueue m_handle = VK_NULL_HANDLE;
    // const QueueFamily* = nullptr;
};

class QueueFamily {
public:
    QueueFamilyIndex        m_index = 0;
    VkQueueFamilyProperties m_properties = {};
    const PhysicalDevice*   m_physical_device = nullptr;

public:
    [[nodiscard]] auto supports_graphics() const -> bool {
        return (m_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U;
    }

    [[nodiscard]] auto supports_compute() const -> bool {
        return (m_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0U;
    }

    [[nodiscard]] auto supports_transfer() const -> bool {
        return (m_properties.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0U;
    }

    [[nodiscard]] auto supports_present(const Surface& surface) const -> bool;

    [[nodiscard]] auto
    query_queues(const LogicalDevice& device, u32 index) const -> Queue;
};
} // namespace vulkan
} // namespace JadeFrame