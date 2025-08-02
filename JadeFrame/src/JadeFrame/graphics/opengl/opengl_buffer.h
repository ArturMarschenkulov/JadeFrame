#pragma once

#include "opengl_wrapper.h"

namespace JadeFrame {
struct Vertex;

class OpenGL_Context;

namespace opengl {

class Buffer {
public:
    Buffer();
    ~Buffer();
    Buffer(const Buffer&) = delete;
    auto operator=(const Buffer&) -> Buffer& = delete;
    Buffer(Buffer&& other) noexcept;
    auto operator=(Buffer&& other) noexcept -> Buffer&;

    enum class TYPE : u8 {
        UNINIT, // TODO: find ways to remove it
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING
    };

    static auto create(OpenGL_Context& context, TYPE type, const void* data, GLuint size)
        -> Buffer;

private:
    Buffer(OpenGL_Context& context, TYPE type, void* data, GLuint size);
    auto alloc(const void* data, GLuint size) const -> void;
    auto reserve(GLuint size) const -> void;

public:
    auto write(const void* data, GLuint size, GLint offset) const -> void;

    template<typename T>
    auto write(const T& data, GLint offset = 0) const -> void {
        this->write(static_cast<const void*>(data), sizeof(T), offset);
    }

public:
    OpenGL_Context* m_context;
    TYPE            m_type;
    size_t          m_size;
    GLuint          m_id;
};

} // namespace opengl
} // namespace JadeFrame