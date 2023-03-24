#include "opengl_buffer.h"
#include "pch.h"

#include "JadeFrame/graphics/mesh.h"
#include "opengl_context.h"

namespace JadeFrame {
namespace opengl {

static auto SHADER_TYPE_to_openGL_type(const SHADER_TYPE type) -> GLenum {
    GLenum result;
    switch (type) {
        case SHADER_TYPE::F32:
        case SHADER_TYPE::V_2_F32:
        case SHADER_TYPE::V_3_F32:
        case SHADER_TYPE::V_4_F32: result = GL_FLOAT; break;
        default:
            assert(false);
            result = 0;
            break;
    }
    return result;
}

Buffer::Buffer() {
    m_id = 0;
    m_type = TYPE::UNINIT;
    m_context = nullptr;
}
Buffer::~Buffer() { this->reset(); }

Buffer::Buffer(Buffer&& other) noexcept
    : m_id(other.release()) {

    m_type = other.m_type;
    m_context = other.m_context;

    other.m_context = nullptr;
}
auto Buffer::operator=(Buffer&& other) -> Buffer& {
    m_id = other.release();
    m_type = other.m_type;
    m_context = other.m_context;

    other.m_context = nullptr;
    return *this;
}

Buffer::Buffer(OpenGL_Context& context, TYPE type, void* data, GLuint size) {
    this->init(context, type);
    this->alloc(data, size);
}


auto Buffer::init(OpenGL_Context& context, TYPE type) -> void {
    m_context = &context;
    glCreateBuffers(1, &m_id);
    m_context->m_buffers.push_back(m_id);
    GLuint t;
    switch (type) {
        case TYPE::VERTEX: t = GL_ARRAY_BUFFER; break;
        case TYPE::INDEX: t = GL_ELEMENT_ARRAY_BUFFER; break;
        case TYPE::UNIFORM: t = GL_UNIFORM_BUFFER; break;
        default: assert(false); break;
    }
    glBindBuffer(t, m_id);
    m_type = type;
    m_context->m_bound_buffer = m_id;
}


auto Buffer::reserve(GLuint size) const -> void {
    // if NULL is passed in as data, it only reserves size_in_bytes bytes.
    // glBufferData(buffer_type, size_in_bytes, NULL, GL_STATIC_DRAW);
    // glNamedBufferData(m_id, size, NULL, GL_STATIC_DRAW);
    this->alloc(nullptr, size);
}
auto Buffer::alloc(void* data, GLuint size) const -> void {
    // glBufferData(buffer_type, size, data, GL_STATIC_DRAW);
    glNamedBufferData(m_id, size, data, GL_STATIC_DRAW);
}
auto Buffer::update(const void* data, GLint offset, GLuint size) const -> void {
    // glBufferSubData(buffer_type, offset, size, data);
    glNamedBufferSubData(m_id, offset, size, data);
}


auto GPUMeshData::operator=(GPUMeshData&& other) -> GPUMeshData& {
    m_vertex_buffer = std::move(other.m_vertex_buffer);
    m_index_buffer = std::move(other.m_index_buffer);

    m_vertex_buffer = std::move(other.m_vertex_buffer);
    m_vertex_array = std::move(other.m_vertex_array);
    m_index_buffer = std::move(other.m_index_buffer);
    m_vertex_format = std::move(other.m_vertex_format);

    return *this;
}
GPUMeshData::GPUMeshData(
    OpenGL_Context& context, const VertexData& vertex_data, VertexFormat vertex_format, bool interleaved)
    : m_vertex_buffer()
    , m_vertex_array()
    , m_index_buffer() {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void* data = (void*)flat_data.data();
    u64   data_size = flat_data.size() * sizeof(f32);
    m_vertex_buffer = context.create_buffer(Buffer::TYPE::VERTEX, data, data_size);

    m_vertex_array.bind();
    this->set_layout(vertex_format);

    if (vertex_data.m_indices.size() > 0) {
        void* data = (void*)vertex_data.m_indices.data();
        u64   data_size = vertex_data.m_indices.size() * sizeof(u32);
        m_index_buffer = context.create_buffer(Buffer::TYPE::INDEX, data, data_size);
    }
    m_vertex_array.unbind();
}

auto GPUMeshData::bind() const -> void { m_vertex_array.bind(); }

auto GPUMeshData::set_layout(const VertexFormat& vertex_format) -> void {
    m_vertex_format = vertex_format;

    i32 vertex_buffer_index = 0;
    for (size_t i = 0; i != vertex_format.m_attributes.size(); i++) {
        const VertexAttribute& attribute = vertex_format.m_attributes[i];

        switch (attribute.type) {
            case SHADER_TYPE::F32:
            case SHADER_TYPE::V_2_F32:
            case SHADER_TYPE::V_3_F32:
            case SHADER_TYPE::V_4_F32: {
                glEnableVertexAttribArray(vertex_buffer_index);
                // glEnableVertexArrayAttrib(m_ID, vertex_buffer_index);
                glVertexAttribPointer(
                    vertex_buffer_index,
                    get_component_count(attribute.type), // element.get_component_count(),
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

} // namespace opengl
} // namespace JadeFrame