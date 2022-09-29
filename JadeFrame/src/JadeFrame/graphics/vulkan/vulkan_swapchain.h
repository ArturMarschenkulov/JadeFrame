#pragma once
#include <vulkan/vulkan.h>

#include "JadeFrame/prelude.h"
#include "vulkan_buffer.h"

#include <vector>


namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanSurface;
namespace vulkan {
class Semaphore;
class Fence;
} // namespace vulkan
// class vulkan::Semaphore;
// class vulkan::Fence;
class VulkanRenderPass;
class VulkanImageView;


class VulkanRenderPass {
public:
    auto init(const VulkanLogicalDevice& device, VkFormat image_format) -> void;
    auto deinit() -> void;

public:
    VkRenderPass               m_handle = VK_NULL_HANDLE;
    const VulkanLogicalDevice* m_device = nullptr;
};

class VulkanFramebuffer {
public:
    auto init(
        const VulkanLogicalDevice& device, const VulkanImageView& image_view, const VulkanRenderPass& render_pass,
        VkExtent2D extent) -> void;
    auto deinit() -> void;

public:
    VkFramebuffer              m_handle;
    const VulkanLogicalDevice* m_device = nullptr;
    const VulkanImageView*     m_image_view = nullptr;
    const VulkanRenderPass*    m_render_pass = nullptr;
    // const VulkanSwapchain* m_swapchain = nullptr;
};


class VulkanSwapchain {
public:
    auto init(VulkanLogicalDevice& device, const VulkanSurface& surface) -> void;
    auto deinit() -> void;
    auto recreate() -> void;

    auto acquire_next_image(const vulkan::Semaphore* semaphore, const vulkan::Fence* fence, VkResult& result) -> u32;
    auto acquire_next_image(const vulkan::Semaphore* semaphore, const vulkan::Fence* fence) -> u32;

public:
    VkSwapchainKHR       m_handle = VK_NULL_HANDLE;
    VulkanLogicalDevice* m_device = nullptr;
    const VulkanSurface* m_surface = nullptr;

    std::vector<VulkanImage>       m_images;
    std::vector<VulkanImageView>   m_image_views;
    VulkanRenderPass               m_render_pass;
    std::vector<VulkanFramebuffer> m_framebuffers;

    VkFormat   m_image_format;
    VkExtent2D m_extent;

    bool m_is_recreated = false;
};
} // namespace JadeFrame