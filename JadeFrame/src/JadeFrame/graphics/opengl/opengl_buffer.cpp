#include "pch.h"
#include "opengl_buffer.h"

#include "JadeFrame/graphics/mesh.h"

namespace JadeFrame {

static auto SHADER_TYPE_to_openGL_type(const SHADER_TYPE type) -> GLenum {
	GLenum result;
	switch (type) {
		case SHADER_TYPE::FLOAT:
		case SHADER_TYPE::FLOAT_2:
		case SHADER_TYPE::FLOAT_3:
		case SHADER_TYPE::FLOAT_4: result = GL_FLOAT; break;
		default: __debugbreak(); result = 0; break;
	}
	return result;
}
static auto SHADER_TYPE_get_size(const SHADER_TYPE type) -> u32 {
	u32 result;
	switch (type) {
		case SHADER_TYPE::FLOAT:	result = 4; break;
		case SHADER_TYPE::FLOAT_2:	result = 4 * 2; break;
		case SHADER_TYPE::FLOAT_3:	result = 4 * 3; break;
		case SHADER_TYPE::FLOAT_4:	result = 4 * 4; break;
		case SHADER_TYPE::MAT_3:	result = 4 * 3 * 3; break;
		case SHADER_TYPE::MAT_4:	result = 4 * 4 * 4; break;
		case SHADER_TYPE::INT:		result = 4; break;
		case SHADER_TYPE::INT_2:	result = 4 * 2; break;
		case SHADER_TYPE::INT_3:	result = 4 * 3; break;
		case SHADER_TYPE::INT_4:	result = 4 * 4; break;
		case SHADER_TYPE::BOOL:	result = 1; break;
		default: __debugbreak(); result = 0; break;
	}
	return result;
}
static auto SHADER_TYPE_get_component_count(const SHADER_TYPE type) -> u32 {
	u32 result;
	switch (type) {
		case SHADER_TYPE::FLOAT:
		case SHADER_TYPE::INT:
		case SHADER_TYPE::BOOL:	result = 1; break;

		case SHADER_TYPE::FLOAT_2:
		case SHADER_TYPE::INT_2:	result = 2; break;

		case SHADER_TYPE::FLOAT_3:
		case SHADER_TYPE::MAT_3:// 3* float3
		case SHADER_TYPE::INT_3:	result = 3; break;

		case SHADER_TYPE::FLOAT_4:
		case SHADER_TYPE::MAT_4:	 // 4* float4
		case SHADER_TYPE::INT_4:	result = 4; break;

		default: __debugbreak(); result = 0; break;
	}
	return result;
}

BufferLayout::BufferElement::BufferElement(SHADER_TYPE type, const std::string& name, bool normalized)
	: name(name)
	, type(type)
	, size(SHADER_TYPE_get_size(type))
	, offset(0)
	, normalized(normalized) {
}
BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
	: m_elements(elements) {
	this->calculate_offset_and_stride(m_elements);
}
auto BufferLayout::calculate_offset_and_stride(std::vector<BufferElement>& elements) -> void {
	size_t offset = 0;
	m_stride = 0;
	for (BufferElement& element : elements) {
		element.offset = offset;
		offset += element.size;
		m_stride += element.size;
	}
}

static auto convert_into_data(const Mesh& mesh, const bool interleaved, bool with_color = true) -> std::vector<f32> {
	//assert(mesh.m_positions.size() == mesh.m_normals.size());
	const u64 size
		= mesh.m_positions.size() * 3
		+ mesh.m_colors.size() * 4
		+ mesh.m_tex_coords.size() * 2
		+ mesh.m_normals.size() * 3;

	std::vector<f32> data;
	data.reserve(size);
	if (interleaved == true) {
		for (size_t i = 0; i < mesh.m_positions.size(); i++) {
			data.push_back(mesh.m_positions[i].x);
			data.push_back(mesh.m_positions[i].y);
			data.push_back(mesh.m_positions[i].z);
			if (mesh.m_colors.size()) {
				data.push_back(mesh.m_colors[i].r);
				data.push_back(mesh.m_colors[i].g);
				data.push_back(mesh.m_colors[i].b);
				data.push_back(mesh.m_colors[i].a);
			}
			if (mesh.m_tex_coords.size()) {
				data.push_back(mesh.m_tex_coords[i].x);
				data.push_back(mesh.m_tex_coords[i].y);
			}
			if (mesh.m_normals.size()) {
				data.push_back(mesh.m_normals[i].x);
				data.push_back(mesh.m_normals[i].y);
				data.push_back(mesh.m_normals[i].z);
			}
		}
	} else {

		__debugbreak();
		for (size_t i = 0; i < mesh.m_positions.size(); i++) {
			data.push_back(mesh.m_positions[i].x);
			data.push_back(mesh.m_positions[i].y);
			data.push_back(mesh.m_positions[i].z);
		}
		for (size_t i = 0; i < mesh.m_colors.size(); i++) {
			data.push_back(mesh.m_colors[i].r);
			data.push_back(mesh.m_colors[i].g);
			data.push_back(mesh.m_colors[i].b);
			data.push_back(mesh.m_colors[i].a);
		}
		for (size_t i = 0; i < mesh.m_tex_coords.size(); i++) {
			data.push_back(mesh.m_tex_coords[i].x);
			data.push_back(mesh.m_tex_coords[i].y);
		}
	}

	return data;
}

OpenGL_GPUMeshData::OpenGL_GPUMeshData(const Mesh& mesh, BufferLayout buffer_layout, bool interleaved)
	: m_vertex_buffer()
	, m_vertex_array()
	, m_index_buffer() {


	const std::vector<f32> data = convert_into_data(mesh, interleaved);
	m_vertex_buffer.bind();
	m_vertex_buffer.send(data);

	m_vertex_array.bind();
	this->set_layout(buffer_layout);

	if (mesh.m_indices.size() > 0) {
		m_index_buffer.bind();
		m_index_buffer.send(mesh.m_indices);

	}
	m_vertex_array.unbind();
	m_index_buffer.unbind();
	m_vertex_buffer.unbind();

}

auto OpenGL_GPUMeshData::bind() const -> void {
	m_vertex_array.bind();
}

auto OpenGL_GPUMeshData::set_layout(const BufferLayout& buffer_layout) -> void {
	m_buffer_layout = buffer_layout;

	i32 vertex_buffer_index = 0;
	for (size_t i = 0; i != buffer_layout.m_elements.size(); i++) {
		const BufferLayout::BufferElement& element = buffer_layout.m_elements[i];

		switch (element.type) {
			case SHADER_TYPE::FLOAT:
			case SHADER_TYPE::FLOAT_2:
			case SHADER_TYPE::FLOAT_3:
			case SHADER_TYPE::FLOAT_4:
			{
				glEnableVertexAttribArray(vertex_buffer_index);
				//glEnableVertexArrayAttrib(m_ID, vertex_buffer_index);
				glVertexAttribPointer(
					vertex_buffer_index,
					SHADER_TYPE_get_component_count(element.type), //element.get_component_count(),
					SHADER_TYPE_to_openGL_type(element.type),
					element.normalized ? GL_TRUE : GL_FALSE,
					buffer_layout.m_stride,
					(const void*)element.offset //reinterpret_cast<const void*>(element.offset)
				);
				
			} break;
			default:
			{
				__debugbreak();
			}

		}
		vertex_buffer_index++;
	}
}



}