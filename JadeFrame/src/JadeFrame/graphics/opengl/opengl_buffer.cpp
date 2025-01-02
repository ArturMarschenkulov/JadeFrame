#include "opengl_buffer.h"

#include <utility>

#include "opengl_context.h"

namespace JadeFrame {
namespace opengl {

/*---------------------------
    Buffer
---------------------------*/
Buffer::Buffer()
    : m_context(nullptr)
    , m_type(TYPE::UNINIT)
    , m_size(0)
    , m_id(0) {}

Buffer::~Buffer() {
    glDeleteBuffers(1, &m_id);
    m_id = 0;
}

Buffer::Buffer(Buffer&& other) noexcept
    : m_context(std::exchange(other.m_context, nullptr))
    , m_type(other.m_type)
    , m_size(std::exchange(other.m_size, 0))
    , m_id(std::exchange(other.m_id, 0)) {}

auto Buffer::operator=(Buffer&& other) noexcept -> Buffer& {
    m_id = std::exchange(other.m_id, 0);
    m_type = other.m_type;
    m_context = std::exchange(other.m_context, nullptr);
    m_size = std::exchange(other.m_size, 0);
    return *this;
}

auto Buffer::create(OpenGL_Context& context, TYPE type, void* data, GLuint size)
    -> Buffer {
    Buffer buffer;
    buffer.m_context = &context;
    buffer.m_type = type;
    buffer.m_size = size;
    glCreateBuffers(1, &buffer.m_id);
    buffer.alloc(data, size);

    // TODO: Move this registering into OpenGL_Context?
    context.m_buffers.push_back(buffer.m_id);
    context.m_bound_buffer = buffer.m_id;
    return buffer;
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
    glNamedBufferSubData(m_id, offset, size, data);
}

} // namespace opengl
} // namespace JadeFrame