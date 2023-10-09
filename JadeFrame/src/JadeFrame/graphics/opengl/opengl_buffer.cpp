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

/*---------------------------
    Buffer
---------------------------*/
Buffer::Buffer() {
    m_id = 0;
    m_type = TYPE::UNINIT;
    m_context = nullptr;
    m_size = 0;
}
Buffer::~Buffer() { this->reset(); }

Buffer::Buffer(Buffer&& other) noexcept
    : m_id(other.release()) {

    m_type = other.m_type;
    m_context = other.m_context;
    m_size = other.m_size;

    other.m_context = nullptr;
    other.m_size = 0;
}
auto Buffer::operator=(Buffer&& other) -> Buffer& {
    m_id = other.release();
    m_type = other.m_type;
    m_context = other.m_context;
    m_size = other.m_size;

    other.m_context = nullptr;
    other.m_size = 0;
    return *this;
}

Buffer::Buffer(OpenGL_Context& context, TYPE type, void* data, GLuint size) {
    m_context = &context;
    m_type = type;
    m_size = size;
    glCreateBuffers(1, &m_id);

    this->alloc(data, size);

    // TODO: Move this registering into OpenGL_Context?
    m_context->m_buffers.push_back(m_id);
    m_context->m_bound_buffer = m_id;
}


auto Buffer::reserve(GLuint size) const -> void {
    if (size > m_size) { this->alloc(nullptr, size); }
}
auto Buffer::alloc(void* data, GLuint size) const -> void {
    i32 usage;
    switch (m_type) {
        case TYPE::UNIFORM: usage = GL_DYNAMIC_DRAW; break;
        default: usage = GL_STATIC_DRAW; break;
    }
    glNamedBufferData(m_id, size, data, usage);

    // GLbitfield flags;
    // switch (m_type) {
    //     case TYPE::UNIFORM: flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT; break;
    //     default: flags = 0; break;
    // }
    // glNamedBufferStorage(m_id, size, data, flags);
}
auto Buffer::write(const void* data, GLint offset, GLuint size) const -> void {
    // glBufferSubData(buffer_type, offset, size, data);
    glNamedBufferSubData(m_id, offset, size, data);
}


auto Buffer::bind_base(GLuint binding_point) const -> void {
    if (m_type == TYPE::UNIFORM) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_id);
    } else {
        Logger::err("Buffer::bind_base() called on non-uniform buffer, only works with uniform buffers");
        assert(false);
    }
}

auto Buffer::bind_buffer_range(GLuint index, GLintptr offset, GLsizeiptr size) const -> void {
    if (m_type == TYPE::UNIFORM) {
        glBindBufferRange(GL_UNIFORM_BUFFER, index, m_id, offset, size);
    } else {
        Logger::err("Buffer::bind_buffer_range() called on non-uniform buffer, only works with uniform buffers");
        assert(false);
    }
}

/*---------------------------
    GPUMeshData
---------------------------*/

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
    , m_index_buffer() 
    , m_vertex_format(vertex_format)
    , m_vertex_array() {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void* data = (void*)flat_data.data();
    u64   data_size = flat_data.size() * sizeof(f32);
    m_vertex_buffer = context.create_buffer(Buffer::TYPE::VERTEX, data, data_size);


    if (vertex_data.m_indices.size() > 0) {
        void* data = (void*)vertex_data.m_indices.data();
        u64   data_size = vertex_data.m_indices.size() * sizeof(u32);
        m_index_buffer = context.create_buffer(Buffer::TYPE::INDEX, data, data_size);
    }
    m_vertex_array = OGLW_VertexArray(&context, vertex_format);
}

} // namespace opengl
} // namespace JadeFrame