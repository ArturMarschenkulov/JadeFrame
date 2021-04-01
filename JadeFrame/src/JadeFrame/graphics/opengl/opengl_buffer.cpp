#include "opengl_buffer.h"

#include "../mesh.h"

static auto SHADER_DATA_TYPE_get_size(const SHADER_DATA_TYPE type) -> uint32_t {
	switch (type) {
		case SHADER_DATA_TYPE::FLOAT:	return 4;
		case SHADER_DATA_TYPE::FLOAT_2:	return 4 * 2;
		case SHADER_DATA_TYPE::FLOAT_3:	return 4 * 3;
		case SHADER_DATA_TYPE::FLOAT_4:	return 4 * 4;
		case SHADER_DATA_TYPE::MAT_3:	return 4 * 3 * 3;
		case SHADER_DATA_TYPE::MAT_4:	return 4 * 4 * 4;
		case SHADER_DATA_TYPE::INT:		return 4;
		case SHADER_DATA_TYPE::INT_2:	return 4 * 2;
		case SHADER_DATA_TYPE::INT_3:	return 4 * 3;
		case SHADER_DATA_TYPE::INT_4:	return 4 * 4;
		case SHADER_DATA_TYPE::BOOL:	return 1;
		default: __debugbreak(); return -1;
	}
	return 0;
}

//static auto SHADER_DATA_TYPE_freom_openGL_type(const GLenum type) -> GLenum {
//	switch (type) {
//		case SHADER_DATA_TYPE::FLOAT:
//		case SHADER_DATA_TYPE::FLOAT_2:
//		case SHADER_DATA_TYPE::FLOAT_3:
//		case SHADER_DATA_TYPE::FLOAT_4: return GL_FLOAT;
//		default: __debugbreak(); return 0;
//	}
//}
static auto SHADER_DATA_TYPE_get_component_count(const SHADER_DATA_TYPE type) -> uint32_t {
	switch (type) {
		//case SHADER_DATA_TYPE::FLOAT:	return 1;
		//case SHADER_DATA_TYPE::FLOAT_2:	return 2;
		//case SHADER_DATA_TYPE::FLOAT_3:	return 3;
		//case SHADER_DATA_TYPE::FLOAT_4:	return 4;
		//case SHADER_DATA_TYPE::MAT_3:	return 3; // 3* float3
		//case SHADER_DATA_TYPE::MAT_4:	return 4; // 4* float4
		//case SHADER_DATA_TYPE::INT:		return 1;
		//case SHADER_DATA_TYPE::INT_2:	return 2;
		//case SHADER_DATA_TYPE::INT_3:	return 3;
		//case SHADER_DATA_TYPE::INT_4:	return 4;
		//case SHADER_DATA_TYPE::BOOL:	return 1;
		case SHADER_DATA_TYPE::FLOAT:
		case SHADER_DATA_TYPE::INT:
		case SHADER_DATA_TYPE::BOOL:	return 1;

		case SHADER_DATA_TYPE::FLOAT_2:
		case SHADER_DATA_TYPE::INT_2:	return 2;

		case SHADER_DATA_TYPE::FLOAT_3:
		case SHADER_DATA_TYPE::MAT_3:// 3* float3
		case SHADER_DATA_TYPE::INT_3:	return 3;

		case SHADER_DATA_TYPE::FLOAT_4:
		case SHADER_DATA_TYPE::MAT_4:	 // 4* float4
		case SHADER_DATA_TYPE::INT_4:	return 4;

		default: __debugbreak(); return -1;
	}
}

BufferLayout::BufferElement::BufferElement(SHADER_DATA_TYPE type, const std::string& name, bool normalized)
	: name(name), type(type), size(SHADER_DATA_TYPE_get_size(type)), offset(0), normalized(normalized) {
}
BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
	: m_elements(elements) {
	this->calculate_offset_and_stride();
}
//auto BufferLayout::BufferElement::get_component_count() const -> uint32_t {
//	switch (type) {
//		case SHADER_DATA_TYPE::FLOAT:	return 1;
//		case SHADER_DATA_TYPE::FLOAT_2:	return 2;
//		case SHADER_DATA_TYPE::FLOAT_3:	return 3;
//		case SHADER_DATA_TYPE::FLOAT_4:	return 4;
//		case SHADER_DATA_TYPE::MAT_3:	return 3; // 3* float3
//		case SHADER_DATA_TYPE::MAT_4:	return 4; // 4* float4
//		case SHADER_DATA_TYPE::INT:		return 1;
//		case SHADER_DATA_TYPE::INT_2:	return 2;
//		case SHADER_DATA_TYPE::INT_3:	return 3;
//		case SHADER_DATA_TYPE::INT_4:	return 4;
//		case SHADER_DATA_TYPE::BOOL:	return 1;
//		default: __debugbreak(); return -1;
//	}
//}
auto BufferLayout::calculate_offset_and_stride() -> void {
	size_t offset = 0;
	m_stride = 0;
	for (BufferElement& element : m_elements) {
		element.offset = offset;
		offset += element.size;
		m_stride += element.size;
	}
}


auto SHADER_DATA_TYPE_to_openGL_type(const SHADER_DATA_TYPE type) -> GLenum {
	switch (type) {
		case SHADER_DATA_TYPE::FLOAT:
		case SHADER_DATA_TYPE::FLOAT_2:
		case SHADER_DATA_TYPE::FLOAT_3:
		case SHADER_DATA_TYPE::FLOAT_4: return GL_FLOAT;
		default: __debugbreak(); return 0;
	}
}

auto SHADER_TYPE_from_openGL_enum(const GLenum type) -> SHADER_DATA_TYPE {
	switch (type) {
		case GL_FLOAT: return SHADER_DATA_TYPE::FLOAT;  break;
		case GL_FLOAT_VEC2: return SHADER_DATA_TYPE::FLOAT_2;  break;
		case GL_FLOAT_VEC3: return SHADER_DATA_TYPE::FLOAT_3; break;
		case GL_FLOAT_VEC4: return SHADER_DATA_TYPE::FLOAT_4; break;
		case GL_FLOAT_MAT4: return SHADER_DATA_TYPE::MAT_4; break;
		case GL_SAMPLER_2D: return SHADER_DATA_TYPE::SAMPLER_2D; break;
		default: __debugbreak(); return {};
	}
}



auto OpenGL_VertexArray::set_layout(const BufferLayout& buffer_layout) -> void {
	m_buffer_layout = buffer_layout;
	int vertex_buffer_index = 0;
	for (size_t i = 0; i != buffer_layout.m_elements.size(); i++) {
		const BufferLayout::BufferElement& element = buffer_layout.m_elements[i];

		switch (element.type) {
			case SHADER_DATA_TYPE::FLOAT:
			case SHADER_DATA_TYPE::FLOAT_2:
			case SHADER_DATA_TYPE::FLOAT_3:
			case SHADER_DATA_TYPE::FLOAT_4:
			{
				glEnableVertexAttribArray(vertex_buffer_index);
				//glEnableVertexArrayAttrib(m_ID, vertex_buffer_index);
				glVertexAttribPointer(
					vertex_buffer_index,
					SHADER_DATA_TYPE_get_component_count(element.type), //element.get_component_count(),
					SHADER_DATA_TYPE_to_openGL_type(element.type),
					element.normalized ? GL_TRUE : GL_FALSE,
					buffer_layout.m_stride,
					(const void*)element.offset //reinterpret_cast<const void*>(element.offset)
				);
				vertex_buffer_index++;
			} break;
			default:
			{
				__debugbreak();
			}

		}
	}
}