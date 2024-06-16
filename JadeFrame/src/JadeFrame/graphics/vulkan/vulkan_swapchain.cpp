#include "pch.h"
#include "vulkan_swapchain.h"

#if _WIN32
    #include <Windows.h> // TODO: Try to remove it
#endif

#include "vulkan_logical_device.h"
#include "vulkan_context.h"
#include "vulkan_sync_object.h"
#include "JadeFrame/utils/assert.h"
#if _WIN32
    #include "JadeFrame/platform/windows/windows_window.h"
#elif __linux__
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

#undef min
#undef max

namespace JadeFrame {
namespace vulkan {

static auto
choose_surface_format(const std::span<VkSurfaceFormatKHR>& available_surface_formats)
    -> VkSurfaceFormatKHR {
    for (u32 i = 0; i < available_surface_formats.size(); i++) {
        auto& format = available_surface_formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return available_surface_formats[0];
}

static auto
choose_present_mode(const std::span<VkPresentModeKHR>& available_surface_formats)
    -> VkPresentModeKHR {
    std::array<VkPresentModeKHR, 3> mode_ranks = {
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR
    };
    for (u32 i = 0; i < available_surface_formats.size(); i++) {
        for (u32 j = 0; j < mode_ranks.size(); j++) {
            const VkPresentModeKHR& format = available_surface_formats[i];
            if (format == mode_ranks[j]) {
                const VkPresentModeKHR best_mode = format;
                return best_mode;
            }
        }
    }
    assert(false && "Should not reach here!");
    return {};
}

static auto choose_extent(
    const VkSurfaceCapabilitiesKHR& available_capabilities,
    const Surface&                  surface
) -> VkExtent2D {
    // vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_surface,
    // &m_surface_capabilities);
    if (false /*m_surface_capabilities.currentExtent.width != UINT32_MAX*/) {
        return available_capabilities.currentExtent;
    } else {
#ifdef _WIN32
        RECT       area;
        const HWND wh = (const HWND)surface.m_window_handle->get();
        GetClientRect(wh, &area);
        i32 width = area.right;
        i32 height = area.bottom;
        // glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {static_cast<u32>(width), static_cast<u32>(height)};

        // actual_extent.width = std::max(m_surface_capabilities.minImageExtent.width,
        // std::min(m_surface_capabilities.maxImageExtent.width, actual_extent.width));
        // actual_extent.height = std::max(m_surface_capabilities.minImageExtent.height,
        // std::min(m_surface_capabilities.maxImageExtent.height, actual_extent.height));

        // actual_extent.width = std::clamp(actual_extent.width,
        // m_surface_capabilities.minImageExtent.width,
        // m_surface_capabilities.maxImageExtent.width); actual_extent.height =
        // std::clamp(actual_extent.height, m_surface_capabilities.minImageExtent.height,
        // m_surface_capabilities.maxImageExtent.height);

        return actual_extent;
#elif __linux__
        const auto* win = static_cast<const JadeFrame::Linux_Window*>(
            surface.m_window_handle->m_native_window.get()
        );
        return VkExtent2D{win->m_size.x, win->m_size.y};
#else
        assert(false && "not implemented yet");
        return {};
#endif
    }
}

/*---------------------------
        Render Pass
---------------------------*/

RenderPass::RenderPass(RenderPass&& other) noexcept
    : m_handle(other.m_handle)
    , m_device(other.m_device) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
}

auto RenderPass::operator=(RenderPass&& other) noexcept -> RenderPass& {
    m_handle = other.m_handle;
    m_device = other.m_device;
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    return *this;
}

RenderPass::~RenderPass() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device->m_handle, m_handle, nullptr);
    }
}

