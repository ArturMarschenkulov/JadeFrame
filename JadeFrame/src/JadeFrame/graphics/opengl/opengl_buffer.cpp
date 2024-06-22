#include "opengl_buffer.h"
#include "pch.h"

#include "JadeFrame/graphics/mesh.h"
#include "opengl_context.h"

namespace JadeFrame {
namespace opengl {

static auto SHADER_TYPE_to_openGL_type(const SHADER_TYPE type) -> GLenum {
    GLenum result = {};
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
Buffer::Buffer()
    : m_context(nullptr)
    , m_type(TYPE::UNINIT)
    , m_size(0)
    , m_id(0) {}

Buffer::~Buffer() { this->reset(); }

Buffer::Buffer(Buffer&& other) noexcept
    : m_context(other.m_context)

    , m_type(other.m_type)
    , m_size(other.m_size)
    , m_id(other.release()) {

    other.m_context = nullptr;
    other.m_size = 0;
}

auto Buffer::operator=(Buffer&& other) noexcept -> Buffer& {
    m_id = other.release();
    m_type = other.m_type;
    m_context = other.m_context;
    m_size = other.m_size;

    other.m_context = nullptr;
    other.m_size = 0;
    return *this;
}

Buffer::Buffer(OpenGL_Context& context, TYPE type, void* data, GLuint size)
    : m_context(&context)

    , m_type(type)
    , m_size(size)
    , m_id(0) {

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
    u32 usage = 0;
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

auto Buffer::write(const void* data, GLuint size, GLint offset) const -> void {
    // glBufferSubData(buffer_type, offset, size, data);
    glNamedBufferSubData(m_id, offset, size, data);
}

auto Buffer::bind_base(GLuint binding_point) const -> void {
    if (m_type == TYPE::UNIFORM) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_id);
    } else {
        Logger::err("Buffer::bind_base() called on non-uniform buffer, only works with "
                    "uniform buffers");
        assert(false);
    }
}

auto Buffer::bind_buffer_range(GLuint index, GLintptr offset, GLsizeiptr size) const
    -> void {
    if (m_type == TYPE::UNIFORM) {
        glBindBufferRange(GL_UNIFORM_BUFFER, index, m_id, offset, size);
    } else {
        Logger::err(
            "Buffer::bind_buffer_range() called on non-uniform buffer, only works with "
            "uniform buffers"
        );
        assert(false);
    }
}

/*---------------------------
    GPUMeshData
---------------------------*/

auto GPUMeshData::operator=(GPUMeshData&& other) noexcept -> GPUMeshData& {
    m_vertex_buffer = other.m_vertex_buffer;
    m_index_buffer = other.m_index_buffer;

    m_vertex_buffer = other.m_vertex_buffer;
    m_index_buffer = other.m_index_buffer;

    return *this;
}

GPUMeshData::GPUMeshData(
    OpenGL_Context&   context,
    const VertexData& vertex_data,
    bool              interleaved
)
    : m_vertex_buffer()
    , m_index_buffer() {

    const std::vector<f32> flat_data = convert_into_data(vertex_data, interleaved);

    void* data = (void*)flat_data.data();
    u32   data_size = static_cast<u32>(flat_data.size() * sizeof(f32));
    m_vertex_buffer = context.create_buffer(Buffer::TYPE::VERTEX, data, data_size);

    if (!vertex_data.m_indices.empty()) {
        void* data = (void*)vertex_data.m_indices.data();
        u32   data_size = static_cast<u32>(vertex_data.m_indices.size() * sizeof(u32));
        m_index_buffer = context.create_buffer(Buffer::TYPE::INDEX, data, data_size);
    }
}

} // namespace opengl
} // namespace JadeFrame