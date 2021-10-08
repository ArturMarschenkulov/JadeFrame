#include "pch.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_instance.h"
#include "vulkan_buffer.h"

#include <Windows.h> // TODO: Try to remove it. This is used in "recreate_swapchain()"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/math.h"

#include <thread>
#include <future>
#include <set>
#include <cassert>


namespace JadeFrame {

static const i32 MAX_FRAMES_IN_FLIGHT = 2;

static auto VkResult_to_string(VkResult x) {
	std::string str;
	switch (x) {
		case VK_SUCCESS: str = "VK_SUCCESS"; break;
		case VK_NOT_READY: str = "VK_NOT_READY"; break;
		case VK_TIMEOUT: str = "VK_TIMEOUT"; break;
		case VK_EVENT_SET: str = "VK_EVENT_SET"; break;
		case VK_EVENT_RESET: str = "VK_EVENT_RESET"; break;
		case VK_INCOMPLETE: str = "VK_INCOMPLETE"; break;
		case VK_SUBOPTIMAL_KHR: str = "VK_SUBOPTIMAL_KHR"; break;
		case VK_ERROR_OUT_OF_DATE_KHR: str = "VK_ERROR_OUT_OF_DATE_KHR"; break;
		default: __debugbreak(); str = ""; break;
	}
	return str;
}

auto VulkanLogicalDevice::recreate_swapchain() -> void {
	m_render_pass.deinit();
	m_swapchain.deinit();

	m_swapchain.init(*this, *m_physical_device_p, m_instance_p->m_surface);
	m_render_pass.init(*this, m_swapchain.m_image_format);
	m_swapchain.create_framebuffers(m_render_pass);
	m_images_in_flight.resize(m_swapchain.m_images.size());

}
auto VulkanLogicalDevice::cleanup_swapchain() -> void {


	//m_command_buffers.deinit();
	vkDestroyRenderPass(m_handle, m_render_pass.m_handle, nullptr);



	for (size_t i = 0; i < m_swapchain.m_images.size(); i++) {
		vkDestroyBuffer(m_handle, m_uniform_buffers[i].m_handle, nullptr);
		vkFreeMemory(m_handle, m_uniform_buffers[i].m_memory, nullptr);
	}

	m_swapchain.deinit();
	vkDestroySwapchainKHR(m_handle, m_swapchain.m_handle, nullptr);
}

auto VulkanLogicalDevice::init(const VulkanInstance& instance, const VulkanPhysicalDevice& physical_device) -> void {
	m_physical_device_p = &physical_device;
	m_instance_p = &instance;

	VkResult  result;

	const QueueFamilyIndices& indices = physical_device.m_queue_family_indices;

	std::set<u32> unique_queue_families = {
		indices.m_graphics_family.value(),
		indices.m_present_family.value()
	};

	constexpr f32 queue_priority = 1.0_f32;
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	for (u32 queue_familiy : unique_queue_families) {
		const VkDeviceQueueCreateInfo queue_create_info = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = {},
			.flags = {},
			.queueFamilyIndex = queue_familiy,
			.queueCount = 1,
			.pQueuePriorities = &queue_priority,
		};
		queue_create_infos.push_back(queue_create_info);
	}

	const VkPhysicalDeviceFeatures devices_features = {};
	const VkDeviceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size()),
		.pQueueCreateInfos = queue_create_infos.data(),
		.enabledLayerCount = instance.m_enable_validation_layers ? static_cast<uint32_t>(instance.m_validation_layers.size()) : 0,
		.ppEnabledLayerNames = instance.m_enable_validation_layers ? instance.m_validation_layers.data() : nullptr,
		.enabledExtensionCount = static_cast<u32>(instance.m_physical_device.m_device_extensions.size()),
		.ppEnabledExtensionNames = physical_device.m_device_extensions.data(),
		.pEnabledFeatures = &devices_features,
	};

	result = vkCreateDevice(physical_device.m_handle, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	m_graphics_queue = this->query_queue(indices.m_graphics_family.value(), 0);
	m_present_queue = this->query_queue(indices.m_present_family.value(), 0);


	// Swapchain stuff
	m_swapchain.init(*this, *m_physical_device_p, m_instance_p->m_surface);
	m_render_pass.init(*this, m_swapchain.m_image_format);
	m_swapchain.create_framebuffers(m_render_pass);
	const u32 swapchain_image_amount = m_swapchain.m_images.size();

	// Uniform stuff
	m_uniform_buffers.resize(swapchain_image_amount, VulkanBuffer::TYPE::UNIFORM);
	for (u32 i = 0; i < swapchain_image_amount; i++) {
		m_uniform_buffers[i].init(*this, VulkanBuffer::TYPE::UNIFORM, nullptr, sizeof(UniformBufferObject));
	}

	// Descriptor stuff
	m_descriptor_set_layout.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
	m_descriptor_set_layout.init(*this);

	m_descriptor_pool.add_pool_size({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain_image_amount });
	m_descriptor_pool.init(*this, swapchain_image_amount);

	m_descriptor_sets = m_descriptor_pool.allocate_descriptor_sets(m_descriptor_set_layout, swapchain_image_amount);
	for (u32 i = 0; i < m_descriptor_sets.size(); i++) {
		m_descriptor_sets[i].add_uniform_buffer(m_uniform_buffers[i], 0, 0);
		m_descriptor_sets[i].update();
	}

	// Commad Buffer stuff
	m_command_pool.init(*this, m_physical_device_p->m_queue_family_indices.m_graphics_family.value());
	m_command_buffers = m_command_pool.allocate_command_buffers(m_swapchain.m_framebuffers.size());

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
	for (size_t i = 0; i < swapchain_image_amount; i++) {
		m_images_in_flight[i].init(*this);
	}

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
	m_render_pass.deinit();
	m_swapchain.deinit();
	result = vkDeviceWaitIdle(m_handle);
	vkDestroyDevice(m_handle, nullptr);
}

auto VulkanLogicalDevice::query_queue(u32 queue_family_index, u32 queue_index) -> VulkanQueue {
	VulkanQueue queue;
	vkGetDeviceQueue(m_handle, queue_family_index, queue_index, &queue.m_handle);
	return queue;
}

}