RenderPass::RenderPass(const LogicalDevice& device, VkFormat image_format)
    : m_device(&device) {

    const VkAttachmentDescription color_attachment = {
        .flags = {},
        .format = image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass = {
        .flags = {},
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = {},
        .pInputAttachments = {},
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pResolveAttachments = {},
        .pDepthStencilAttachment = {},
        .preserveAttachmentCount = {},
        .pPreserveAttachments = {},
    };

    const VkRenderPassCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = {},
        .flags = {},
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = {},
        .pDependencies = {},
    };

    VkResult result =
        vkCreateRenderPass(device.m_handle, &info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

/*---------------------------
        Swapchain
---------------------------*/

auto Swapchain::init(LogicalDevice& device, const Surface& surface) -> void {
    m_device = &device;
    m_surface = &surface;

    const PhysicalDevice* gpu = device.m_physical_device;

    auto formats = gpu->query_surface_formats(surface);
    auto present_modes = gpu->query_surface_present_modes(surface);
    auto caps = gpu->query_surface_capabilities(surface);

    u32 image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
        image_count = caps.maxImageCount;
    }

    const VkSurfaceFormatKHR surface_format = choose_surface_format(formats);
    const VkPresentModeKHR   present_mode = choose_present_mode(present_modes);
    const VkExtent2D         extent = choose_extent(caps, surface);
    m_image_format = surface_format.format;
    m_extent = extent;

    const QueueFamilyPointers& pointers = gpu->m_chosen_queue_family_pointers;
    assert(pointers.m_graphics_family != nullptr && "graphics family is nullptr");
    assert(pointers.m_present_family != nullptr && "present family is nullptr");

    const std::array<u32, 2> queue_family_indices = {
        pointers.m_graphics_family->m_index, pointers.m_present_family->m_index
    };
    const bool is_same_queue_family =
        pointers.m_graphics_family == pointers.m_present_family;

    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.pNext = nullptr;
    info.flags = 0;
    info.surface = surface.m_handle;
    info.minImageCount = image_count;
    info.imageFormat = surface_format.format;
    info.imageColorSpace = surface_format.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices = nullptr;
    info.preTransform = caps.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = present_mode;
    info.clipped = VK_TRUE;
    info.oldSwapchain = VK_NULL_HANDLE;
    if (!is_same_queue_family) {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queue_family_indices.data();
    } else {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    VkResult result =
        vkCreateSwapchainKHR(device.m_handle, &info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");

    m_images = this->query_images();
    assert(m_images.size() == image_count);
    m_image_views.resize(m_images.size());
    for (u32 i = 0; i < m_images.size(); i++) {
        m_image_views[i] = device.create_image_view(m_images[i], surface_format.format);
    }
}

auto Swapchain::query_images() -> std::vector<Image> {
    u32 image_count = 0;

    vkGetSwapchainImagesKHR(m_device->m_handle, m_handle, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    images.resize(image_count);
    vkGetSwapchainImagesKHR(m_device->m_handle, m_handle, &image_count, images.data());

    std::vector<Image> result;
    result.resize(images.size());
    for (u32 i = 0; i < image_count; i++) { result[i] = Image(*m_device, images[i]); }

    return result;
}

auto Swapchain::deinit() -> void {
    vkDestroySwapchainKHR(m_device->m_handle, m_handle, nullptr);
}

auto Swapchain::recreate() -> void {
    vkDeviceWaitIdle(m_device->m_handle);
    this->deinit();

    this->init(*m_device, m_device->m_instance->m_surface);
}

auto Swapchain::acquire_image_index(
    const Semaphore* semaphore,
    const Fence*     fence,
    VkResult&        out_result
) -> u32 {
    u32         image_index = 0;
    VkSemaphore p_semaphore = semaphore == nullptr ? VK_NULL_HANDLE : semaphore->m_handle;
    VkFence     p_fence = fence == nullptr ? VK_NULL_HANDLE : fence->m_handle;

    out_result = vkAcquireNextImageKHR(
        m_device->m_handle, // device
        m_handle,           // swapchain
        UINT64_MAX,         // timeout
        p_semaphore,        // semaphore
        p_fence,            // fence
        &image_index        // image index
    );
    return image_index;
}

auto Swapchain::acquire_image_index(const Semaphore* semaphore, const Fence* fence)
    -> u32 {

    u32         image_index = 0;
    VkSemaphore p_semaphore = semaphore == nullptr ? VK_NULL_HANDLE : semaphore->m_handle;
    VkFence     p_fence = fence == nullptr ? VK_NULL_HANDLE : fence->m_handle;

    VkResult result = vkAcquireNextImageKHR(
        m_device->m_handle, // device
        m_handle,           // swapchain
        UINT64_MAX,         // timeout
        p_semaphore,        // semaphore
        p_fence,            // fence
        &image_index        // image index
    );
    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Logger::debug("recreate because of vkAcquireNextImageKHR");
            this->recreate();
            m_is_recreated = true;
        } else if (result == VK_SUBOPTIMAL_KHR) {
            Logger::debug("suboptimal");
            // this->recreate();
        } else {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
    }
    return image_index;
}

/*---------------------------
        Framebuffer
---------------------------*/

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_handle(other.m_handle)
    , m_device(other.m_device)
    , m_image_view(other.m_image_view)
    , m_render_pass(other.m_render_pass) {
    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_image_view = nullptr;
    other.m_render_pass = nullptr;
}

auto Framebuffer::operator=(Framebuffer&& other) noexcept -> Framebuffer& {
    if (&other == this) { return *this; }
    m_handle = other.m_handle;
    m_device = other.m_device;
    m_image_view = other.m_image_view;
    m_render_pass = other.m_render_pass;

    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_image_view = nullptr;
    other.m_render_pass = nullptr;

    return *this;
}

Framebuffer::~Framebuffer() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(m_device->m_handle, m_handle, nullptr);
    }
}

Framebuffer::Framebuffer(
    const LogicalDevice& device,
    const ImageView&     image_view,
    const RenderPass&    render_pass,
    VkExtent2D           extent
)
    : m_device(&device)
    , m_image_view(&image_view)
    , m_render_pass(&render_pass) {

    std::array<VkImageView, 1> attachments = {image_view.m_handle};

    const VkFramebufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderPass = render_pass.m_handle,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };

    VkResult result =
        vkCreateFramebuffer(device.m_handle, &info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

} // namespace vulkan
} // namespace JadeFrame