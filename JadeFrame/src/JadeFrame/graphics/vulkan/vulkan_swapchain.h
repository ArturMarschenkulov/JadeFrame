#pragma once
#include <vulkan/vulkan.h>

#include "JadeFrame/prelude.h"
#include "vulkan_buffer.h"

#include <vector>

namespace JadeFrame {

namespace vulkan {
class Instance;
class LogicalDevice;
class Semaphore;
class Fence;
class ImageView;

class PhysicalDevice;
class Surface;

class RenderPass;

class RenderPass {
public:
    RenderPass() = default;
    ~RenderPass();
    RenderPass(const RenderPass&) = delete;
    auto operator=(const RenderPass&) -> RenderPass& = delete;
    RenderPass(RenderPass&& other);
    auto operator=(RenderPass&& other) -> RenderPass&;

    RenderPass(const LogicalDevice& device, VkFormat image_format);

public:
    VkRenderPass         m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
};

class Framebuffer {
public:
    Framebuffer() = default;
    ~Framebuffer();
    Framebuffer(const Framebuffer&) = delete;
    auto operator=(const Framebuffer&) -> Framebuffer& = delete;
    Framebuffer(Framebuffer&& other);
    auto operator=(Framebuffer&& other) -> Framebuffer&;

public:
    Framebuffer(
        const LogicalDevice& device,
        const ImageView&     image_view,
        const RenderPass&    render_pass,
        VkExtent2D           extent
    );

public:
    VkFramebuffer        m_handle = VK_NULL_HANDLE;
    const LogicalDevice* m_device = nullptr;
    const ImageView* m_image_view = nullptr; // TODO: Find out whether this is even needed
    const RenderPass* m_render_pass = nullptr;
    // const Swapchain* m_swapchain = nullptr;
};

class Swapchain {
public:
    Swapchain() = default;
    ~Swapchain() = default;
    Swapchain(const Swapchain&) = delete;
    auto operator=(const Swapchain&) -> Swapchain& = delete;
    Swapchain(Swapchain&& other) = default;
    auto operator=(Swapchain&& other) -> Swapchain& = default;

public:
    auto init(LogicalDevice& device, const Surface& surface) -> void;
    auto deinit() -> void;
    auto recreate() -> void;

    auto
    acquire_image_index(const Semaphore* semaphore, const Fence* fence, VkResult& result)
        -> u32;
    auto acquire_image_index(const Semaphore* semaphore, const Fence* fence) -> u32;

    auto query_images() -> std::vector<Image>;

public:
    VkSwapchainKHR m_handle = VK_NULL_HANDLE;
    LogicalDevice* m_device = nullptr;
    const Surface* m_surface = nullptr;

    std::vector<Image>     m_images;
    std::vector<ImageView> m_image_views;

    VkFormat   m_image_format;
    VkExtent2D m_extent;

    bool m_is_recreated = false;
};
} // namespace vulkan
} // namespace JadeFrame