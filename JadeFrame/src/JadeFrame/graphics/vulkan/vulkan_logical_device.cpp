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
auto VulkanQueue::submit(const VkSubmitInfo& submit_info, const VulkanFence* p_fence) const -> void {
    VkResult result;
    result = vkQueueSubmit(m_handle, 1, &submit_info, p_fence->m_handle);
    if (result != VK_SUCCESS) assert(false);
}
auto VulkanQueue::submit(
    const VulkanCommandBuffer& cmd_buffer, const VulkanSemaphore* wait_semaphore,
    const VulkanSemaphore* signal_semaphore, const VulkanFence* fence) -> void {
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
    if (result != VK_SUCCESS) assert(false);
}
auto VulkanQueue::wait_idle() const -> void {
    VkResult result;
    result = vkQueueWaitIdle(m_handle);
    if (result != VK_SUCCESS) assert(false);
}
auto VulkanQueue::present(VkPresentInfoKHR info, VkResult& result) const -> void {
    result = vkQueuePresentKHR(m_handle, &info);
    if (result != VK_SUCCESS) assert(false);
}

auto VulkanQueue::present(
    const u32& index, const VulkanSwapchain& swapchain, const VulkanSemaphore* semaphore, VkResult* out_result) const
    -> void {
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

    *out_result = vkQueuePresentKHR(m_handle, &info);
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

auto VulkanLogicalDevice::recreate_swapchain() -> void {
    vkDeviceWaitIdle(m_handle);
    m_swapchain.deinit();

    m_swapchain.init(*this, m_instance->m_surface);
    m_images_in_flight.resize(m_swapchain.m_images.size());
}
auto VulkanLogicalDevice::cleanup_swapchain() -> void {

    // m_render_pass.deinit();
    m_swapchain.deinit();
}

auto VulkanLogicalDevice::init(const VulkanInstance& instance, const VulkanPhysicalDevice& physical_device) -> void {
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
              .enabledLayerCount =
            instance.m_enable_validation_layers ? static_cast<uint32_t>(instance.m_desired_layer_names.size()) : 0,
              .ppEnabledLayerNames = instance.m_enable_validation_layers ? instance.m_desired_layer_names.data() : nullptr,
              .enabledExtensionCount = static_cast<u32>(instance.m_physical_device.m_device_extensions.size()),
              .ppEnabledExtensionNames = physical_device.m_device_extensions.data(),
              .pEnabledFeatures = &devices_features,
    };

    result = vkCreateDevice(physical_device.m_handle, &create_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);




    m_graphics_queue = this->query_queue(indices.m_graphics_family.value(), 0);
    m_present_queue = this->query_queue(indices.m_present_family.value(), 0);


    // Swapchain stuff
    m_swapchain.init(*this, m_instance->m_surface);
    const u32 swapchain_image_amount = static_cast<u32>(m_swapchain.m_images.size());

    // Uniform stuff

    m_ub_cam.init(*this, VulkanBuffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));
    m_ub_tran.init(*this, VulkanBuffer::TYPE::UNIFORM, nullptr, sizeof(Matrix4x4));


    // Create main descriptor pool, which should have all kinds of types. In the future maybe make it more specific.
    u32 descriptor_count = 1000;
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_SAMPLER, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptor_count});
    m_main_descriptor_pool.add_pool_size({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptor_count});
    m_main_descriptor_pool.init(*this, 1);

    m_descriptor_set_layout_0.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    m_descriptor_set_layout_0.add_binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT);
    // m_descriptor_set_layout_0.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL);



    Logger::debug("maxBoundDescriptorSets: {}", m_physical_device->m_properties.limits.maxBoundDescriptorSets);
    JF_ASSERT(m_physical_device->m_properties.limits.maxBoundDescriptorSets >= 4, "");




    m_descriptor_set_layout_0.init(*this);

    m_descriptor_sets = m_main_descriptor_pool.allocate_descriptor_sets(m_descriptor_set_layout_0, 1);
    m_descriptor_sets[0].add_uniform_buffer(0, m_ub_cam, 0, sizeof(Matrix4x4));
    m_descriptor_sets[0].add_uniform_buffer(1, m_ub_tran, 0, sizeof(Matrix4x4));
    m_descriptor_sets[0].update();


    // Commad Buffer stuff
    m_command_pool.init(*this, m_physical_device->m_queue_family_indices.m_graphics_family.value());
    m_command_buffers = m_command_pool.allocate_command_buffers(static_cast<u32>(m_swapchain.m_framebuffers.size()));

    // Sync objects stuff
    m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    m_images_in_flight.resize(swapchain_image_amount);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        m_image_available_semaphores[i].init(*this);
        m_render_finished_semaphores[i].init(*this);
        m_in_flight_fences[i].init(*this);
    }
    for (size_t i = 0; i < swapchain_image_amount; i++) { m_images_in_flight[i].init(*this); }
}

auto VulkanLogicalDevice::deinit() -> void {
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

auto VulkanLogicalDevice::query_queue(u32 queue_family_index, u32 queue_index) -> VulkanQueue {
    VulkanQueue queue;
    vkGetDeviceQueue(m_handle, queue_family_index, queue_index, &queue.m_handle);
    return queue;
}

} // namespace JadeFrame