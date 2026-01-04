#pragma once

#include "opengl_wrapper.h"

namespace JadeFrame {

class OpenGL_Context;

namespace opengl {

class Buffer {
public:
    Buffer() = delete;
    ~Buffer();
    Buffer(const Buffer&) = delete;
    auto operator=(const Buffer&) -> Buffer& = delete;
    Buffer(Buffer&& other) noexcept;
    auto operator=(Buffer&& other) noexcept -> Buffer&;

    enum class TYPE : u8 {
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING
    };

    static auto create(opengl::Context& context, TYPE type, const void* data, GLuint size)
        -> Buffer;

    auto destroy() -> void;

private:
    Buffer(opengl::Context& context, TYPE type, const void* data, GLuint size);
    auto alloc(const void* data, GLuint size) -> void;
    auto reserve(GLuint size) -> void;

public:
    auto write(const void* data, GLuint size, GLint offset) const -> void;

    template<typename T>
    auto write(const T& data, GLint offset = 0) const -> void {
        this->write(static_cast<const void*>(&data), sizeof(T), offset);
    }

public:
    Context* m_context = nullptr;
    TYPE     m_type;
    // The size of the buffer in bytes
    size_t m_size;
    GLuint m_id;
};

static_assert(std::is_move_constructible<Buffer>());
} // namespace opengl
} // namespace JadeFrame