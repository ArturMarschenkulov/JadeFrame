#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "vulkan_shader.h"
#include "../opengl/opengl_renderer.h"

#include "../graphics_shared.h"

namespace JadeFrame {


Vulkan_Renderer::Vulkan_Renderer(const Windows_Window& window)
	: m_context(window) {

}
auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void {
	m_clear_color = color;
}

auto Vulkan_Renderer::main_loop() -> void {
	m_context.main_loop();
}



auto Vulkan_Renderer::clear_background() -> void {

}

auto Vulkan_Renderer::submit(const Object& obj) -> void {


	if (obj.m_GPU_mesh_data.m_is_initialized == false) {
		VertexFormat vertex_format;
		//In case there is no buffer layout provided use a default one
		if (obj.m_vertex_format.m_attributes.size() == 0) {
			const VertexFormat vf = {
				{ "v_position", SHADER_TYPE::FLOAT_3 },
				{ "v_color", SHADER_TYPE::FLOAT_4 },
				{ "v_texture_coord", SHADER_TYPE::FLOAT_2 },
				{ "v_normal", SHADER_TYPE::FLOAT_3 },
			};
			vertex_format = vf;
		} else {
			vertex_format = obj.m_vertex_format;
		}
		const VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;
		obj.m_GPU_mesh_data.m_handle = new Vulkan_GPUMeshData(ld, *obj.m_vertex_data, vertex_format);
		obj.m_GPU_mesh_data.m_is_initialized = true;
	}
	if (obj.m_material_handle->m_is_initialized == false) {
		obj.m_material_handle->m_shader_handle->m_api = GRAPHICS_API::VULKAN;
		//obj.m_material_handle->m_shader_handle->init();
		const VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;
		ShaderHandle* sh = obj.m_material_handle->m_shader_handle;

		Vulkan_Shader::DESC shader_desc;
		shader_desc.code = sh->m_code;
		shader_desc.buffer_layout = sh->m_vertex_format;
		obj.m_material_handle->m_shader_handle->m_handle = new Vulkan_Shader(ld, shader_desc);

		if (obj.m_material_handle->m_texture_handle != nullptr) {
			obj.m_material_handle->m_texture_handle->m_api = GRAPHICS_API::VULKAN;
			obj.m_material_handle->m_texture_handle->init();
		}
		obj.m_material_handle->m_is_initialized = true;
	}
	const Vulkan_RenderCommand command = {
		.transform = &obj.m_transform,
		.vertex_data = obj.m_vertex_data,
		.material_handle = obj.m_material_handle,
		.m_GPU_mesh_data = &obj.m_GPU_mesh_data,
	};
	m_render_commands.push_back(command);


}
auto Vulkan_Renderer::render(const Matrix4x4& view_projection) -> void {
	m_view_projection = view_projection;
	VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;

	for (u32 i = 0; i < ld.m_command_buffers.size(); i++) {
		ld.m_command_buffers[i].reset();
	}

	for (size_t i = 0; i < m_render_commands.size(); i++) {
		VkResult result;
		ld.m_in_flight_fences[ld.m_current_frame].wait_for_fences();
		u32 image_index = ld.m_swapchain.acquire_next_image(nullptr, nullptr, result);
		if (result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
				ld.recreate_swapchain();
				return;
			} else if (result == VK_SUBOPTIMAL_KHR) {
				std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
				//this->recreate_swapchain();
			} else {
				throw std::runtime_error("failed to acquire swap chain image!");
			}
		}
		const VulkanImage& image = ld.m_swapchain.m_images[image_index];
		ld.m_present_image_index = image_index;



		const Matrix4x4& transform = *m_render_commands[i].transform;
		const std::array<Matrix4x4, 2>& matrices = { view_projection , transform };
		UniformBufferObject u = { view_projection, transform };
		ld.m_uniform_buffers[image_index].map_to_GPU((void*)matrices.data(), sizeof(matrices));
		//UniformBufferObject ubo = ld.update_ubo(view_projection);
		//ld.m_uniform_buffers[image_index].map_to_GPU(&ubo, sizeof(ubo));


		if (ld.m_images_in_flight[image_index].m_handle != VK_NULL_HANDLE) {
			ld.m_images_in_flight[image_index].wait_for_fences();
		}
		ld.m_images_in_flight[image_index].m_handle = ld.m_in_flight_fences[ld.m_current_frame].m_handle;


		Vulkan_Shader* shader = static_cast<Vulkan_Shader*>(m_render_commands[i].material_handle->m_shader_handle->m_handle);
		Vulkan_GPUMeshData* gpu = static_cast<Vulkan_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);
		const VertexData& vertex_data = *m_render_commands[i].vertex_data;
		const auto& c = m_clear_color;
		ld.m_command_buffers[image_index].draw_into(image_index,
			ld.m_render_pass,
			ld.m_swapchain,
			shader->m_pipeline,
			ld.m_descriptor_sets,
			*gpu,
			vertex_data,
			VkClearValue{ c.r, c.b, c.g, c.a }
		);

