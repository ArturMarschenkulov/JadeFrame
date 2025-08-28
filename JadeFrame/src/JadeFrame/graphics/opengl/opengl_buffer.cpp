#include "opengl_buffer.h"

#include <utility>

#include <glad/glad.h>

#include "opengl_context.h"

namespace JadeFrame {
namespace opengl {

/*---------------------------
    Buffer
---------------------------*/

Buffer::~Buffer() { this->destroy(); }

Buffer::Buffer(Buffer&& other) noexcept
    : m_context(std::exchange(other.m_context, nullptr))
    , m_type(other.m_type)
    , m_size(std::exchange(other.m_size, 0))
    , m_id(std::exchange(other.m_id, 0)) {}

auto Buffer::operator=(Buffer&& other) noexcept -> Buffer& {
    if (this == &other) { return *this; }
    if (m_id != 0) { this->destroy(); }
    m_id = std::exchange(other.m_id, 0);
    m_type = other.m_type;
    m_context = std::exchange(other.m_context, nullptr);
    m_size = std::exchange(other.m_size, 0);
    return *this;
}

auto Buffer::destroy() -> void {
    // NOTE(artur): This is probably a pointless check, as `m_id==0` already leads to a
    // noop, but just in case.
    if (m_id == 0) { return; }
    glDeleteBuffers(1, &m_id);
    m_id = 0;
}

auto Buffer::create(opengl::OpenGL_Context& context, TYPE type, const void* data, GLuint size)
    -> Buffer {

    Buffer buffer = Buffer(context, type, data, size);
    return buffer;
}

Buffer::Buffer(opengl::OpenGL_Context& context, TYPE type, const void* data, GLuint size)
    : m_context(&context)

    , m_type(type)
    , m_size(size)
    , m_id(0) {

    glCreateBuffers(1, &m_id);

    this->alloc(data, size);

    // TODO(artur): Move this registering into OpenGL_Context?
    m_context->m_buffers.push_back(m_id);
    m_context->m_bound_buffer = m_id;
}

auto Buffer::reserve(GLuint size) const -> void {
    if (size > m_size) { this->alloc(nullptr, size); }
}

auto Buffer::alloc(const void* data, GLuint size) const -> void {
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