#pragma once
#include <vulkan/vulkan.h>

#include "JadeFrame/prelude.h"
#include "vulkan_buffer.h"

#include <vector>


namespace JadeFrame {


class VulkanInstance;

namespace vulkan {
class LogicalDevice;
class Semaphore;
class Fence;
class ImageView;

class PhysicalDevice;
class Surface;


class RenderPass;



class RenderPass {
public:
    auto init(const LogicalDevice& device, VkFormat image_format) -> void;
    auto deinit() -> void;

public:
    VkRenderPass         m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
};

class Framebuffer {
public:
    auto
    init(const LogicalDevice& device, const ImageView& image_view, const RenderPass& render_pass, VkExtent2D extent)
        -> void;
    auto deinit() -> void;

public:
    VkFramebuffer        m_handle;
    const LogicalDevice* m_device = nullptr;
    const ImageView*     m_image_view = nullptr;
    const RenderPass*    m_render_pass = nullptr;
    // const Swapchain* m_swapchain = nullptr;
};


class Swapchain {
public:
    auto init(LogicalDevice& device, const Surface& surface) -> void;
    auto deinit() -> void;
    auto recreate() -> void;

    auto acquire_next_image(const Semaphore* semaphore, const Fence* fence, VkResult& result) -> u32;
    auto acquire_next_image(const Semaphore* semaphore, const Fence* fence) -> u32;

public:
    VkSwapchainKHR m_handle = VK_NULL_HANDLE;
    LogicalDevice* m_device = nullptr;
    const Surface* m_surface = nullptr;

    std::vector<Image>       m_images;
    std::vector<ImageView>   m_image_views;
    RenderPass               m_render_pass;
    std::vector<Framebuffer> m_framebuffers;

    VkFormat   m_image_format;
    VkExtent2D m_extent;

    bool m_is_recreated = false;
};
} // namespace vulkan
} // namespace JadeFrame