#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "vulkan_shader.h"
#include "../opengl/opengl_renderer.h"

#include "../graphics_shared.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {


Vulkan_Renderer::Vulkan_Renderer(const Windows_Window& window)
	: m_context(window) {

}
auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void {
	m_clear_color = color;
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
	VulkanLogicalDevice& d = m_context.m_instance.m_logical_device;
	//for (u32 i = 0; i < d.m_command_buffers.size(); i++) {
	//	d.m_command_buffers[i].reset();
	//}
	static u32 frame_num = 0;
	Logger::log(Logger::LEVEL::INFO, "Frame {}", frame_num );
	frame_num++;

	VkResult result;
	d.m_in_flight_fences[d.m_current_frame].wait_for_fences();
	const u32 image_index = d.m_swapchain.acquire_next_image(&d.m_image_available_semaphores[d.m_current_frame], nullptr, result);
	if (result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
			d.recreate_swapchain();
			return;
		} else if (result == VK_SUBOPTIMAL_KHR) {
			std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
			//this->recreate_swapchain();
		} else {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}
	const VulkanImage& image = d.m_swapchain.m_images[image_index];
	d.m_present_image_index = image_index;

	if (d.m_images_in_flight[image_index].m_handle != VK_NULL_HANDLE) {
		d.m_images_in_flight[image_index].wait_for_fences();
	}
	d.m_images_in_flight[image_index].m_handle = d.m_in_flight_fences[d.m_current_frame].m_handle;


	VulkanCommandBuffer& cb = d.m_command_buffers[image_index];
	cb.record_begin();
	{
		const auto& c = m_clear_color;
		cb.render_pass_begin(d.m_swapchain.m_framebuffers[image_index], d.m_render_pass, d.m_swapchain, { c.r, c.g, c.b, c.a });
		{
			const std::array<Matrix4x4, 1>& matrices = { view_projection };
			d.m_ub_cam.map_to_GPU((void*)matrices.data(), 0, sizeof(matrices));


			const size_t min_ubo_alignment = m_context.m_instance.m_physical_device.m_properties.limits.minUniformBufferOffsetAlignment;
			size_t block_size = sizeof(Matrix4x4);
			if(min_ubo_alignment > 0) {
				block_size = (block_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1);
			}
			u32 draw_call = 0;
			for (size_t i = 0; i < m_render_commands.size(); i++) {
				const u32 offset = block_size * i;
				const Vulkan_Shader& shader = *static_cast<Vulkan_Shader*>(m_render_commands[i].material_handle->m_shader_handle->m_handle);
				const Vulkan_GPUMeshData& gpu_data = *static_cast<Vulkan_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);
				const VertexData& vertex_data = *m_render_commands[i].vertex_data;
				const Matrix4x4& transform = *m_render_commands[i].transform;


				Logger::log(Logger::LEVEL::INFO, "Draw Call {}", draw_call);
				draw_call++;


				const std::array<Matrix4x4, 1>& matrices2 = { transform };
				d.m_ub_tran.map_to_GPU((void*)matrices2.data(), offset, sizeof(matrices2));

				vkCmdBindPipeline(cb.m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.m_pipeline.m_graphics_pipeline);
				VkBuffer vertex_buffers[] = { gpu_data.m_vertex_buffer.m_handle };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cb.m_handle, 0, 1, vertex_buffers, offsets);
				vkCmdBindDescriptorSets(cb.m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.m_pipeline.m_pipeline_layout, 0, 1, &d.m_descriptor_sets[0].m_handle, 1, &offset);
				//vkCmdBindDescriptorSets(cb.m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.m_pipeline.m_pipeline_layout, 0, 1, &d.m_ds[0].m_handle, 0, nullptr);
				//vkCmdPushConstants(cb.m_handle, shader.m_pipeline.m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrix4x4), &transform);
				if (vertex_data.m_indices.size() > 0) {
					vkCmdBindIndexBuffer(cb.m_handle, gpu_data.m_index_buffer.m_handle, 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(cb.m_handle, vertex_data.m_indices.size(), 1, 0, 0, 0);
				} else {
					vkCmdDraw(cb.m_handle, static_cast<u32>(vertex_data.m_positions.size()), 1, 0, 0);
				}
			}
		}
		cb.render_pass_end();
	}
	cb.record_end();


	std::array<VkSemaphore, 1> wait_semaphores = { d.m_image_available_semaphores[d.m_current_frame].m_handle };
	std::array<VkSemaphore, 1> signal_semaphores = { d.m_render_finished_semaphores[d.m_current_frame].m_handle };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	d.m_in_flight_fences[d.m_current_frame].reset();

	const VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = wait_semaphores.size(),
		.pWaitSemaphores = wait_semaphores.data(),
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = &d.m_command_buffers[image_index].m_handle,
		.signalSemaphoreCount = signal_semaphores.size(),
		.pSignalSemaphores = signal_semaphores.data(),
	};
	//result = vkQueueSubmit(d.m_graphics_queue.m_handle, 1, &submit_info, d.m_in_flight_fences[d.m_current_frame].m_handle);
	//if (result != VK_SUCCESS) __debugbreak();
	d.m_graphics_queue.submit(submit_info, &d.m_in_flight_fences[d.m_current_frame]);


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


auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {

}

auto Vulkan_Renderer::take_screenshot(const char* filename) -> void {
	__debugbreak();
}
}