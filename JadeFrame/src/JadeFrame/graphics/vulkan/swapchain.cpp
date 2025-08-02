#include "swapchain.h"

#if _WIN32
    #include <Windows.h> // TODO: Try to remove it
#endif
#if _WIN32
    #include "JadeFrame/platform/windows/windows_window.h"
#elif __linux__
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

#include "JadeFrame/utils/assert.h"
#include "logical_device.h"
#include "context.h"
#include "sync_object.h"

#undef min
#undef max

namespace JadeFrame {
namespace vulkan {

static auto
choose_surface_format(const std::span<VkSurfaceFormatKHR>& available_surface_formats
) -> VkSurfaceFormatKHR {
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
choose_present_mode(const std::span<VkPresentModeKHR>& available_surface_formats
) -> VkPresentModeKHR {
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
        RECT  area;
        auto& native_window = surface.m_window_handle->m_native_window;
        auto  win32_native_window =
            dynamic_cast<win32::NativeWindow*>(native_window.get());
        auto       window_handle = win32_native_window->m_window_handle;
        const HWND wh = window_handle;
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
        const auto* win = static_cast<const JadeFrame::X11_NativeWindow*>(
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
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_device(std::exchange(other.m_device, nullptr)) {}

auto RenderPass::operator=(RenderPass&& other) noexcept -> RenderPass& {
    m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
    m_device = std::exchange(other.m_device, nullptr);
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

    const VkAttachmentDescription depth_attachment = {
        .flags = {},
        .format = VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    const VkAttachmentReference depth_attachment_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass = {
        .flags = {},
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = {},
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pResolveAttachments = {},
        .pDepthStencilAttachment = &depth_attachment_ref,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = {},
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0,
    };

    bool dependency_with_depth = true;
    if (dependency_with_depth) {
        dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    std::array<VkAttachmentDescription, 2> attachments = {
        color_attachment, depth_attachment
    };
    const VkRenderPassCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = {},
        .flags = {},
        .attachmentCount = static_cast<u32>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkResult result =
        vkCreateRenderPass(device.m_handle, &info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

/*---------------------------
        Swapchain
---------------------------*/
static auto get_queue_family_with_present(
    std::span<QueueFamily> queue_families,
    const Surface&         surface
) -> QueueFamily* {
    for (auto& family : queue_families) {
        VkBool32 is_present = VK_FALSE;

        vkGetPhysicalDeviceSurfaceSupportKHR(
            family.m_physical_device->m_handle,
            family.m_index,
            surface.m_handle,
            &is_present
        );
        if (is_present == VK_TRUE) { return &family; }
    }
    return nullptr;
}

auto Swapchain::init(LogicalDevice& device, Window* window) -> void {
    m_device = &device;

    const PhysicalDevice* gpu = device.m_physical_device;

    m_surface = gpu->m_instance_p->create_surface(window);

    auto formats = gpu->query_surface_formats(m_surface);
    auto present_modes = gpu->query_surface_present_modes(m_surface);
    auto caps = gpu->query_surface_capabilities(m_surface);

    u32 image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
        image_count = caps.maxImageCount;
    }

    const VkSurfaceFormatKHR surface_format = choose_surface_format(formats);
    const VkPresentModeKHR   present_mode = choose_present_mode(present_modes);
    const VkExtent2D         extent = choose_extent(caps, m_surface);
    m_image_format = surface_format.format;
    m_extent = extent;

    const QueueFamilyPointers& pointers = gpu->m_chosen_queue_family_pointers;
    assert(pointers.m_graphics_family != nullptr && "graphics family is nullptr");
    assert(pointers.m_transfer_family != nullptr && "present family is nullptr");

    auto  queue_families = gpu->query_queue_families();
    auto* present_family = get_queue_family_with_present(queue_families, m_surface);
    if (present_family == nullptr) {
        throw std::runtime_error("no queue family supports present");
    }
    m_present_queue = present_family->query_queues(device, 0);

    const std::array<u32, 2> queue_family_indices = {
        pointers.m_graphics_family->m_index, present_family->m_index
    };
    const bool is_same_queue_family =
        pointers.m_graphics_family->m_index == present_family->m_index;

    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.pNext = nullptr;
    info.flags = 0;
    info.surface = m_surface.m_handle;
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
        m_image_views[i] = device.create_image_view(
            m_images[i], surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT
        );
    }

    auto depth_format = VK_FORMAT_D32_SFLOAT;
    m_depth_image = Image(
        device,
        v2u32::create(extent.width, extent.height),
        depth_format,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    m_depth_image_view =
        ImageView(device, m_depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

auto Swapchain::query_images() const -> std::vector<Image> {
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
    m_device->wait_until_idle();
    assert("not implemented yet");
    // this->deinit();

    // this->init(*m_device, m_surface);
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
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_device(std::exchange(other.m_device, nullptr))
    , m_image_view(std::exchange(other.m_image_view, nullptr))
    , m_render_pass(std::exchange(other.m_render_pass, nullptr)) {}

auto Framebuffer::operator=(Framebuffer&& other) noexcept -> Framebuffer& {
    if (&other == this) { return *this; }
    m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
    m_device = std::exchange(other.m_device, nullptr);
    m_image_view = std::exchange(other.m_image_view, nullptr);
    m_render_pass = std::exchange(other.m_render_pass, nullptr);
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
    const ImageView&     depth_view,
    const RenderPass&    render_pass,
    VkExtent2D           extent
)
    : m_device(&device)
    , m_image_view(&image_view)
    , m_render_pass(&render_pass) {
    std::array<VkImageView, 2> attachments = {image_view.m_handle, depth_view.m_handle};

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