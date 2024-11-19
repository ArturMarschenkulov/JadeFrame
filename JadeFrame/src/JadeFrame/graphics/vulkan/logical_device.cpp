#include "pch.h"
#include "logical_device.h"
#include "physical_device.h"
#include "context.h"
#include "buffer.h"
#include "sync_object.h"

#include "JadeFrame/utils/assert.h"

#include <set>
#include <cassert>
#include <array>

namespace JadeFrame {

static auto VkResult_to_string(VkResult x) {
    std::string str;
#define JF_SET_ENUM_STRING(str, name)                                                    \
    case name: str = #name; break

    switch (x) {
        JF_SET_ENUM_STRING(str, VK_SUCCESS);
        JF_SET_ENUM_STRING(str, VK_NOT_READY);
        JF_SET_ENUM_STRING(str, VK_TIMEOUT);
        JF_SET_ENUM_STRING(str, VK_EVENT_SET);
        JF_SET_ENUM_STRING(str, VK_EVENT_RESET);
        JF_SET_ENUM_STRING(str, VK_INCOMPLETE);
        JF_SET_ENUM_STRING(str, VK_SUBOPTIMAL_KHR);
        JF_SET_ENUM_STRING(str, VK_ERROR_OUT_OF_DATE_KHR);
        default:
            JF_ASSERT(false, "Unknown VkResult");
            str = "";
            break;
    }
    return str;
#undef JF_SET_ENUM_STRING
}

/*---------------------------
        Queue
---------------------------*/

namespace vulkan {
Queue::Queue(const LogicalDevice& device, u32 queue_family_index, u32 queue_index) {
    vkGetDeviceQueue(device.m_handle, queue_family_index, queue_index, &m_handle);
}

auto Queue::submit(const CommandBuffer& cmd_buffer) const -> void {
    assert(cmd_buffer.m_stage == CommandBuffer::STAGE::EXCECUTABLE);
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer.m_handle,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };
    VkResult result = vkQueueSubmit(m_handle, 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) { assert(false); }
    cmd_buffer.m_stage = CommandBuffer::STAGE::PENDING;

    // this->submit(cmd_buffer, nullptr, nullptr, nullptr);
}

auto Queue::submit(
    const CommandBuffer& cmd_buffer,
    const Semaphore*     wait_semaphore,
    const Semaphore*     signal_semaphore,
    const Fence*         fence
) const -> void {
    assert(cmd_buffer.m_stage == CommandBuffer::STAGE::EXCECUTABLE);

    const bool has_fenc = fence != nullptr;
    const bool has_wait_semaphore = wait_semaphore != nullptr;
    const bool has_signal_semaphore = signal_semaphore != nullptr;

    VkFence fence_handle = has_fenc ? fence->m_handle : nullptr;

    std::array<VkPipelineStageFlags, 1> wait_stages = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    const VkSubmitInfo info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = has_wait_semaphore ? 1_u32 : 0,
        .pWaitSemaphores = has_wait_semaphore ? &wait_semaphore->m_handle : nullptr,
        .pWaitDstStageMask = wait_stages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer.m_handle,
        .signalSemaphoreCount = has_signal_semaphore ? 1_u32 : 0,
        .pSignalSemaphores = has_signal_semaphore ? &signal_semaphore->m_handle : nullptr,
    };
    VkResult result = vkQueueSubmit(m_handle, 1, &info, fence_handle);
    if (result != VK_SUCCESS) { JF_ASSERT(false, to_string(result)); }
    cmd_buffer.m_stage = CommandBuffer::STAGE::PENDING;
}

auto Queue::wait_idle() const -> void {
    VkResult result = VK_SUCCESS;
    result = vkQueueWaitIdle(m_handle);
    if (result != VK_SUCCESS) { assert(false); }
}

auto Queue::present(VkPresentInfoKHR info) const -> VkResult {
    VkResult result = vkQueuePresentKHR(m_handle, &info);
    if (result != VK_SUCCESS) { assert(false); }
    return result;
}

