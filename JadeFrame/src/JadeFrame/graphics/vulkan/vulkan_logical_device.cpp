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


struct Meshhh {
	std::vector<VVertex> m_vertices = {
		{{-0.5f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
		{{+0.5f, -0.5f}, {+0.0f, +1.0f, +0.0f}},
		{{+0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}},

		{{-0.5f, +0.5f}, {+1.0f, +1.0f, +1.0f}},
	};
	std::vector<u16> m_indices = {
		0, 1, 2,
		2, 3, 0,
	};
};

static Meshhh g_mesh;


auto VulkanLogicalDevice::create_command_buffers(
	const VulkanCommandPool& command_pool,
	const VulkanSwapchain& swapchain) -> void {

	VkResult result;
	m_command_buffers.resize(swapchain.m_framebuffers.size());

	const VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = command_pool.m_handle,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<u32>(m_command_buffers.size()),
	};
	result = vkAllocateCommandBuffers(m_handle, &alloc_info, m_command_buffers.data());
	if (result != VK_SUCCESS) __debugbreak();
}

auto VulkanLogicalDevice::draw_into_command_buffers(
	const VulkanRenderPass& render_pass,
	const VulkanSwapchain& swapchain,
	const VulkanPipeline& pipeline,
	const VulkanDescriptorSets& descriptor_sets,
	const VulkanBuffer& vertex_buffer,
	const VulkanBuffer& index_buffer,
	const std::vector<u16>& indices,
	const VkClearValue color_value
) -> void {

	VkResult result;

	for (size_t i = 0; i < m_command_buffers.size(); i++) {
		const VkCommandBufferBeginInfo begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};
		result = vkBeginCommandBuffer(m_command_buffers[i], &begin_info);
		if (result != VK_SUCCESS) __debugbreak();
		{
			//const VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			const VkClearValue clear_color = color_value;
			const VkRenderPassBeginInfo render_pass_info = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass = render_pass.m_handle,
				.framebuffer = swapchain.m_framebuffers[i],
				.renderArea = {
					.offset = {0, 0},
					.extent = swapchain.m_extent
				},
				.clearValueCount = 1,
				.pClearValues = &clear_color,
			};

			vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_graphics_pipeline);
				VkBuffer vertex_buffers[] = { vertex_buffer.m_buffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertex_buffers, offsets);
				//vkCmdDraw(m_command_buffers[i], static_cast<u32>(g_vertices.size()), 1, 0, 0);

				vkCmdBindIndexBuffer(m_command_buffers[i], index_buffer.m_buffer, 0, VK_INDEX_TYPE_UINT16);
				vkCmdBindDescriptorSets(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_pipeline_layout, 0, 1, &descriptor_sets.m_descriptor_sets[i], 0, nullptr);
				vkCmdDrawIndexed(m_command_buffers[i], indices.size(), 1, 0, 0, 0);
			}
			vkCmdEndRenderPass(m_command_buffers[i]);
		}
		result = vkEndCommandBuffer(m_command_buffers[i]);
		if (result != VK_SUCCESS) __debugbreak();
	}
}



auto VulkanLogicalDevice::create_sync_objects() -> void {
	m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	m_images_in_flight.resize(m_swapchain.m_images.size());

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_image_available_semaphores[i].init(*this);
		m_render_finished_semaphores[i].init(*this);
		m_in_flight_fences[i].init(*this);
	}
}

