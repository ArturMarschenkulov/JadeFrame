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

class OpenGL_Context;
namespace opengl {

class Buffer {
public:
    enum class TYPE {
        UNINIT, // TODO: find ways to remove it
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING
    };


    Buffer(const Buffer&) = delete;
    auto operator=(const Buffer&) -> Buffer& = delete;
    auto operator=(Buffer&&) -> Buffer& = delete;

    Buffer() {
        m_id = 0;
        m_type = TYPE::UNINIT;
        m_context = nullptr;
    }
    Buffer(Buffer&& other) noexcept
        : m_id(other.release()) {

        // m_type = other.m_type;
        // m_context = other.m_context;
        // other.m_context = nullptr;
    }
    ~Buffer() { this->reset(); }
    auto init(OpenGL_Context& context, TYPE type) -> void;

    auto reserve(GLuint size_in_bytes) const -> void {
        // if NULL is passed in as data, it only reserves size_in_bytes bytes.
        // glBufferData(buffer_type, size_in_bytes, NULL, GL_STATIC_DRAW);
        glNamedBufferData(m_id, size_in_bytes, NULL, GL_STATIC_DRAW);
    }
    template<typename U>
    auto send(const std::initializer_list<U>& data) const -> void {
        // glBufferData(buffer_type, data.size() * sizeof(U), data.data(), GL_STATIC_DRAW);
        glNamedBufferData(m_id, data.size() * sizeof(U), data.begin(), GL_STATIC_DRAW);
    }
    template<typename U>
    auto send(const std::vector<U>& data) const -> void {
        // glBufferData(buffer_type, data.size() * sizeof(U), data.data(), GL_STATIC_DRAW);
        glNamedBufferData(m_id, data.size() * sizeof(U), data.data(), GL_STATIC_DRAW);
    }
    auto update(GLuint size_in_bytes, const void* data) const -> void {
        // glBufferSubData(buffer_type, 0, size_in_bytes, data);
        glNamedBufferSubData(m_id, 0, size_in_bytes, data);
    }
    auto bind_base(GLuint binding_point) const -> void {
        if (m_type == TYPE::UNIFORM) {
            glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_id);
        } else {
            Logger::err("Buffer::bind_base() called on non-uniform buffer, only works with uniform buffers");
            assert(false);
        }
    }

    auto bind_buffer_range(GLuint index, GLintptr offset, GLsizeiptr size) const -> void {
        if (m_type == TYPE::UNIFORM) {
            glBindBufferRange(GL_UNIFORM_BUFFER, index, m_id, offset, size);
        } else {
            Logger::err("Buffer::bind_buffer_range() called on non-uniform buffer, only works with uniform buffers");
            assert(false);
        }
    }

private:
    auto release() -> GLuint {
        auto id = m_id;
        m_id = 0;
        return id;
    }
    auto reset(GLuint id = 0) -> void {
        glDeleteBuffers(1, &m_id);
        m_id = id;
    }

public:
    GLuint          m_id;
    TYPE            m_type;
    OpenGL_Context* m_context;
};

class GPUMeshData {
public:
    GPUMeshData(
        OpenGL_Context& context, const VertexData& vertex_data, VertexFormat vertex_format, bool interleaved = true);
    auto bind() const -> void;
    auto set_layout(const VertexFormat& vertex_format) -> void;

public: // private:
    Buffer           m_vertex_buffer;
    OGLW_VertexArray m_vertex_array;
    Buffer           m_index_buffer;
    VertexFormat     m_vertex_format;
};
} // namespace opengl
} // namespace JadeFrame