auto Queue::present(
    const u32&       index,
    const Swapchain& swapchain,
    const Semaphore* semaphore
) const -> VkResult {

    const bool has_semaphore = semaphore != nullptr;

    const VkPresentInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = has_semaphore ? 1_u32 : 0_u32,
        .pWaitSemaphores = has_semaphore ? &semaphore->m_handle : VK_NULL_HANDLE,
        .swapchainCount = 1,
        .pSwapchains = &swapchain.m_handle,
        .pImageIndices = &index,
        .pResults = nullptr,
    };
    return vkQueuePresentKHR(m_handle, &info);
}

/*---------------------------
        Logical Device
---------------------------*/

LogicalDevice::~LogicalDevice() {
    if (m_handle != VK_NULL_HANDLE) { deinit(); }
}

LogicalDevice::LogicalDevice(LogicalDevice&& other) noexcept {
    m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
    m_instance = std::exchange(other.m_instance, nullptr);
    m_physical_device = std::exchange(other.m_physical_device, nullptr);
}

auto LogicalDevice::operator=(LogicalDevice&& other) noexcept -> LogicalDevice& {
    if (this != &other) {
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_instance = std::exchange(other.m_instance, nullptr);
        m_physical_device = std::exchange(other.m_physical_device, nullptr);
    }
    return *this;
}

static auto init_vma(
    const Instance&       instance,
    const PhysicalDevice& physical_device,
    const LogicalDevice&  logical_device

) -> VmaAllocator {

    VmaVulkanFunctions vma_vk_fns = {};
    vma_vk_fns.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vma_vk_fns.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo vma_info = {};
    vma_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    vma_info.vulkanApiVersion = VK_API_VERSION_1_2;
    vma_info.physicalDevice = physical_device.m_handle;
    vma_info.device = logical_device.m_handle;
    vma_info.instance = instance.m_instance;
    vma_info.pVulkanFunctions = &vma_vk_fns;

    VmaAllocator vma_allocator = nullptr;
    vmaCreateAllocator(&vma_info, &vma_allocator);
    return vma_allocator;
}

auto LogicalDevice::init(const Instance& instance, const PhysicalDevice& physical_device)
    -> void {
    m_physical_device = &physical_device;
    m_instance = &instance;

    VkResult result = VK_SUCCESS;

    const QueueFamilyPointers& pointers = physical_device.m_chosen_queue_family_pointers;
    const std::set<QueueFamily*> unique_queue_families = {
        pointers.m_graphics_family, pointers.m_transfer_family
    };

    constexpr f32                        queue_priority = 1.0_f32;
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (const auto& queue_family : unique_queue_families) {
        const VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_family->m_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };
        queue_create_infos.push_back(queue_create_info);
    }

    const VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = 0,         // this is deprecated and ignored
        .ppEnabledLayerNames = nullptr, // this is deprecated and ignored
        .enabledExtensionCount =
            static_cast<u32>(physical_device.m_device_extensions.size()),
        .ppEnabledExtensionNames = physical_device.m_device_extensions.data(),
        .pEnabledFeatures = &physical_device.m_features,
    };

    result = vkCreateDevice(
        physical_device.m_handle, &create_info, Instance::allocator(), &m_handle
    );
    if (result != VK_SUCCESS) { assert(false); }

    // TODO: Maybe the VMA allocator should be created somewhere else, but for now it is
    // here.
    m_vma_allocator = init_vma(instance, physical_device, *this);

    Logger::debug(
        "maxBoundDescriptorSets: {}",
        m_physical_device->m_properties.limits.maxBoundDescriptorSets
    );
    JF_ASSERT(
        m_physical_device->m_properties.limits.maxBoundDescriptorSets >= 4,
        "maxBoundDescriptorSets too low, it must be at least 4"
    );
    m_graphics_queue = pointers.m_graphics_family->query_queues(*this, 0);

    m_command_pool = this->create_command_pool(
        *m_physical_device->m_chosen_queue_family_pointers.m_graphics_family
    );
    const u32                         descriptor_count = 1000;
    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor_count},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor_count},
        {  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptor_count},
        {        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor_count},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptor_count},
        {  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptor_count},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor_count},
        {               VK_DESCRIPTOR_TYPE_SAMPLER, descriptor_count},
        {         VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptor_count},
        {         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptor_count},
    };
    m_set_pool = this->create_descriptor_pool(4, pool_sizes);
}

