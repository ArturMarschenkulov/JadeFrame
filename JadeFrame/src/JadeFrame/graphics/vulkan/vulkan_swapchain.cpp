#include "pch.h"
#include "vulkan_swapchain.h"

#if _WIN32
#include <Windows.h> // TODO: Try to remove it
#endif


#include "vulkan_logical_device.h"
#include "vulkan_context.h"
#include "vulkan_sync_object.h"
#include "JadeFrame/utils/assert.h"

#undef min
#undef max

namespace JadeFrame {

static auto choose_surface_format(const std::vector<VkSurfaceFormatKHR>& available_surface_formats)
    -> VkSurfaceFormatKHR {
    for (u32 i = 0; i < available_surface_formats.size(); i++) {
        if (available_surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_surface_formats[i];
        }
    }
    return available_surface_formats[0];
}
static auto choose_present_mode(const std::vector<VkPresentModeKHR>& available_surface_formats) -> VkPresentModeKHR {
    std::array<VkPresentModeKHR, 3> mode_ranks = {
        VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};
    for (u32 i = 0; i < available_surface_formats.size(); i++) {
        for (u32 j = 0; j < mode_ranks.size(); j++) {
            if (available_surface_formats[i] == mode_ranks[j]) {
                const VkPresentModeKHR best_mode = available_surface_formats[i];
                return best_mode;
            }
        }
    }
    assert(!"Should not reach here!");
    return {};
}
static auto choose_extent(const VkSurfaceCapabilitiesKHR& available_capabilities, const VulkanSwapchain& swapchain)
    -> VkExtent2D {
    // vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_surface, &m_surface_capabilities);
    if (false /*m_surface_capabilities.currentExtent.width != UINT32_MAX*/) {
        return available_capabilities.currentExtent;
    } else {
#ifdef _WIN32
        RECT        area;
        const HWND& wh = swapchain.m_surface->m_window_handle;
        GetClientRect(wh, &area);
        i32 width = area.right;
        i32 height = area.bottom;
        // glfwGetFramebufferSize(window, &width, &height);


        VkExtent2D actual_extent = {static_cast<u32>(width), static_cast<u32>(height)};

        // actual_extent.width = std::max(m_surface_capabilities.minImageExtent.width,
        // std::min(m_surface_capabilities.maxImageExtent.width, actual_extent.width)); actual_extent.height =
        // std::max(m_surface_capabilities.minImageExtent.height, std::min(m_surface_capabilities.maxImageExtent.height,
        // actual_extent.height));


        // actual_extent.width = std::clamp(actual_extent.width, m_surface_capabilities.minImageExtent.width,
        // m_surface_capabilities.maxImageExtent.width); actual_extent.height = std::clamp(actual_extent.height,
        // m_surface_capabilities.minImageExtent.height, m_surface_capabilities.maxImageExtent.height);

        return actual_extent;
#else
        assert(false && "not implemented yet");
        return {};
#endif
    }
}

/*---------------------------
        Render Pass
---------------------------*/

auto VulkanRenderPass::init(const VulkanLogicalDevice& device, VkFormat image_format) -> void {
    m_device = &device;
    VkResult result;

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

    const VkRenderPassCreateInfo render_pass_info = {
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

    result = vkCreateRenderPass(device.m_handle, &render_pass_info, nullptr, &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}
auto VulkanRenderPass::deinit() -> void { vkDestroyRenderPass(m_device->m_handle, m_handle, nullptr); }

/*---------------------------
        Swapchain
---------------------------*/

auto VulkanSwapchain::init(VulkanLogicalDevice& device, const VulkanSurface& surface) -> void {
    m_device = &device;
    m_surface = &surface;

    VkResult                     result;
    const VulkanPhysicalDevice*  gpu = device.m_physical_device;
    const SurfaceSupportDetails& surface_details = gpu->m_surface_support_details;

    u32 image_count = surface_details.m_capabilities.minImageCount + 1;
    if (surface_details.m_capabilities.maxImageCount > 0 &&
        image_count > surface_details.m_capabilities.maxImageCount) {
        image_count = surface_details.m_capabilities.maxImageCount;
    }

    const VkSurfaceFormatKHR surface_format = choose_surface_format(surface_details.m_formats);
    const VkPresentModeKHR   present_mode = choose_present_mode(surface_details.m_present_modes);
    const VkExtent2D         extent = choose_extent(surface_details.m_capabilities, *this);
    m_image_format = surface_format.format;
    m_extent = extent;

    const QueueFamilyIndices& indices = gpu->m_queue_family_indices;
    const u32 queue_family_indices[] = {indices.m_graphics_family.value(), indices.m_present_family.value()};

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.surface = surface.m_handle;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.preTransform = gpu->m_surface_support_details.m_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;
    if (indices.m_graphics_family != indices.m_present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    result = vkCreateSwapchainKHR(device.m_handle, &create_info, nullptr, &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");




    std::vector<VkImage> images;
    result = vkGetSwapchainImagesKHR(device.m_handle, m_handle, &image_count, nullptr);
    JF_ASSERT(result == VK_SUCCESS, "");
    images.resize(image_count);
    result = vkGetSwapchainImagesKHR(device.m_handle, m_handle, &image_count, images.data());
    JF_ASSERT(result == VK_SUCCESS, "");

    m_images.resize(image_count);
    for (u32 i = 0; i < m_images.size(); i++) { m_images[i].init(device, images[i]); }

    m_image_views.resize(image_count);
    for (u32 i = 0; i < m_images.size(); i++) { m_image_views[i].init(device, m_images[i], surface_format.format); }

    m_render_pass.init(device, m_image_format);

    m_framebuffers.resize(m_image_views.size());
    for (size_t i = 0; i < m_image_views.size(); i++) {
        m_framebuffers[i].init(*m_device, m_image_views[i], m_render_pass, m_extent);
    }
}

auto VulkanSwapchain::deinit() -> void {
    for (uint32_t i = 0; i < m_framebuffers.size(); i++) { m_framebuffers[i].deinit(); }
    m_render_pass.deinit();
    for (uint32_t i = 0; i < m_image_views.size(); i++) { m_image_views[i].deinit(); }

    vkDestroySwapchainKHR(m_device->m_handle, m_handle, nullptr);
}

auto VulkanSwapchain::recreate() -> void {
    vkDeviceWaitIdle(m_device->m_handle);
    this->deinit();

    this->init(*m_device, m_device->m_instance->m_surface);
    m_device->m_images_in_flight.resize(m_images.size());
}

auto VulkanSwapchain::acquire_next_image(
    const vulkan::Semaphore* semaphore, const vulkan::Fence* fence, VkResult& out_result) -> u32 {
    u32 image_index;
    out_result = vkAcquireNextImageKHR(
        m_device->m_handle,                                          // device
        m_handle,                                                    // swapchain
        UINT64_MAX,                                                  // timeout
        semaphore == nullptr ? VK_NULL_HANDLE : semaphore->m_handle, // semaphore
        fence == nullptr ? VK_NULL_HANDLE : fence->m_handle,         // fence
        &image_index                                                 // image index
    );
    return image_index;
}

auto VulkanSwapchain::acquire_next_image(const vulkan::Semaphore* semaphore, const vulkan::Fence* fence) -> u32 {
    VkResult result;
    u32      image_index;
    result = vkAcquireNextImageKHR(
        m_device->m_handle, m_handle, UINT64_MAX, semaphore == nullptr ? VK_NULL_HANDLE : semaphore->m_handle,
        fence == nullptr ? VK_NULL_HANDLE : fence->m_handle, &image_index);
    if (result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
            this->recreate();
            m_is_recreated = true;
        } else if (result == VK_SUBOPTIMAL_KHR) {
            std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
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

auto VulkanFramebuffer::init(
    const VulkanLogicalDevice& device, const VulkanImageView& image_view, const VulkanRenderPass& render_pass,
    VkExtent2D extent) -> void {
    m_device = &device;
    m_image_view = &image_view;
    m_render_pass = &render_pass;

    VkResult                   result;
    std::array<VkImageView, 1> attachments = {image_view.m_handle};

    const VkFramebufferCreateInfo framebuffer_info = {
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

    result = vkCreateFramebuffer(device.m_handle, &framebuffer_info, nullptr, &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

auto VulkanFramebuffer::deinit() -> void { vkDestroyFramebuffer(m_device->m_handle, m_handle, nullptr); }

} // namespace JadeFrame