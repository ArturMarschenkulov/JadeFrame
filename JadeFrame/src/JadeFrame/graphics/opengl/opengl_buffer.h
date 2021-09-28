#pragma once
#include <glad/glad.h>

#include "opengl_wrapper.h"
#include "../graphics_shared.h"

#include "JadeFrame/graphics/mesh.h"

#include <cstdint>
#include <string>
#include <vector>

namespace JadeFrame {
struct Vertex;

class OpenGL_GPUMeshData {
public:
	OpenGL_GPUMeshData(const VertexData& vertex_data, VertexFormat vertex_format, bool interleaved = true);
	auto bind() const -> void;
	auto set_layout(const VertexFormat& vertex_format) -> void;
public://private:
	OGLW_Buffer<GL_ARRAY_BUFFER> m_vertex_buffer;
	OGLW_VertexArray m_vertex_array;
	OGLW_Buffer<GL_ELEMENT_ARRAY_BUFFER> m_index_buffer;
	VertexFormat m_vertex_format;
};
}