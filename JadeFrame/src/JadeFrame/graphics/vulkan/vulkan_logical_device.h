#pragma once

#include <vulkan/vulkan.h>
#include "vulkan_shared.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_shader.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_command_buffers.h"

#include "JadeFrame/prelude.h"

#include <array>
#include <vector>

namespace JadeFrame {

class Vulkan_Renderer;

namespace vulkan {
class Instance;
class LogicalDevice;
class Pipeline;
class PhysicalDevice;
class Fence;
class Semaphore;

class Queue {
public:
    Queue() = default;
    ~Queue() = default;
    Queue(const Queue&) = delete;
    auto operator=(const Queue&) -> Queue& = delete;

    Queue(Queue&& other)
        : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE)) {}

    auto operator=(Queue&& other) -> Queue& {
        if (this != &other) { m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE); }
        return *this;
    }

    Queue(const LogicalDevice& device, u32 queue_family_index, u32 queue_index);

public:
    auto submit(const VkSubmitInfo& submit_info, const Fence* p_fence) const -> void;
    auto submit(
        const CommandBuffer& cmd_buffer, const Semaphore* wait_semaphore, const Semaphore* signal_semaphore,
        const Fence* p_fence
    ) const -> void;
    auto wait_idle() const -> void;
    auto present(VkPresentInfoKHR info) const -> VkResult;
    auto present(const u32& index, const Swapchain& swapchain, const Semaphore* result) const -> VkResult;

public:
    VkQueue m_handle = VK_NULL_HANDLE;
    // const QueueFamily* = nullptr;
};

class LogicalDevice {
private:

public:
    LogicalDevice() = default;
    ~LogicalDevice();

    LogicalDevice(const LogicalDevice&) = delete;
    auto operator=(const LogicalDevice&) -> LogicalDevice& = delete;

    LogicalDevice(LogicalDevice&& other);
    auto operator=(LogicalDevice&& other) -> LogicalDevice&;

    auto init(const Instance& instance, const PhysicalDevice& physical_device, const Surface& surface) -> void;
    auto deinit() -> void;

public:
    VkDevice              m_handle = VK_NULL_HANDLE;
    const Instance*       m_instance = nullptr;
    const PhysicalDevice* m_physical_device = nullptr;

public:
    auto query_queues(u32 queue_family_index, u32 queue_index) -> Queue;

    Queue m_graphics_queue;
    Queue m_present_queue;

public:
    auto create_command_pool(const QueueFamilyIndex& queue_family_index) -> CommandPool;

    CommandPool m_command_pool;

    auto create_descriptor_pool(u32 max_sets, std::vector<VkDescriptorPoolSize>& pool_sizes) -> DescriptorPool;
    auto create_descriptor_set_layout(std::vector<vulkan::DescriptorSetLayout::Binding>& bindings) const
        -> DescriptorSetLayout;
    DescriptorPool m_set_pool;

public: // Swapchain stuff
    auto create_swapchain(const Surface& surface) -> Swapchain;
    auto create_render_pass(VkFormat image_format) -> RenderPass;
    auto create_framebuffer(const ImageView& image_view, const RenderPass& render_pass, VkExtent2D extent)
        -> Framebuffer;

public:
    // auto create_image() -> vulkan::Image;
    auto create_image_view(Image& image, VkFormat) -> ImageView;

public: // synchro objects
    auto create_semaphore() const -> Semaphore;
    auto create_fence(bool signaled) const -> Fence;
    auto wait_for_fence(const Fence& fences, bool wait_all, u64 timeout) const -> void;
    auto wait_for_fences(const std::vector<Fence>& fences, bool wait_all, u64 timeout) const -> void;

public: // Misc

public:
    auto create_buffer(Buffer::TYPE buffer_type, void* data, size_t size) const -> Buffer*;

    auto create_shader(const Vulkan_Renderer& renderer, const Vulkan_Shader::Desc& desc) -> Vulkan_Shader;

public:
    // template<typename T, typename U>
    // using HashMap = std::unordered_map<T, U>;
    mutable std::unordered_map<u32, vulkan::Buffer> m_buffers;
};

} // namespace vulkan
} // namespace JadeFrame