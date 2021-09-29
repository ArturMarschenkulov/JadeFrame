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
		obj.m_material_handle->m_shader_handle->api = ShaderHandle::API::VULKAN;
		//obj.m_material_handle->m_shader_handle->init();
		const VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;
		ShaderHandle* sh = obj.m_material_handle->m_shader_handle;

		Vulkan_Shader::DESC shader_desc;
		shader_desc.code = sh->m_code;
		shader_desc.buffer_layout = sh->m_vertex_format;
		obj.m_material_handle->m_shader_handle->m_handle = new Vulkan_Shader(ld, shader_desc);

		if (obj.m_material_handle->m_texture_handle != nullptr) {
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
		Vulkan_Shader* shader = static_cast<Vulkan_Shader*>(m_render_commands[i].material_handle->m_shader_handle->m_handle);
		Vulkan_GPUMeshData* gpu = static_cast<Vulkan_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);
		const VertexData& vertex_data = *m_render_commands[i].vertex_data;



		const Matrix4x4& transform = *m_render_commands[i].transform;
		const std::vector<Matrix4x4>& matrices = { view_projection , transform };
		ld.m_uniform_buffers[ld.m_present_image_index].map_to_GPU((void*)&matrices, sizeof(matrices));

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
auto Vulkan_Renderer::present() -> void {
	VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;
	ld.present_frame(m_view_projection);
}

auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {

}

auto Vulkan_Renderer::take_screenshot(const char* filename) -> void {
	__debugbreak();
}
}