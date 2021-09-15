#pragma once
#include <glad/glad.h>

#include "opengl_wrapper.h"

#include "JadeFrame/graphics/mesh.h"

#include <cstdint>
#include <string>
#include <vector>

namespace JadeFrame {
struct Vertex;

enum class SHADER_TYPE {
	NONE = 0,
	FLOAT, FLOAT_2, FLOAT_3, FLOAT_4,
	MAT_3, MAT_4,
	INT, INT_2, INT_3, INT_4,
	BOOL,
	SAMPLER_1D, SAMPLER_2D, SAMPLER_3D, SAMPLER_CUBE,
};



class BufferLayout {
public:
	struct BufferElement {
		std::string name;
		SHADER_TYPE type;
		u32 size;
		size_t offset;
		bool normalized;

		BufferElement(SHADER_TYPE type, const std::string& name, bool normalized = false);
	};

public:
	BufferLayout() = default;
	BufferLayout(const std::initializer_list<BufferElement>& elements);
	BufferLayout(const BufferLayout&) = default;
	auto operator=(const BufferLayout&) -> BufferLayout& = default;
	auto calculate_offset_and_stride(std::vector<BufferElement>& elements) -> void;

	std::vector<BufferElement> m_elements;
	u8 m_stride = 0;

};

class OpenGL_GPUMeshData {
public:
	OpenGL_GPUMeshData(const Mesh& mesh, BufferLayout buffer_layout, bool interleaved = true);
	auto bind() const -> void;
	auto set_layout(const BufferLayout& buffer_layout) -> void;
public://private:
	OGLW_Buffer<GL_ARRAY_BUFFER> m_vertex_buffer;
	OGLW_VertexArray m_vertex_array;
	OGLW_Buffer<GL_ELEMENT_ARRAY_BUFFER> m_index_buffer;
	BufferLayout m_buffer_layout;
};
}