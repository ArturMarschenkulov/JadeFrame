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
	static bool temp_bool = false;
	if (temp_bool == false) {
		const std::vector<VVertex> vertices = {
			{{-0.5f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
			{{+0.5f, -0.5f}, {+0.0f, +1.0f, +0.0f}},
			{{+0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}},

			{{-0.5f, +0.5f}, {+1.0f, +1.0f, +1.0f}},
		};
		const std::vector<u32> indices = {
			0, 1, 2,
			2, 3, 0,
		};

		
		const f32 s = 0.5f;
		VertexData vertex_data;
		vertex_data.m_positions = {
			{ -s, -s, 0.0f },
			{ +s, -s, 0.0f },
			{ +s, +s, 0.0f },
			{ -s, +s, 0.0f }
		};

		vertex_data.m_colors = {
			{ 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f }
		};

		vertex_data.m_indices = {
			0, 1, 2,
			2, 3, 0
		};

		//Mesh mesh;
		//mesh.add_to_data(vertex_data);
		//convert_into_data(mesh, true);

		VulkanBuffer vertex_buffer = { VULKAN_BUFFER_TYPE::VERTEX };
		VulkanBuffer index_buffer = { VULKAN_BUFFER_TYPE::INDEX };
		VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;


		vertex_buffer.init(ld, VULKAN_BUFFER_TYPE::VERTEX, (void*)vertices.data(), sizeof(vertices[0]) * vertices.size());
		index_buffer.init(ld, VULKAN_BUFFER_TYPE::INDEX, (void*)vertex_data.m_indices.data(), sizeof(vertex_data.m_indices[0]) * vertex_data.m_indices.size());


		VertexFormat vf = {
			{ "v_position", SHADER_TYPE::FLOAT_2 },
			{ "v_color", SHADER_TYPE::FLOAT_3 },
		};
		VulkanPipeline pipeline;
		pipeline.init(ld, ld.m_swapchain.m_extent, ld.m_descriptor_set_layout, ld.m_render_pass, GLSLCodeLoader::get_by_name("spirv_test_0"), vf);


		const auto& c = m_clear_color;
		ld.m_command_buffers.draw_into(
			ld.m_render_pass,
			ld.m_swapchain,
			pipeline,
			ld.m_descriptor_sets,
			vertex_buffer,
			index_buffer,
			indices,
			VkClearValue{ c.r, c.b, c.g, c.a }
		);

		temp_bool = true;
	}
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

		obj.m_GPU_mesh_data.m_handle = new Vulkan_GPUMeshData(m_context.m_instance.m_logical_device, *obj.m_mesh, vertex_format);
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
		.mesh = obj.m_mesh,
		.material_handle = obj.m_material_handle,
		.m_GPU_mesh_data = &obj.m_GPU_mesh_data,
	};
	m_render_commands.push_back(command);
}
auto Vulkan_Renderer::render(const Matrix4x4& view_projection) -> void {
	m_view_projection = view_projection;
	auto& device = m_context.m_instance.m_logical_device;

	for (size_t i = 0; i < m_render_commands.size(); i++) {
		VulkanLogicalDevice& ld = m_context.m_instance.m_logical_device;
		Vulkan_Shader& shader = *static_cast<Vulkan_Shader*>(m_render_commands[i].material_handle->m_shader_handle->m_handle);
		Vulkan_GPUMeshData& gpu = *static_cast<Vulkan_GPUMeshData*>(m_render_commands[i].m_GPU_mesh_data->m_handle);
		auto mesh = m_render_commands[i].mesh;

		//const auto& c = m_clear_color;
		//ld.m_command_buffers.draw_into(
		//	ld.m_render_pass,
		//	ld.m_swapchain,
		//	shader.m_pipeline,
		//	ld.m_descriptor_sets,
		//	gpu.m_vertex_buffer,
		//	gpu.m_index_buffer,
		//	mesh->m_indices,
		//	VkClearValue{ c.r, c.b, c.g, c.a }
		//);
	}

	//device.draw_into_command_buffers();
}
auto Vulkan_Renderer::present() -> void {
	m_context.m_instance.m_logical_device.present_frame(m_view_projection);
}

auto Vulkan_Renderer::set_viewport(u32 /*x*/, u32 /*y*/, u32 /*width*/, u32 /*height*/) const -> void {

}

auto Vulkan_Renderer::take_screenshot(const char* filename) -> void {
	__debugbreak();
}
}