		std::array<VkSemaphore, 1> wait_semaphores = { ld.m_image_available_semaphores[ld.m_current_frame].m_handle };
		std::array<VkSemaphore, 1> signal_semaphores = { ld.m_render_finished_semaphores[ld.m_current_frame].m_handle };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		ld.m_in_flight_fences[ld.m_current_frame].reset();

		const VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = wait_semaphores.size(),
			.pWaitSemaphores = wait_semaphores.data(),
			.pWaitDstStageMask = wait_stages,
			.commandBufferCount = 1,
			.pCommandBuffers = &ld.m_command_buffers[image_index].m_handle,
			.signalSemaphoreCount = signal_semaphores.size(),
			.pSignalSemaphores = signal_semaphores.data(),
		};
		result = vkQueueSubmit(ld.m_graphics_queue.m_handle, 1, &submit_info, ld.m_in_flight_fences[ld.m_current_frame].m_handle);
		if (result != VK_SUCCESS) __debugbreak();


	}

	m_render_commands.clear();
}

auto Vulkan_Renderer::present() -> void {
	VkResult result;
	VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;
	std::array<VkSwapchainKHR, 1> swapchains = { ld.m_swapchain.m_handle };
	std::array<VkSemaphore, 1> signal_semaphores = { ld.m_render_finished_semaphores[ld.m_current_frame].m_handle };

	const VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = signal_semaphores.size(),
		.pWaitSemaphores = signal_semaphores.data(),
		.swapchainCount = swapchains.size(),
		.pSwapchains = swapchains.data(),
		.pImageIndices = &ld.m_present_image_index,
		.pResults = nullptr,
	};
	result = vkQueuePresentKHR(ld.m_present_queue.m_handle, &present_info);
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || ld.m_framebuffer_resized) {
			ld.m_framebuffer_resized = false;
			std::cout << "recreate because of vkQueuePresentKHR" << std::endl;
			//__debugbreak();
			ld.recreate_swapchain();
		} else if (result != VK_SUCCESS) {
			std::cout << "failed to present swap chain image!" << std::endl;
			__debugbreak();
		}
	}

	ld.m_current_frame = (ld.m_current_frame + 1) % 2/*MAX_FRAMES_IN_FLIGHT*/;

	//ld.present_frame(m_view_projection);
}
auto Vulkan_Renderer::render_2(const Matrix4x4& view_projection) -> void {
	m_view_projection = view_projection;
	VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;

	//for (u32 i = 0; i < ld.m_command_buffers.size(); i++) {
	//	ld.m_command_buffers[i].reset();
	//}

	for (size_t i = 0; i < m_render_commands.size(); i++) {
		VkResult res;
		//ld.m_swapchain.acquire_next_image(nullptr, nullptr, res);
		Vulkan_Shader* shader = static_cast<Vulkan_Shader*>(m_render_commands[i].material_handle->m_shader_handle->m_handle);
		Vulkan_GPUMeshData* gpu = static_cast<Vulkan_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);
		const VertexData& vertex_data = *m_render_commands[i].vertex_data;



		const Matrix4x4& transform = *m_render_commands[i].transform;
		const std::vector<Matrix4x4>& matrices = { view_projection , transform };

		const auto& c = m_clear_color;
		for (u32 i = 0; i < ld.m_command_buffers.size(); i++) {
			ld.m_command_buffers[i].draw_into(i,
				ld.m_render_pass,
				ld.m_swapchain,
				shader->m_pipeline,
				ld.m_descriptor_sets,
				*gpu,
				vertex_data,
				VkClearValue{ c.r, c.b, c.g, c.a }
			);
		}

	}

	m_render_commands.clear();
}
auto Vulkan_Renderer::present_2() -> void {
	VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;
	ld.present_frame(m_view_projection);
}

auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {

}

auto Vulkan_Renderer::take_screenshot(const char* filename) -> void {
	__debugbreak();
}
}