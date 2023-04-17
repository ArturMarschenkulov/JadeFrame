#include "pch.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_context.h"
#include "vulkan_buffer.h"
#include "vulkan_sync_object.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/math.h"
#include "JadeFrame/utils/assert.h"

#include <thread>
#include <future>
#include <set>
#include <cassert>
#include <array>


namespace JadeFrame {


static auto VkResult_to_string(VkResult x) {
    std::string str;
#define JF_SET_ENUM_STRING(str, name)                                                                                  \
    case name: str = #name

    switch (x) {
        JF_SET_ENUM_STRING(str, VK_SUCCESS);
        break;
        JF_SET_ENUM_STRING(str, VK_NOT_READY);
        break;
        JF_SET_ENUM_STRING(str, VK_TIMEOUT);
        break;
        JF_SET_ENUM_STRING(str, VK_EVENT_SET);
        break;
        JF_SET_ENUM_STRING(str, VK_EVENT_RESET);
        break;
        JF_SET_ENUM_STRING(str, VK_INCOMPLETE);
        break;
        JF_SET_ENUM_STRING(str, VK_SUBOPTIMAL_KHR);
        break;
        JF_SET_ENUM_STRING(str, VK_ERROR_OUT_OF_DATE_KHR);
        break;
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
auto Queue::submit(const VkSubmitInfo& submit_info, const Fence* p_fence) const -> void {
    VkResult result;
    result = vkQueueSubmit(m_handle, 1, &submit_info, p_fence ? p_fence->m_handle : nullptr);
    if (result != VK_SUCCESS) assert(false);
}
auto Queue::submit(
    const CommandBuffer& cmd_buffer, const Semaphore* wait_semaphore, const Semaphore* signal_semaphore,
    const Fence* fence) const -> void {

    VkFence fence_handle = fence ? fence->m_handle : 0;

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSubmitInfo   submit_info = {
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .pNext = nullptr,
          .waitSemaphoreCount = wait_semaphore ? 1_u32 : 0,
          .pWaitSemaphores = wait_semaphore ? &wait_semaphore->m_handle : 0,
          .pWaitDstStageMask = wait_stages,
          .commandBufferCount = 1,
          .pCommandBuffers = &cmd_buffer.m_handle,
          .signalSemaphoreCount = signal_semaphore ? 1_u32 : 0,
          .pSignalSemaphores = signal_semaphore ? &signal_semaphore->m_handle : 0,
    };
    VkResult result;
    result = vkQueueSubmit(m_handle, 1, &submit_info, fence_handle);
    if (result != VK_SUCCESS) JF_ASSERT(false, to_string(result));
    cmd_buffer.m_stage = CommandBuffer::STAGE::PENDING;
}
auto Queue::wait_idle() const -> void {
    VkResult result;
    result = vkQueueWaitIdle(m_handle);
    if (result != VK_SUCCESS) assert(false);
}
auto Queue::present(VkPresentInfoKHR info) const -> VkResult {
    VkResult result = vkQueuePresentKHR(m_handle, &info);
    if (result != VK_SUCCESS) assert(false);
    return result;
}

auto Queue::present(const u32& index, const Swapchain& swapchain, const Semaphore* semaphore) const -> VkResult {
    const VkPresentInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = semaphore ? 1_u32 : 0_u32,
        .pWaitSemaphores = semaphore ? &semaphore->m_handle : VK_NULL_HANDLE,
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
LogicalDevice::LogicalDevice(LogicalDevice&& other) {
    m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
    m_instance = std::exchange(other.m_instance, nullptr);
    m_physical_device = std::exchange(other.m_physical_device, nullptr);
}
auto LogicalDevice::operator=(LogicalDevice&& other) -> LogicalDevice& {
    if (this != &other) {
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_instance = std::exchange(other.m_instance, nullptr);
        m_physical_device = std::exchange(other.m_physical_device, nullptr);
    }
    return *this;
}


auto LogicalDevice::wait_for_fence(const Fence& fences, bool wait_all, u64 timeout) const -> void {
    VkResult result;
    result = vkWaitForFences(m_handle, 1, &fences.m_handle, wait_all, timeout);
    if (result != VK_SUCCESS) assert(false);
}
auto LogicalDevice::wait_for_fences(const std::vector<Fence>& fences, bool wait_all, u64 timeout) const -> void {
    assert(fences.size() < 5);
    std::array<VkFence, 5> vk_fences;
    for (u32 i = 0; i < fences.size(); ++i) { vk_fences[i] = fences[i].m_handle; }
    VkResult result;
    result = vkWaitForFences(m_handle, static_cast<u32>(fences.size()), vk_fences.data(), wait_all, timeout);
    if (result != VK_SUCCESS) assert(false);
}

auto LogicalDevice::query_queues(u32 queue_family_index, u32 queue_index) -> Queue {
    Queue queue(*this, queue_family_index, queue_index);
    return queue;
}

auto LogicalDevice::create_buffer(Buffer::TYPE buffer_type, void* data, size_t size) const -> Buffer {
    Buffer buffer(*this, buffer_type, data, size);
    return buffer;
}
auto LogicalDevice::create_descriptor_pool(u32 max_sets, std::vector<VkDescriptorPoolSize>& pool_sizes)
    -> DescriptorPool {
    DescriptorPool pool(*this, max_sets, pool_sizes);
    return pool;
}

auto LogicalDevice::create_descriptor_set_layout(std::vector<vulkan::DescriptorSetLayout::Binding>& bindings) const
    -> DescriptorSetLayout {
    DescriptorSetLayout layout(*this, bindings);
    return layout;
}

auto LogicalDevice::create_shader(const Vulkan_Renderer& renderer, const Vulkan_Shader::Desc& desc) -> Vulkan_Shader {
    return Vulkan_Shader(*this, renderer, desc);
}

auto LogicalDevice::init(const VulkanInstance& instance, const PhysicalDevice& physical_device, const Surface& surface)
    -> void {
    m_physical_device = &physical_device;
    m_instance = &instance;

    VkResult result;

    const QueueFamilyIndices& indices = physical_device.m_queue_family_indices;
    const std::set<u32> unique_queue_families = {indices.m_graphics_family.value(), indices.m_present_family.value()};

    constexpr f32                        queue_priority = 1.0_f32;
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (u32 queue_familiy : unique_queue_families) {
        const VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_familiy,
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
        .enabledExtensionCount = static_cast<u32>(physical_device.m_device_extensions.size()),
        .ppEnabledExtensionNames = physical_device.m_device_extensions.data(),
        .pEnabledFeatures = &physical_device.m_features,
    };

    result = vkCreateDevice(physical_device.m_handle, &create_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);

    Logger::debug("maxBoundDescriptorSets: {}", m_physical_device->m_properties.limits.maxBoundDescriptorSets);
    JF_ASSERT(
        m_physical_device->m_properties.limits.maxBoundDescriptorSets >= 4,
        "maxBoundDescriptorSets too low, it must be at least 4");

    m_graphics_queue = this->query_queues(indices.m_graphics_family.value(), 0);
    m_present_queue = this->query_queues(indices.m_present_family.value(), 0);

    m_command_pool = this->create_command_pool(m_physical_device->m_queue_family_indices.m_graphics_family.value());
    u32                               descriptor_count = 1000;
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

auto LogicalDevice::create_semaphore() const -> Semaphore {
    Semaphore semaphore(*this);
    return semaphore;
}
auto LogicalDevice::create_fence(bool signaled) const -> Fence {
    Fence fence(*this, signaled);
    return fence;
}

auto LogicalDevice::create_command_pool(const QueueFamilyIndex& queue_family_index) -> CommandPool {
    CommandPool cp(*this, queue_family_index);
    return cp;
}

auto LogicalDevice::create_swapchain(const Surface& surface) -> Swapchain {
    Swapchain sc;
    sc.init(*this, surface);
    return sc;
}

auto LogicalDevice::create_image_view(Image& image, VkFormat format) -> ImageView {
    ImageView iv(*this, image, format);
    return iv;
}

auto LogicalDevice::create_render_pass(VkFormat image_format) -> RenderPass {
    RenderPass rp(*this, image_format);
    return rp;
}
auto LogicalDevice::create_framebuffer(const ImageView& image_view, const RenderPass& render_pass, VkExtent2D extent)
    -> Framebuffer {
    Framebuffer fb(*this, image_view, render_pass, extent);
    return fb;
}

auto LogicalDevice::deinit() -> void {
    VkResult result;


    result = vkDeviceWaitIdle(m_handle);
    if (result != VK_SUCCESS) assert(false);
    vkDestroyDevice(m_handle, nullptr);
}

} // namespace vulkan
} // namespace JadeFrame