#include "opengl_buffer.h"
#include "pch.h"

#include "JadeFrame/graphics/mesh.h"

namespace JadeFrame {

static auto SHADER_TYPE_to_openGL_type(const SHADER_TYPE type) -> GLenum {
    GLenum result;
    switch (type) {
        case SHADER_TYPE::FLOAT:
        case SHADER_TYPE::FLOAT_2:
        case SHADER_TYPE::FLOAT_3:
        case SHADER_TYPE::FLOAT_4: result = GL_FLOAT; break;
        default:
            assert(false);
            result = 0;
            break;
    }
    return result;
}

static auto SHADER_TYPE_get_component_count(const SHADER_TYPE type) -> u32 {
    u32 result;
    switch (type) {
        case SHADER_TYPE::FLOAT:
        case SHADER_TYPE::INT:
        case SHADER_TYPE::BOOL: result = 1; break;

        case SHADER_TYPE::FLOAT_2:
        case SHADER_TYPE::INT_2: result = 2; break;

        case SHADER_TYPE::FLOAT_3:
        case SHADER_TYPE::MAT_3: // 3* float3
        case SHADER_TYPE::INT_3: result = 3; break;

        case SHADER_TYPE::FLOAT_4:
        case SHADER_TYPE::MAT_4: // 4* float4
        case SHADER_TYPE::INT_4: result = 4; break;

        default:
            assert(false);
            result = 0;
            break;
    }
    return result;
}




OpenGL_GPUMeshData::OpenGL_GPUMeshData(const VertexData& vertex_data, VertexFormat vertex_format, bool interleaved)
    : m_vertex_buffer()
    , m_vertex_array()
    , m_index_buffer() {


    const std::vector<f32> data = convert_into_data(vertex_data, interleaved);
    m_vertex_buffer.bind();
    m_vertex_buffer.send(data);

    m_vertex_array.bind();
    this->set_layout(vertex_format);

    if (vertex_data.m_indices.size() > 0) {
        m_index_buffer.bind();
        m_index_buffer.send(vertex_data.m_indices);
    }
    m_vertex_array.unbind();
    m_index_buffer.unbind();
    m_vertex_buffer.unbind();
}

auto OpenGL_GPUMeshData::bind() const -> void { m_vertex_array.bind(); }

auto OpenGL_GPUMeshData::set_layout(const VertexFormat& vertex_format) -> void {
    m_vertex_format = vertex_format;

    i32 vertex_buffer_index = 0;
    for (size_t i = 0; i != vertex_format.m_attributes.size(); i++) {
        const VertexAttribute& attribute = vertex_format.m_attributes[i];

        switch (attribute.type) {
            case SHADER_TYPE::FLOAT:
            case SHADER_TYPE::FLOAT_2:
            case SHADER_TYPE::FLOAT_3:
            case SHADER_TYPE::FLOAT_4: {
                glEnableVertexAttribArray(vertex_buffer_index);
                // glEnableVertexArrayAttrib(m_ID, vertex_buffer_index);
                glVertexAttribPointer(
                    vertex_buffer_index,
                    SHADER_TYPE_get_component_count(attribute.type), // element.get_component_count(),
                    SHADER_TYPE_to_openGL_type(attribute.type), attribute.normalized ? GL_TRUE : GL_FALSE,
                    vertex_format.m_stride,
                    (const void*)attribute.offset // reinterpret_cast<const void*>(element.offset)
                );

            } break;
            default: {
                assert(false);
            }
        }
        vertex_buffer_index++;
    }
}



} // namespace JadeFrame