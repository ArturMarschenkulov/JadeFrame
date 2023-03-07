#include "pch.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_context.h"
#include "vulkan_buffer.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/math.h"
#include "JadeFrame/utils/assert.h"

#include <thread>
#include <future>
#include <set>
#include <cassert>
#include <array>


namespace JadeFrame {

static const i32 MAX_FRAMES_IN_FLIGHT = 2;

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
            assert(false);
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
    result = vkQueueSubmit(m_handle, 1, &submit_info, p_fence->m_handle);
    if (result != VK_SUCCESS) assert(false);
}
auto Queue::submit(
    const CommandBuffer& cmd_buffer, const Semaphore* wait_semaphore, const Semaphore* signal_semaphore,
    const Fence* fence) const -> void {
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSubmitInfo   submit_info = {
          .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
          .pNext = nullptr,
          .waitSemaphoreCount = 1,
          .pWaitSemaphores = &wait_semaphore->m_handle,
          .pWaitDstStageMask = wait_stages,
          .commandBufferCount = 1,
          .pCommandBuffers = &cmd_buffer.m_handle,
          .signalSemaphoreCount = 1,
          .pSignalSemaphores = &signal_semaphore->m_handle,
    };
    /*this->submit(submit_info);*/
    VkResult result;
    result = vkQueueSubmit(m_handle, 1, &submit_info, fence->m_handle);
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
    /*VkResult result;*/

    const VkPresentInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &semaphore->m_handle,
        .swapchainCount = 1,
        .pSwapchains = &swapchain.m_handle,
        .pImageIndices = &index,
        .pResults = nullptr,
    };

    return vkQueuePresentKHR(m_handle, &info);
    // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR /*|| d.m_framebuffer_resized*/) {
    //	/*d.m_framebuffer_resized = false;*/
    //	std::cout << "recreate because of vkQueuePresentKHR" << std::endl;
    //	//__debugbreak();
    //	//d.recreate_swapchain();
    //	this->recreate();
    // } else if (result != VK_SUCCESS) {
    //	std::cout << "failed to present swap chain image!" << std::endl;
    //	__debugbreak();
    // }
}

/*---------------------------
        Logical Device
---------------------------*/

auto LogicalDevice::recreate_swapchain() -> void {
    vkDeviceWaitIdle(m_handle);
    m_swapchain.deinit();

    m_swapchain.init(*this, m_instance->m_surface);
}
auto LogicalDevice::cleanup_swapchain() -> void {

    // m_render_pass.deinit();
    m_swapchain.deinit();
}

auto LogicalDevice::wait_for_fence(const Fence& fences, bool wait_all, u64 timeout) -> void {
    VkResult result;
    result = vkWaitForFences(m_handle, 1, &fences.m_handle, wait_all, timeout);
    if (result != VK_SUCCESS) assert(false);
}
auto LogicalDevice::wait_for_fences(const std::vector<Fence>& fences, bool wait_all, u64 timeout) -> void {
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

auto LogicalDevice::create_buffer(Buffer::TYPE buffer_type, void* data, size_t size) -> Buffer {
    Buffer buffer = {buffer_type};
    buffer.init(*this, buffer_type, data, size);
    return buffer;
}
auto LogicalDevice::create_descriptor_pool(u32 max_sets, std::vector<VkDescriptorPoolSize>& pool_sizes)
    -> DescriptorPool {
    DescriptorPool pool;
    pool.init(*this, max_sets, pool_sizes);
    return pool;
}

auto LogicalDevice::create_descriptor_set_layout(std::vector<vulkan::DescriptorSetLayout::Binding>& bindings) const
    -> DescriptorSetLayout {
    DescriptorSetLayout layout;
    layout.init(*this, bindings);
    return layout;
}

auto LogicalDevice::create_shader(const Vulkan_Shader::DESC& desc) -> Vulkan_Shader {
    Vulkan_Shader shader(*this, desc);
    return shader;
}

auto LogicalDevice::init(const VulkanInstance& instance, const PhysicalDevice& physical_device) -> void {
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

    const VkPhysicalDeviceFeatures devices_features = {};
    const VkDeviceCreateInfo       create_info = {
              .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
              .pNext = nullptr,
              .flags = 0,
              .queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
              .pQueueCreateInfos = queue_create_infos.data(),
              .enabledLayerCount = 0,         // this is deprecated and ignored
              .ppEnabledLayerNames = nullptr, // this is deprecated and ignored
              .enabledExtensionCount = static_cast<u32>(physical_device.m_device_extensions.size()),
              .ppEnabledExtensionNames = physical_device.m_device_extensions.data(),
              .pEnabledFeatures = &devices_features,
    };

    result = vkCreateDevice(physical_device.m_handle, &create_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);



    m_graphics_queue = this->query_queues(indices.m_graphics_family.value(), 0);
    m_present_queue = this->query_queues(indices.m_present_family.value(), 0);


    // Swapchain stuff
    m_swapchain = this->create_swapchain();
    const u32 swapchain_image_amount = static_cast<u32>(m_swapchain.m_images.size());

    // Commad Buffer stuff
    m_command_pool = this->create_command_pool(m_physical_device->m_queue_family_indices.m_graphics_family.value());
    m_command_buffers = m_command_pool.allocate_buffers(static_cast<u32>(m_swapchain.m_framebuffers.size()));

    Logger::debug("maxBoundDescriptorSets: {}", m_physical_device->m_properties.limits.maxBoundDescriptorSets);
    JF_ASSERT(m_physical_device->m_properties.limits.maxBoundDescriptorSets >= 4, "");

    // Sync objects stuff
    m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_image_available_semaphores[i] = this->create_semaphore();
        m_render_finished_semaphores[i] = this->create_semaphore();
        m_in_flight_fences[i] = this->create_fence(true);
    }
}

auto LogicalDevice::create_semaphore() -> Semaphore {
    Semaphore semaphore;
    semaphore.init(*this);
    return semaphore;
}
auto LogicalDevice::create_fence(bool signaled) -> Fence {
    Fence fence;
    fence.init(*this, signaled);
    return fence;
}

auto LogicalDevice::create_command_pool(const QueueFamilyIndex& queue_family_index) -> CommandPool {
    CommandPool cp;
    cp.init(*this, queue_family_index);
    return cp;
}

auto LogicalDevice::create_swapchain() -> Swapchain {
    Swapchain sc;
    sc.init(*this, m_instance->m_surface);
    return sc;
}

auto LogicalDevice::create_image_view(Image& image, VkFormat format) -> ImageView {
    ImageView iv;
    iv.init(*this, image, format);
    return iv;
}

auto LogicalDevice::deinit() -> void {
    VkResult result;
    this->cleanup_swapchain();
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_render_finished_semaphores[i].deinit();
        m_image_available_semaphores[i].deinit();
        m_in_flight_fences[i].deinit();
    }

    m_command_pool.deinit();
    // m_render_pass.deinit();
    m_swapchain.deinit();
    result = vkDeviceWaitIdle(m_handle);
    if (result != VK_SUCCESS) assert(false);
    vkDestroyDevice(m_handle, nullptr);
}

} // namespace vulkan
} // namespace JadeFrame