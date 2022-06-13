#pragma once

#include <vulkan/vulkan.h>
#include "vulkan_shared.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_buffer.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_sync_object.h"
#include "vulkan_command_buffers.h"


#include "JadeFrame/prelude.h"

#include <array>
#include <vector>

namespace JadeFrame {
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanPipeline;
class VulkanBuffer;

class VulkanQueue {
public:
    auto submit(const VkSubmitInfo& submit_info, const VulkanFence* p_fence) const -> void;
    auto submit(
        const VulkanCommandBuffer& cmd_buffer, const VulkanSemaphore* wait_semaphore,
        const VulkanSemaphore* signal_semaphore, const VulkanFence* p_fence) -> void;
    auto wait_idle() const -> void;
    auto present(VkPresentInfoKHR info, VkResult& result) const -> void;
    auto present(
        const u32& index, const VulkanSwapchain& swapchain, const VulkanSemaphore* result, VkResult* out_result) const
        -> void;

public:
    VkQueue m_handle = VK_NULL_HANDLE;
    // const VulkanQueueFamily* = nullptr;
};




class VulkanLogicalDevice {
private:
public:
    auto init(const VulkanInstance& instance, const VulkanPhysicalDevice& physical_device) -> void;
    auto deinit() -> void;

    auto query_queue(u32 queue_family_index, u32 queue_index) -> VulkanQueue;

public:
    VkDevice                    m_handle = VK_NULL_HANDLE;
    const VulkanInstance*       m_instance = nullptr;
    const VulkanPhysicalDevice* m_physical_device = nullptr;

    VulkanQueue m_graphics_queue;
    VulkanQueue m_present_queue;

public: // Swapchain stuff
    auto recreate_swapchain() -> void;
    auto cleanup_swapchain() -> void;

    VulkanSwapchain m_swapchain;
    // VulkanRenderPass m_render_pass;


    // TODO: Move the descriptor stuff to the shader code
public: // Descriptor set
    VulkanDescriptorPool      m_main_descriptor_pool;
    VulkanDescriptorSetLayout m_descriptor_set_layout_0;

    std::vector<VulkanDescriptorSet> m_descriptor_sets;
    VulkanBuffer                     m_ub_cam = {VulkanBuffer::TYPE::UNIFORM};
    VulkanBuffer                     m_ub_tran = {VulkanBuffer::TYPE::UNIFORM};

public:
    VulkanCommandPool                m_command_pool;
    std::vector<VulkanCommandBuffer> m_command_buffers;


public: // synchro objects
    std::vector<VulkanSemaphore> m_image_available_semaphores;
    std::vector<VulkanSemaphore> m_render_finished_semaphores;
    std::vector<VulkanFence>     m_in_flight_fences;
    std::vector<VulkanFence>     m_images_in_flight;

public: // Misc
    u32    m_present_image_index = 0;
    size_t m_current_frame = 0;
    bool   m_framebuffer_resized = false;




    //	// To be removed
public: // texture stuff
    auto create_texture_image(const std::string& path) -> void;
    auto create_image(
        v2u32 size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory) -> void;
    // auto copy_buffer_to_image(VkBuffer buffer, VkImage image, u32 width, u32 height) -> void;
    VulkanImage m_texture_image;
    // VkImage m_texture_image;
    VkDeviceMemory m_texture_image_Memory;

    // auto create_image_view(VkImage image, VkFormat format)->VkImageView;
    auto create_texture_image_view() -> void;
    auto create_texture_sampler() -> void;

    VulkanImageView m_texture_image_view;
    // VkImageView m_texture_image_view;
    VkSampler m_texture_sampler;
};
} // namespace JadeFrame