auto LogicalDevice::wait_until_idle() const -> void {
    VkResult result = VK_SUCCESS;
    result = vkDeviceWaitIdle(m_handle);
    if (result != VK_SUCCESS) { assert(false); }
}

auto LogicalDevice::deinit() -> void {
    this->wait_until_idle();
    vmaDestroyAllocator(m_vma_allocator);
    vkDestroyDevice(m_handle, nullptr);
}

auto LogicalDevice::wait_for_fence(const Fence& fences, bool wait_all, u64 timeout) const
    -> void {
    VkResult result = VK_SUCCESS;
    result = vkWaitForFences(
        m_handle, 1, &fences.m_handle, static_cast<VkBool32>(wait_all), timeout
    );
    if (result != VK_SUCCESS) { assert(false); }
}

auto LogicalDevice::wait_for_fences(
    const std::span<Fence>& fences,
    bool                    wait_all,
    u64                     timeout
) const -> void {
    constexpr auto max_fences = 5;
    assert(fences.size() < max_fences);
    std::array<VkFence, max_fences> vk_fences = {};
    for (u32 i = 0; i < fences.size(); ++i) { vk_fences[i] = fences[i].m_handle; }
    VkResult result = VK_SUCCESS;
    result = vkWaitForFences(
        m_handle,
        static_cast<u32>(fences.size()),
        vk_fences.data(),
        static_cast<VkBool32>(wait_all),
        timeout
    );
    if (result != VK_SUCCESS) { assert(false); }
}

auto LogicalDevice::create_buffer(Buffer::TYPE buffer_type, void* data, size_t size) const
    -> Buffer* {
    static u32 id = 0;

    m_buffers[id] = Buffer(*this, buffer_type, data, size);
    id++;
    return &m_buffers[id - 1];
}

auto LogicalDevice::destroy_buffer(Buffer* buffer) const -> void {
    for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it) {
        if (&it->second == buffer) {
            m_buffers.erase(it);
            return;
        }
    }
}

auto LogicalDevice::create_descriptor_pool(
    u32                                    max_sets,
    const std::span<VkDescriptorPoolSize>& pool_sizes
) -> DescriptorPool {
    DescriptorPool pool(*this, max_sets, pool_sizes);
    return pool;
}

auto LogicalDevice::create_descriptor_set_layout(
    const std::span<vulkan::DescriptorSetLayout::Binding>& bindings
) const -> DescriptorSetLayout {
    DescriptorSetLayout layout(*this, bindings);
    return layout;
}

auto LogicalDevice::create_shader(
    const Vulkan_Renderer&     renderer,
    const Vulkan_Shader::Desc& desc
) -> Vulkan_Shader {
    return {*this, renderer, desc};
}

auto LogicalDevice::create_semaphore() -> Semaphore {
    Semaphore semaphore(*this);
    return semaphore;
}

auto LogicalDevice::create_fence(bool signaled) -> Fence {
    Fence fence(*this, signaled);
    return fence;
}

auto LogicalDevice::create_command_pool(QueueFamily& queue_family) -> CommandPool {
    CommandPool cp(*this, queue_family);
    return cp;
}

auto LogicalDevice::create_swapchain(const Window* window) -> Swapchain {
    Swapchain sc;
    sc.init(*this, window);
    return sc;
}

auto LogicalDevice::create_image_view(
    Image&                image,
    VkFormat              format,
    VkImageAspectFlagBits aspect_flags
) -> ImageView {
    ImageView iv(*this, image, format, aspect_flags);
    return iv;
}

auto LogicalDevice::create_render_pass(VkFormat image_format) -> RenderPass {
    RenderPass rp(*this, image_format);
    return rp;
}

auto LogicalDevice::create_framebuffer(
    const ImageView&  image_view,
    const ImageView&  depth_view,
    const RenderPass& render_pass,
    VkExtent2D        extent
) -> Framebuffer {
    Framebuffer fb(*this, image_view, depth_view, render_pass, extent);
    return fb;
}

} // namespace vulkan
} // namespace JadeFrame