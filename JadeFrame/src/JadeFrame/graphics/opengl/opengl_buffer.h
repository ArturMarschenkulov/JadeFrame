#pragma once
#include <glad/glad.h>

#include "opengl_wrapper.h"
#include "../shared.h"

#include "JadeFrame/graphics/mesh.h"

#include <cstdint>
#include <string>
#include <vector>

namespace JadeFrame {
struct Vertex;

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