#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "JadeFrame/graphics/vulkan/queue.h"
#include "JadeFrame/prelude.h"
#include "buffer.h"
#include "surface.h"

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
    RenderPass(RenderPass&& other) noexcept;
    auto operator=(RenderPass&& other) noexcept -> RenderPass&;

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
    Framebuffer(Framebuffer&& other) noexcept;
    auto operator=(Framebuffer&& other) noexcept -> Framebuffer&;

public:
    Framebuffer(
        const LogicalDevice& device,
        const ImageView&     image_view,
        const ImageView&     depth_view,
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

/// Chooses the extent of the swapchain images for a surface with `capabilities`.
///
/// A surface that dictates its size reports it as `currentExtent`, which then has to be
/// used as is. Otherwise `currentExtent` is (0xFFFFFFFF, 0xFFFFFFFF) and the swapchain
/// determines the size: `framebuffer_extent`, the drawable area of the window in
/// physical pixels, with width and height each clamped to what the surface supports.
///
/// The result can be empty, most notably for a minimized window. Check it with
/// `is_valid_swapchain_extent` before creating a swapchain.
[[nodiscard]] auto choose_swapchain_extent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    VkExtent2D                      framebuffer_extent
) -> VkExtent2D;

/// Whether a swapchain may be created with `extent`: both dimensions must be non-zero.
[[nodiscard]] auto is_valid_swapchain_extent(VkExtent2D extent) -> bool;

/// While the surface has no drawable area (minimized window) there is no swapchain:
/// `m_handle` is `VK_NULL_HANDLE`, `m_extent` is empty and there are no images.
/// `recreate()` creates it once the surface is drawable again.
class Swapchain {
public:
    Swapchain() = default;
    ~Swapchain();
    Swapchain(const Swapchain&) = delete;
    auto operator=(const Swapchain&) -> Swapchain& = delete;
    Swapchain(Swapchain&& other) noexcept;
    auto operator=(Swapchain&& other) noexcept -> Swapchain&;

public:
    auto init(LogicalDevice& device, Window* window) -> void;
    auto deinit() -> void;
    auto recreate() -> void;

    /// Acquires the next image index from the swapchain.
    /// If a semaphore is provided, it will be signaled when the image is ready to be
    /// used. If a fence is provided, it will be signaled when the image is ready to be
    /// used.
    auto
    acquire_image_index(const Semaphore* semaphore, const Fence* fence, VkResult& result)
        -> u32;
    auto acquire_image_index(const Semaphore* semaphore, const Fence* fence) -> u32;

    [[nodiscard]] auto query_images() const -> std::vector<Image>;

public:
    VkSwapchainKHR m_handle = VK_NULL_HANDLE;
    LogicalDevice* m_device = nullptr;
    Window*        m_window = nullptr;
    Surface        m_surface;

    std::vector<Image>     m_images;
    std::vector<ImageView> m_image_views;

    VkFormat   m_image_format = VK_FORMAT_UNDEFINED;
    VkExtent2D m_extent = {};

    bool m_is_recreated = false;
    // Whether the images were created with VK_IMAGE_USAGE_TRANSFER_SRC_BIT and can
    // therefore be copied from (screenshots). The surface does not have to support it.
    bool m_supports_readback = false;

    Image     m_depth_image;
    ImageView m_depth_image_view;

    Queue m_present_queue;
};
} // namespace vulkan
} // namespace JadeFrame
