#include "pch.h"

#include "vulkan_renderer.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "../opengl/opengl_renderer.h"

#include "../shared.h"

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
	static bool temp_bool = false;
	if (temp_bool == false) {
		const std::vector<VVertex> vertices = {
			{{-0.5f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
			{{+0.5f, -0.5f}, {+0.0f, +1.0f, +0.0f}},
			{{+0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}},

			{{-0.5f, +0.5f}, {+1.0f, +1.0f, +1.0f}},
		};
		const std::vector<u16> indices = {
			0, 1, 2,
			2, 3, 0,
		};
		VulkanBuffer m_vertex_buffer = { VULKAN_BUFFER_TYPE::VERTEX };
		VulkanBuffer m_index_buffer = { VULKAN_BUFFER_TYPE::INDEX };
		VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;

		m_vertex_buffer.init(ld, VULKAN_BUFFER_TYPE::VERTEX, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
		m_index_buffer.init(ld, VULKAN_BUFFER_TYPE::INDEX, (void*)indices.data(), sizeof(indices[0]) * indices.size());

		VulkanPipeline m_pipeline;
		m_pipeline.init(ld, ld.m_swapchain.m_extent, ld.m_descriptor_set_layout, ld.m_render_pass, GLSLCodeLoader::get_by_name("spirv_test_0"));


		const auto& c = m_clear_color;
		ld.draw_into_command_buffers(
			ld.m_render_pass,
			ld.m_swapchain,
			m_pipeline,
			ld.m_descriptor_sets,
			m_vertex_buffer,
			m_index_buffer,
			indices,
			VkClearValue{ c.r, c.b, c.g, c.a }
		);

		temp_bool = true;
	}
	if(obj.m_GPU_mesh_data.m_is_initialized == false) {
		BufferLayout buffer_layout;
		//In case there is no buffer layout provided use a default one
		if (obj.m_buffer_layout.m_elements.size() == 0) {
			const BufferLayout bl = {
				{ SHADER_TYPE::FLOAT_3, "v_position" },
				{ SHADER_TYPE::FLOAT_4, "v_color" },
				{ SHADER_TYPE::FLOAT_2, "v_texture_coord" },
				{ SHADER_TYPE::FLOAT_3, "v_normal" },
			};
			buffer_layout = bl;
		} else {
			buffer_layout = obj.m_buffer_layout;
		}

		obj.m_GPU_mesh_data.m_handle = new Vulkan_GPUMeshData(m_context.m_instance.m_logical_device, *obj.m_mesh, buffer_layout);
		obj.m_GPU_mesh_data.m_is_initialized = true;
	}
	if (obj.m_material_handle->m_is_initialized == false) {
		//obj.m_material_handle->init();
		obj.m_material_handle->m_shader_handle->api = ShaderHandle::API::VULKAN;
		obj.m_material_handle->m_shader_handle->init();
		//obj.m_material_handle->m_shader_handle->m_handle = new Vulkan_Shader();

		if (obj.m_material_handle->m_texture_handle != nullptr) {
			obj.m_material_handle->m_texture_handle->init();
		}
		obj.m_material_handle->m_is_initialized = true;
	}

	const Vulkan_RenderCommand command = {
		.transform = &obj.m_transform,
		.mesh = obj.m_mesh,
		.material_handle = obj.m_material_handle,
		.m_GPU_mesh_data = &obj.m_GPU_mesh_data,
	};
	m_render_commands.push_back(command);
}
auto Vulkan_Renderer::render(const Matrix4x4& view_projection) -> void {
	m_view_projection = view_projection;
	auto& device = m_context.m_instance.m_logical_device;

	//device.draw_into_command_buffers()
}
auto Vulkan_Renderer::present() -> void {
	m_context.m_instance.m_logical_device.draw_frame(m_view_projection);
}

auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {

}

auto Vulkan_Renderer::take_screenshot(const char* filename) -> void {
	__debugbreak();
}
}