auto VulkanLogicalDevice::recreate_swapchain() -> void {

#if 0
	VkResult result;
	RECT area;
	GetClientRect(m_instance_p->m_window_handle, &area);
	i32 width = area.right;
	i32 height = area.bottom;


	for (size_t i = 0; i < m_swapchain.m_images.size(); i++) {
		m_uniform_buffers[i].deinit();
	}
	m_command_pool.deinit();
	m_pipeline.deinit();
	m_render_pass.deinit();
	m_swapchain.deinit();

	m_swapchain.init(*this, *m_physical_device_p, m_instance_p->m_surface);
	m_render_pass.init(*this, m_swapchain.m_image_format);
	m_pipeline.init(*this, m_swapchain, m_descriptor_set_layout, m_render_pass, GLSLCodeLoader::get_by_name("spirv_test_0"));
	m_swapchain.create_framebuffers(m_render_pass.m_handle);


	m_command_pool.init(*this, *m_physical_device_p);

	for (u32 i = 0; i < m_swapchain.m_images.size(); i++) {
		m_uniform_buffers[i].init(*this, VULKAN_BUFFER_TYPE::UNIFORM, nullptr, sizeof(UniformBufferObject));
	}
	m_descriptor_pool.init(*this);
	m_descriptor_sets.init(*this, m_swapchain.m_images.size(), m_descriptor_set_layout, m_descriptor_pool);

	this->create_command_buffers(m_command_pool, m_swapchain);
	this->draw_into_command_buffers(
		m_render_pass, m_swapchain, m_pipeline,
		m_descriptor_sets,
		m_vertex_buffer,
		m_index_buffer,
		g_indices
	);
	m_images_in_flight.resize(m_swapchain.m_images.size(), VK_NULL_HANDLE);
#else
	m_swapchain.deinit();


	m_swapchain.init(*this, *m_physical_device_p, m_instance_p->m_surface);
	m_render_pass.init(*this, m_swapchain.m_image_format);
	m_pipeline.init(*this, m_swapchain, m_descriptor_set_layout, m_render_pass, GLSLCodeLoader::get_by_name("spirv_test_0"));
	m_swapchain.create_framebuffers(m_render_pass.m_handle);
	m_images_in_flight.resize(m_swapchain.m_images.size());
#endif
}
auto VulkanLogicalDevice::cleanup_swapchain() -> void {
	for (auto framebuffer : m_swapchain.m_framebuffers) {
		vkDestroyFramebuffer(m_handle, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(m_handle, m_command_pool.m_handle, static_cast<uint32_t>(m_command_buffers.size()), m_command_buffers.data());

	vkDestroyPipeline(m_handle, m_pipeline.m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_handle, m_pipeline.m_pipeline_layout, nullptr);
	vkDestroyRenderPass(m_handle, m_render_pass.m_handle, nullptr);

	for (auto image_view : m_swapchain.m_image_views) {
		vkDestroyImageView(m_handle, image_view, nullptr);
	}

	for (size_t i = 0; i < m_swapchain.m_images.size(); i++) {
		vkDestroyBuffer(m_handle, m_uniform_buffers[i].m_buffer, nullptr);
		vkFreeMemory(m_handle, m_uniform_buffers[i].m_memory, nullptr);
	}

	vkDestroySwapchainKHR(m_handle, m_swapchain.m_handle, nullptr);
}

auto VulkanLogicalDevice::update_uniform_buffer(u32 current_image, const Matrix4x4& view_projection) -> void {
	static auto start_time = std::chrono::high_resolution_clock::now();

	auto current_time = std::chrono::high_resolution_clock::now();
	f32 time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time).count();

	UniformBufferObject ubo = {};
	ubo.model = Matrix4x4::rotation_matrix(
		time * to_radians(90.0f),
		Vec3(0.0f, 0.0f, 1.0f)
	);
	ubo.view = Matrix4x4(1.0f);
	ubo.proj = Matrix4x4(1.0f);
	ubo.proj[1][1] *= -1;

	void* mapped_data = m_uniform_buffers[current_image].map_to_GPU(&ubo, sizeof(ubo));

}

auto VulkanLogicalDevice::draw_frame(const Matrix4x4& view_projection) -> void {
	VkResult result;

	//prepare buffers
	u32 image_index;
	result = vkWaitForFences(m_handle, 1, &m_in_flight_fences[m_current_frame].m_handle, VK_TRUE, UINT64_MAX);
	if (result != VK_SUCCESS) __debugbreak();
	{
		result = vkAcquireNextImageKHR(m_handle, m_swapchain.m_handle, UINT64_MAX, m_image_available_semaphores[m_current_frame].m_handle, VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
			this->recreate_swapchain();
			return;
		} else if (result == VK_SUBOPTIMAL_KHR) {
			std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
			//this->recreate_swapchain();
		} else if (result == VK_SUCCESS) {
		} else {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}
	this->update_uniform_buffer(image_index, view_projection);
	//~prepare buffers

	if (m_images_in_flight[image_index].m_handle != VK_NULL_HANDLE) {
		result = vkWaitForFences(m_handle, 1, &m_images_in_flight[image_index].m_handle, VK_TRUE, UINT64_MAX);
		if (result != VK_SUCCESS) __debugbreak();
	}
	VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame].m_handle };
	VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame].m_handle };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSwapchainKHR swapchains[] = { m_swapchain.m_handle };
	// swap buffers

	{
		const VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = wait_semaphores,
			.pWaitDstStageMask = wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &m_command_buffers[image_index],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = signal_semaphores,
		};

		result = vkResetFences(m_handle, 1, &m_in_flight_fences[m_current_frame].m_handle);
		if (result != VK_SUCCESS) __debugbreak();

		result = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame].m_handle);
		if (result != VK_SUCCESS) __debugbreak();
	}

	{
		const VkPresentInfoKHR present_info = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signal_semaphores,
			.swapchainCount = 1,
			.pSwapchains = swapchains,
			.pImageIndices = &image_index,
			.pResults = nullptr,
		};


		result = vkQueuePresentKHR(m_present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized) {
			m_framebuffer_resized = false;
			std::cout << "recreate because of vkQueuePresentKHR" << std::endl;
			//__debugbreak();
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

	const QueueFamilyIndices& indices = instance.m_physical_device.m_queue_family_indices;

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
		.ppEnabledExtensionNames = instance.m_physical_device.m_device_extensions.data(),
		.pEnabledFeatures = &devices_features,
	};

	result = vkCreateDevice(instance.m_physical_device.m_handle, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	vkGetDeviceQueue(m_handle, indices.m_graphics_family.value(), 0, &m_graphics_queue);
	vkGetDeviceQueue(m_handle, indices.m_present_family.value(), 0, &m_present_queue);

	m_physical_device_p = &instance.m_physical_device;
	m_instance_p = &instance;



	m_swapchain.init(*this, *m_physical_device_p, m_instance_p->m_surface);
	m_render_pass.init(*this, m_swapchain.m_image_format);
	m_swapchain.create_framebuffers(m_render_pass.m_handle);

	m_uniform_buffers.resize(m_swapchain.m_images.size(), VULKAN_BUFFER_TYPE::UNIFORM);
	for (u32 i = 0; i < m_swapchain.m_images.size(); i++) {
		m_uniform_buffers[i].init(*this, VULKAN_BUFFER_TYPE::UNIFORM, nullptr, sizeof(UniformBufferObject));
	}

	m_descriptor_set_layout.init(*this);
	m_descriptor_pool.init(*this, m_swapchain);
	m_descriptor_sets.init(*this, m_swapchain.m_images.size(), m_descriptor_set_layout, m_descriptor_pool, m_uniform_buffers);


	m_command_pool.init(*this, *m_physical_device_p);
	this->create_command_buffers(m_command_pool, m_swapchain);


	m_pipeline.init(*this, m_swapchain, m_descriptor_set_layout, m_render_pass, GLSLCodeLoader::get_by_name("spirv_test_0"));

	m_vertex_buffer.init(*this, VULKAN_BUFFER_TYPE::VERTEX, (void*)g_vertices.data(), sizeof(g_vertices[0]) * g_vertices.size());
	m_index_buffer.init(*this, VULKAN_BUFFER_TYPE::INDEX, (void*)g_indices.data(), sizeof(g_indices[0]) * g_indices.size());

	this->draw_into_command_buffers(
		m_render_pass,
		m_swapchain,
		m_pipeline,
		m_descriptor_sets,
		m_vertex_buffer,
		m_index_buffer,
		g_indices,
		VkClearValue{0.0f, 0.0f, 0.0f, 1.0f}
	);

	this->create_sync_objects();

	//this->create_texture_image("C:\\DEV\\Projects\\JadeFrame\\JadeFrame\\resource\\wall.jpg");
	//this->create_texture_image_view();
	//this->create_texture_sampler();

}

auto VulkanLogicalDevice::deinit() -> void {
	this->cleanup_swapchain();
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_render_finished_semaphores[i].deinit();
		m_image_available_semaphores[i].deinit();
		m_in_flight_fences[i].deinit();
	}

	m_command_pool.deinit();
	m_pipeline.deinit();
	m_render_pass.deinit();
	m_swapchain.deinit();

	vkDestroyDevice(m_handle, nullptr);
}

}