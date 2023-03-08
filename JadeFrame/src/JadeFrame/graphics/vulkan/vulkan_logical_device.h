#pragma once

#include <vulkan/vulkan.h>
#include "vulkan_shared.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_shader.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_sync_object.h"
#include "vulkan_command_buffers.h"


#include "JadeFrame/prelude.h"

#include <array>
#include <vector>

namespace JadeFrame {
class VulkanInstance;



namespace vulkan {
class LogicalDevice;
class Pipeline;
class PhysicalDevice;
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
        const Fence* p_fence) const -> void;
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
    auto init(const VulkanInstance& instance, const PhysicalDevice& physical_device) -> void;
    auto deinit() -> void;

    auto wait_for_fence(const Fence& fences, bool wait_all, u64 timeout) -> void;
    auto wait_for_fences(const std::vector<Fence>& fences, bool wait_all, u64 timeout) -> void;

public:
    VkDevice              m_handle = VK_NULL_HANDLE;
    const VulkanInstance* m_instance = nullptr;
    const PhysicalDevice* m_physical_device = nullptr;

    Queue m_graphics_queue;
    Queue m_present_queue;



public: // Swapchain stuff
    auto create_swapchain() -> Swapchain;
    auto recreate_swapchain() -> void;
    auto cleanup_swapchain() -> void;

    Swapchain m_swapchain;
    // RenderPass m_render_pass;

public:
    // auto create_image() -> vulkan::Image;
    auto create_image_view(Image& image, VkFormat) -> ImageView;



public:
    auto create_command_pool(const QueueFamilyIndex& queue_family_index) -> CommandPool;

    CommandPool                m_command_pool;
    std::vector<CommandBuffer> m_command_buffers;


public: // synchro objects
    auto create_semaphore() -> Semaphore;
    auto create_fence(bool signaled) -> Fence;

    std::vector<Semaphore> m_image_available_semaphores;
    std::vector<Semaphore> m_render_finished_semaphores;
    std::vector<Fence>     m_in_flight_fences;

public: // Misc
    u32    m_present_image_index = 0;
    size_t m_current_frame = 0;
    bool   m_framebuffer_resized = false;


public:
    auto create_buffer(Buffer::TYPE buffer_type, void* data, size_t size) -> Buffer;

    auto create_descriptor_pool(u32 max_sets, std::vector<VkDescriptorPoolSize>& pool_sizes) -> DescriptorPool;
    auto create_descriptor_set_layout(std::vector<vulkan::DescriptorSetLayout::Binding>& bindings) const
        -> DescriptorSetLayout;

    auto query_queues(u32 queue_family_index, u32 queue_index) -> Queue;

    auto create_shader(const Vulkan_Shader::Desc& desc) -> Vulkan_Shader;

    //	// To be removed
public: // texture stuff
    auto create_texture_image(const std::string& path) -> void;
    auto create_image(
        v2u32 size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory) -> void;
    // auto copy_buffer_to_image(VkBuffer buffer, VkImage image, u32 width, u32 height) -> void;
    Image m_texture_image;
    // VkImage m_texture_image;
    VkDeviceMemory m_texture_image_Memory;

    // auto create_image_view(VkImage image, VkFormat format)->VkImageView;
    auto create_texture_image_view() -> void;
    auto create_texture_sampler() -> void;

    ImageView m_texture_image_view;
    // VkImageView m_texture_image_view;
    VkSampler m_texture_sampler;
};

} // namespace vulkan
} // namespace JadeFrame