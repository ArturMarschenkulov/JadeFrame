#include "vulkan_logical_device.h"
#include "vulkan_shared.h" // TODO: This should be temprary
#include "vulkan_context.h"

#include <Windows.h> // TODO: Try to remove it

#include <iostream>

#include <thread>
#include <future>
#include <set>
#include <JadeFrame/math/mat_4.h>

#include <shaderc/shaderc.hpp>
#include <JadeFrame/math/math.h>

static const i32 MAX_FRAMES_IN_FLIGHT = 2;



auto VkResult_to_string(VkResult x) {
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

//const std::vector<Vertex> g_vertices = {
//	{{+0.0f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
//	{{+0.5f, +0.5f}, {+0.0f, +1.0f, +0.0f}},
//	{{-0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}}
//};

const std::vector<VVertex> g_vertices = {
	{{-0.5f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
	{{+0.5f, -0.5f}, {+0.0f, +1.0f, +0.0f}},
	{{+0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}},

	{{-0.5f, +0.5f}, {+1.0f, +1.0f, +1.0f}},
};
const std::vector<u16> g_indices = {
	0, 1, 2,
	2, 3, 0,
};

static auto find_memory_type(const VulkanPhysicalDevice& physical_device, u32 type_filter, VkMemoryPropertyFlags properties) -> u32 {
	const VkPhysicalDeviceMemoryProperties& mem_props = physical_device.m_memory_properties;
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}

auto VulkanLogicalDevice::create_command_buffers() -> void {
	VkResult result;
	m_command_buffers.resize(m_swapchain.m_framebuffers.size());

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.commandPool = m_command_pool.m_command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)m_command_buffers.size();

	result = vkAllocateCommandBuffers(m_handle, &alloc_info, m_command_buffers.data());
	if (result != VK_SUCCESS) {
		__debugbreak();
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < m_command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.pNext = nullptr;
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		result = vkBeginCommandBuffer(m_command_buffers[i], &begin_info);
		if (result != VK_SUCCESS) {
			__debugbreak();
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = m_render_pass.m_render_pass;
		render_pass_info.framebuffer = m_swapchain.m_framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = m_swapchain.m_swapchain_extent;
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clear_color;

		vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.m_graphics_pipeline);
			VkBuffer vertex_buffers[] = { m_vertex_buffer.m_buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertex_buffers, offsets);
			//vkCmdDraw(m_command_buffers[i], static_cast<u32>(g_vertices.size()), 1, 0, 0);

			vkCmdBindIndexBuffer(m_command_buffers[i], m_index_buffer.m_buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.m_pipeline_layout, 0, 1, &m_descriptor_sets[i], 0, nullptr);
			vkCmdDrawIndexed(m_command_buffers[i], static_cast<u32>(g_indices.size()), 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(m_command_buffers[i]);

		if (vkEndCommandBuffer(m_command_buffers[i]) != VK_SUCCESS) {
			__debugbreak();
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

auto VulkanLogicalDevice::create_sync_objects() -> void {
	m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	m_images_in_flight.resize(m_swapchain.m_swapchain_images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;
	semaphore_info.flags = 0;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkResult res_0 = vkCreateSemaphore(m_handle, &semaphore_info, nullptr, &m_image_available_semaphores[i]);
		VkResult res_1 = vkCreateSemaphore(m_handle, &semaphore_info, nullptr, &m_render_finished_semaphores[i]);
		VkResult res_2 = vkCreateFence(m_handle, &fence_info, nullptr, &m_in_flight_fences[i]);

		if (res_0 != VK_SUCCESS ||
			res_1 != VK_SUCCESS ||
			res_2 != VK_SUCCESS) {
			__debugbreak();
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

auto VulkanLogicalDevice::recreate_swapchain() -> void {
	VkResult result;
	std::cout << " lololo " << std::endl;
	RECT area;
	GetClientRect(m_instance_p->m_window_handle, &area);
	i32 width = area.right;
	i32 height = area.bottom;

	result = vkDeviceWaitIdle(m_handle);

	this->cleanup_swapchain();

	m_swapchain.init(*this, *m_instance_p);
	m_render_pass.init(*this);
	m_pipeline.init(*this);
	m_swapchain.create_framebuffers(m_render_pass.m_render_pass);

	this->create_command_buffers();
	//this->create_uniform_buffer();
	m_uniform_buffers.resize(m_swapchain.m_swapchain_images.size());
	for (u32 i = 0; i < m_swapchain.m_swapchain_images.size(); i++) {
		m_uniform_buffers[i].init(*this);
	}
	this->create_descriptor_pool();
	this->create_descriptor_sets();
	this->create_command_buffers();

	m_images_in_flight.resize(m_swapchain.m_swapchain_images.size(), VK_NULL_HANDLE);
}
auto VulkanLogicalDevice::cleanup_swapchain() -> void {
	for (auto framebuffer : m_swapchain.m_framebuffers) {
		vkDestroyFramebuffer(m_handle, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(m_handle, m_command_pool.m_command_pool, static_cast<uint32_t>(m_command_buffers.size()), m_command_buffers.data());

	vkDestroyPipeline(m_handle, m_pipeline.m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_handle, m_pipeline.m_pipeline_layout, nullptr);
	vkDestroyRenderPass(m_handle, m_render_pass.m_render_pass, nullptr);

	for (auto image_view : m_swapchain.m_swapchain_image_views) {
		vkDestroyImageView(m_handle, image_view, nullptr);
	}

	for (size_t i = 0; i < m_swapchain.m_swapchain_images.size(); i++) {
		vkDestroyBuffer(m_handle, m_uniform_buffers[i].m_buffer, nullptr);
		vkFreeMemory(m_handle, m_uniform_buffers[i].m_memory, nullptr);
	}

	vkDestroySwapchainKHR(m_handle, m_swapchain.m_swapchain, nullptr);
}


auto VulkanLogicalDevice::create_descriptor_set_layout() -> void {
	VkResult result;

	VkDescriptorSetLayoutBinding ubo_layout_binding = {};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &ubo_layout_binding;

	result = vkCreateDescriptorSetLayout(m_handle, &layout_info, nullptr, &m_descriptor_set_layout);
	if (result != VK_SUCCESS) {
		__debugbreak();
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

auto VulkanLogicalDevice::update_uniform_buffer(u32 current_image) -> void {
	static auto start_time = std::chrono::high_resolution_clock::now();

	auto current_time = std::chrono::high_resolution_clock::now();
	f32 time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time).count();

	f32 d_0 = to_radians(90.0f);
	f32 d_1 = to_radians(45.0f);


	//UniformBufferObject ubo = {};
	//ubo.model = Matrix4x4::rotation_matrix(
	//	time * d_0, 
	//	Vec3(0.0f, 0.0f, 1.0f)
	//) * Matrix4x4(1.0f);
	//ubo.view = Matrix4x4::look_at_matrix(
	//	Vec3(2.0f, 2.0f, 2.0f), 
	//	Vec3(0.0f, 0.0f, 0.0f), 
	//	Vec3(0.0f, 0.0f, 1.0f)
	//);
	//ubo.proj = Matrix4x4::perspective_projection_matrix(
	//	d_1, 
	//	m_swapchain_extent.width / (f32)m_swapchain_extent.height, 
	//	0.1f, 
	//	10.0f
	//);
	//ubo.proj[1][1] *= -1;

	UniformBufferObject ubo = {};
	ubo.model = Matrix4x4::rotation_matrix(
		time * d_0,
		Vec3(0.0f, 0.0f, 1.0f)
	);
	ubo.view = Matrix4x4(1.0f);
	//ubo.proj = Matrix4x4::perspective_projection_matrix(
	//	d_1, 
	//	m_swapchain_extent.width / (f32)m_swapchain_extent.height, 
	//	0.1f, 
	//	10.0f
	//);
	ubo.proj = Matrix4x4(1.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(m_handle, m_uniform_buffers[current_image].m_memory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(m_handle, m_uniform_buffers[current_image].m_memory);
}

auto VulkanLogicalDevice::create_descriptor_pool() -> void {
	VkResult result;
	VkDescriptorPoolSize pool_size{};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = static_cast<u32>(m_swapchain.m_swapchain_images.size());

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = static_cast<u32>(m_swapchain.m_swapchain_images.size());

	result = vkCreateDescriptorPool(m_handle, &pool_info, nullptr, &m_descriptor_pool);
	if (result != VK_SUCCESS) {
		__debugbreak();
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

auto VulkanLogicalDevice::create_descriptor_sets() -> void {
	VkResult result;

	std::vector<VkDescriptorSetLayout> layouts(m_swapchain.m_swapchain_images.size(), m_descriptor_set_layout);

	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.descriptorPool = m_descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<u32>(m_swapchain.m_swapchain_images.size());
	alloc_info.pSetLayouts = layouts.data();

	m_descriptor_sets.resize(m_swapchain.m_swapchain_images.size());

	result = vkAllocateDescriptorSets(m_handle, &alloc_info, m_descriptor_sets.data());
	if (result != VK_SUCCESS) {
		__debugbreak();
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (u32 i = 0; i < m_swapchain.m_swapchain_images.size(); i++) {
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = m_uniform_buffers[i].m_buffer;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptor_write = {};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.pNext = nullptr;
		descriptor_write.dstSet = m_descriptor_sets[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorCount = 1;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.pImageInfo = nullptr;
		descriptor_write.pBufferInfo = &buffer_info;
		descriptor_write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(m_handle, 1, &descriptor_write, 0, nullptr);
	}
}

auto VulkanLogicalDevice::draw_frame() -> void {
	VkResult result;

	//prepare buffers
	u32 image_index;
	result = vkWaitForFences(m_handle, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
	{
		result = vkAcquireNextImageKHR(m_handle, m_swapchain.m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
			this->recreate_swapchain();
			return;
		} else if (result == VK_SUBOPTIMAL_KHR) {
			//std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
			//this->recreate_swapchain();
		} else if (result == VK_SUCCESS) {
		} else {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}
	this->update_uniform_buffer(image_index);
	//~prepare buffers

	if (m_images_in_flight[image_index] != VK_NULL_HANDLE) {
		result = vkWaitForFences(m_handle, 1, &m_images_in_flight[image_index], VK_TRUE, UINT64_MAX);
	}
	VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame] };
	VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSwapchainKHR swapchains[] = { m_swapchain.m_swapchain };
	// swap buffers

	{
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &m_command_buffers[image_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		result = vkResetFences(m_handle, 1, &m_in_flight_fences[m_current_frame]);

		result = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}

	{
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr;


		result = vkQueuePresentKHR(m_present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized) {
			m_framebuffer_resized = false;
			std::cout << "recreate because of vkQueuePresentKHR" << std::endl;
			__debugbreak();
			this->recreate_swapchain();
		} else if (result != VK_SUCCESS) {
			std::cout << "failed to present swap chain image!" << std::endl;
			__debugbreak();
		}
	}
	m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

	//~swap buffer
}

auto VulkanLogicalDevice::init(const VulkanInstance& instance) -> void {
	VkResult  result;

	QueueFamilyIndices indices = instance.m_physical_device.m_queue_family_indices;

	std::set<u32> unique_queue_families = {
		indices.m_graphics_family.value(),
		indices.m_present_family.value()
	};

	constexpr f32 queue_priority = 1.0_f32;
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	for (u32 queue_familiy : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_familiy;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures devices_features = {};
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size());;
	create_info.pEnabledFeatures = &devices_features;
	create_info.enabledExtensionCount = static_cast<u32>(g_device_extensions.size());
	create_info.ppEnabledExtensionNames = g_device_extensions.data();
	create_info.enabledLayerCount = g_enable_validation_layers ? static_cast<uint32_t>(g_validation_layers.size()) : 0;
	create_info.ppEnabledLayerNames = g_enable_validation_layers ? g_validation_layers.data() : nullptr;

	result = vkCreateDevice(instance.m_physical_device.m_handle, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_handle, indices.m_graphics_family.value(), 0, &m_graphics_queue);
	vkGetDeviceQueue(m_handle, indices.m_present_family.value(), 0, &m_present_queue);

	m_physical_device_p = &instance.m_physical_device;
	m_instance_p = &instance;


	m_swapchain.init(*this, instance);
	m_render_pass.init(*this);

	this->create_descriptor_set_layout();

	m_pipeline.init(*this);
	m_swapchain.create_framebuffers(m_render_pass.m_render_pass);

	m_command_pool.init(this->m_handle, *m_physical_device_p);


	m_vertex_buffer.init(*this, g_vertices);
	m_index_buffer.init(*this, g_indices);

	m_uniform_buffers.resize(m_swapchain.m_swapchain_images.size(), VULKAN_BUFFER_TYPE::UNIFORM);
	for (u32 i = 0; i < m_swapchain.m_swapchain_images.size(); i++) {
		m_uniform_buffers[i].init(*this);
	}

	this->create_descriptor_pool();
	this->create_descriptor_sets();
	this->create_command_buffers();
	this->create_sync_objects();

}

auto VulkanLogicalDevice::deinit() -> void {
	this->cleanup_swapchain();
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_handle, m_render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(m_handle, m_image_available_semaphores[i], nullptr);
		vkDestroyFence(m_handle, m_in_flight_fences[i], nullptr);
	}
	vkDestroyCommandPool(m_handle, m_command_pool.m_command_pool, nullptr);

	vkDestroyPipeline(m_handle, m_pipeline.m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_handle, m_pipeline.m_pipeline_layout, nullptr);

	vkDestroyRenderPass(m_handle, m_render_pass.m_render_pass, nullptr);
	for (u32 i = 0; i < m_swapchain.m_swapchain_image_views.size(); i++) {
		vkDestroyImageView(m_handle, m_swapchain.m_swapchain_image_views[i], nullptr);
	}
	vkDestroySwapchainKHR(m_handle, m_swapchain.m_swapchain, nullptr);
	vkDestroyDevice(m_handle, nullptr);
}