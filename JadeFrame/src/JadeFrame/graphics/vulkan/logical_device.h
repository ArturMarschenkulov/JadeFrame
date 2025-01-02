#pragma once

#include <vulkan/vulkan.h>

#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"

#include "swapchain.h"
#include "physical_device.h"
#include "pipeline.h"
#include "buffer.h"
#include "shader.h"
#include "descriptor_set.h"
#include "command_buffer.h"
#include "queue.h"
#include "sync_object.h"
#include "surface.h"


// #include "JadeFrame/prelude.h"

namespace JadeFrame {

class Vulkan_Renderer;

namespace vulkan {
class Instance;
class LogicalDevice;
class Pipeline;
class PhysicalDevice;
class Fence;
class Semaphore;

class LogicalDevice {
private:

public:
    LogicalDevice() = default;
    ~LogicalDevice();

    LogicalDevice(const LogicalDevice&) = delete;
    auto operator=(const LogicalDevice&) -> LogicalDevice& = delete;

    LogicalDevice(LogicalDevice&& other) noexcept;
    auto operator=(LogicalDevice&& other) noexcept -> LogicalDevice&;

    auto init(const Instance& instance, const PhysicalDevice& physical_device) -> void;
    auto deinit() -> void;

    auto wait_until_idle() const -> void;

public:
    VkDevice              m_handle = VK_NULL_HANDLE;
    const Instance*       m_instance = nullptr;
    const PhysicalDevice* m_physical_device = nullptr;

public:
    Queue m_graphics_queue;

public:
    auto create_command_pool(QueueFamily& queue_family) -> CommandPool;

    CommandPool m_command_pool;

    auto create_descriptor_pool(
        u32                                    max_sets,
        const std::span<VkDescriptorPoolSize>& pool_sizes
    ) -> DescriptorPool;
    auto create_descriptor_set_layout(
        const std::span<vulkan::DescriptorSetLayout::Binding>& bindings
    ) const -> DescriptorSetLayout;
    DescriptorPool m_set_pool;

public: // Swapchain stuff
    auto create_swapchain(const Window* window) -> Swapchain;
    auto create_render_pass(VkFormat image_format) -> RenderPass;
    auto create_framebuffer(
        const ImageView&  image_view,
        const ImageView&  depth_view,
        const RenderPass& render_pass,
        VkExtent2D        extent
    ) -> Framebuffer;

public:
    // auto create_image() -> vulkan::Image;
    auto create_image_view(Image& image, VkFormat, VkImageAspectFlagBits aspect_flags)
        -> ImageView;

public: // synchro objects
    auto create_semaphore() -> Semaphore;
    auto create_fence(bool signaled) -> Fence;
    auto wait_for_fence(const Fence& fences, bool wait_all, u64 timeout) const -> void;
    auto wait_for_fences(const std::span<Fence>& fences, bool wait_all, u64 timeout) const
        -> void;

public: // Misc

public:
    auto create_buffer(Buffer::TYPE buffer_type, void* data, size_t size) const
        -> Buffer*;
    auto destroy_buffer(Buffer* buffer) const -> void;

    auto create_shader(const Vulkan_Renderer& renderer, const Vulkan_Shader::Desc& desc)
        -> Vulkan_Shader;

public:
    // template<typename T, typename U>
    // using HashMap = std::unordered_map<T, U>;
    mutable std::unordered_map<u32, vulkan::Buffer> m_buffers;

    VmaAllocator m_vma_allocator = VK_NULL_HANDLE;
};

} // namespace vulkan
} // namespace JadeFrame