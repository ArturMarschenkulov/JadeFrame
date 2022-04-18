#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "vulkan_shader.h"
#include "../opengl/opengl_renderer.h"

#include "../graphics_shared.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {


Vulkan_Renderer::Vulkan_Renderer(const Window& window)
	: m_context(window) {

}
auto Vulkan_Renderer::set_clear_color(const RGBAColor& color) -> void {
	m_clear_color = color;
}



auto Vulkan_Renderer::clear_background() -> void {

}

auto Vulkan_Renderer::submit(const Object& obj) -> void {
	const VulkanLogicalDevice& d = m_context.m_instance.m_logical_device;

	if (false == obj.m_GPU_mesh_data.m_is_initialized) {
		assert(obj.m_vertex_format.m_attributes.size() > 0);

		obj.m_GPU_mesh_data.m_handle = new Vulkan_GPUMeshData(d, *obj.m_vertex_data, obj.m_vertex_format);
		obj.m_GPU_mesh_data.m_is_initialized = true;
	}

	MaterialHandle* mh = obj.m_material_handle;
	ShaderHandle* sh = obj.m_material_handle->m_shader_handle;
	if (mh->m_is_initialized == false) {
		mh->m_shader_handle->m_api = GRAPHICS_API::VULKAN;

		Vulkan_Shader::DESC shader_desc;
		shader_desc.code = sh->m_code;
		shader_desc.vertex_format = sh->m_vertex_format;
		mh->m_shader_handle->m_handle = new Vulkan_Shader(d, shader_desc);

		if (mh->m_texture_handle != nullptr) {
			mh->m_texture_handle->m_api = GRAPHICS_API::VULKAN;
			//mh->m_texture_handle->init();
			Vulkan_Texture* texture = new Vulkan_Texture();
			texture->init(d,
				mh->m_texture_handle->m_data,
				{ static_cast<u32>(mh->m_texture_handle->m_size.x),
				static_cast<u32>(mh->m_texture_handle->m_size.y) },
				VK_FORMAT_R8G8B8A8_SRGB
			);
			mh->m_texture_handle->m_handle = texture;
		}
		mh->m_is_initialized = true;
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
	VulkanLogicalDevice& d = m_context.m_instance.m_logical_device;

	d.m_in_flight_fences[d.m_current_frame].wait_for_fences();
	const u32 image_index = d.m_swapchain.acquire_next_image(&d.m_image_available_semaphores[d.m_current_frame], nullptr);

	if (d.m_swapchain.m_is_recreated == true) {
		d.m_swapchain.m_is_recreated = false;
		return;
	}

	//const VulkanImage& image = d.m_swapchain.m_images[image_index];
	d.m_present_image_index = image_index;

	if (d.m_images_in_flight[image_index].m_handle != VK_NULL_HANDLE) {
		d.m_images_in_flight[image_index].wait_for_fences();
	}
	d.m_images_in_flight[image_index].m_handle = d.m_in_flight_fences[d.m_current_frame].m_handle;


	VulkanCommandBuffer& cb = d.m_command_buffers[image_index];
	cb.record_begin();
	{
		const auto& c = m_clear_color;
		cb.render_pass_begin(d.m_swapchain.m_framebuffers[image_index], d.m_swapchain.m_render_pass, d.m_swapchain.m_extent, { {c.r, c.g, c.b, c.a }});
		{
			//Per Frame ubo
			d.m_ub_cam.send((void*)&view_projection, 0, sizeof(view_projection));

			const u64 min_ubo_alignment = m_context.m_instance.m_physical_device.m_properties.limits.minUniformBufferOffsetAlignment;
			const u64 block_size = sizeof(Matrix4x4);
			const u64 aligned_block_size
				= min_ubo_alignment > 0
				? (block_size + min_ubo_alignment - 1) & ~(min_ubo_alignment - 1)
				: block_size
				;

			//Update ubo buffer and descriptor set when the amount of render commands changes
			if (m_render_commands.size() * aligned_block_size != d.m_ub_tran.m_size) {
				d.m_ub_tran.resize(m_render_commands.size() * aligned_block_size);
				d.m_descriptor_sets[0].readd_uniform_buffer(1, d.m_ub_tran);
				d.m_descriptor_sets[0].update();

			}

			for (u64 i = 0; i < m_render_commands.size(); i++) {
				const u32 offset = static_cast<u32>(aligned_block_size * i);
				const Vulkan_Shader& shader = *static_cast<Vulkan_Shader*>(m_render_commands[i].material_handle->m_shader_handle->m_handle);
				const Vulkan_GPUMeshData& gpu_data = *static_cast<Vulkan_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);
				const VertexData& vertex_data = *m_render_commands[i].vertex_data;
				const Matrix4x4& transform = *m_render_commands[i].transform;

				//Per DrawCall ubo
				d.m_ub_tran.send((void*)&transform, offset, sizeof(transform));



				vkCmdBindPipeline(cb.m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.m_pipeline.m_handle);
				VkBuffer vertex_buffers[] = { gpu_data.m_vertex_buffer.m_handle };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(cb.m_handle, 0, 1, vertex_buffers, offsets);
				vkCmdBindDescriptorSets(cb.m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.m_pipeline.m_layout, 0, 1, &d.m_descriptor_sets[0].m_handle, d.m_descriptor_sets[0].m_layout->m_dynamic_count, &offset);


				if (vertex_data.m_indices.size() > 0) {
					vkCmdBindIndexBuffer(cb.m_handle, gpu_data.m_index_buffer.m_handle, 0, VK_INDEX_TYPE_UINT32);
					vkCmdDrawIndexed(cb.m_handle, static_cast<u32>(vertex_data.m_indices.size()), 1, 0, 0, 0);
				} else {
					vkCmdDraw(cb.m_handle, static_cast<u32>(vertex_data.m_positions.size()), 1, 0, 0);
				}
			}
		}
		cb.render_pass_end();
	}
	cb.record_end();

	d.m_in_flight_fences[d.m_current_frame].reset();

	d.m_graphics_queue.submit(
		d.m_command_buffers[image_index],
		&d.m_image_available_semaphores[d.m_current_frame],
		&d.m_render_finished_semaphores[d.m_current_frame],
		&d.m_in_flight_fences[d.m_current_frame]
	);
	m_render_commands.clear();
}

auto Vulkan_Renderer::present() -> void {
	VkResult result;
	VulkanLogicalDevice& d = m_context.m_instance.m_logical_device;

	d.m_present_queue.present(d.m_present_image_index, d.m_swapchain, &d.m_render_finished_semaphores[d.m_current_frame], &result);
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || d.m_framebuffer_resized) {
			d.m_framebuffer_resized = false;
			std::cout << "recreate because of vkQueuePresentKHR" << std::endl;
			//__debugbreak();
			//d.recreate_swapchain();
			d.m_swapchain.recreate();
		} else if (result != VK_SUCCESS) {
			std::cout << "failed to present swap chain image!" << std::endl;
            assert(false);
		}
	}

	d.m_current_frame = (d.m_current_frame + 1) % 2/*MAX_FRAMES_IN_FLIGHT*/;
}


auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {

}

auto Vulkan_Renderer::take_screenshot(const char* /*filename*/) -> void {
    assert(false);
}
}