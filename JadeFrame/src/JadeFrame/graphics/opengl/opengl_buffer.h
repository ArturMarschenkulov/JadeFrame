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
    Buffer();
    ~Buffer();
    Buffer(const Buffer&) = delete;
    auto operator=(const Buffer&) -> Buffer& = delete;
    Buffer(Buffer&& other) noexcept;
    auto operator=(Buffer&& other) -> Buffer&;

    enum class TYPE {
        UNINIT, // TODO: find ways to remove it
        VERTEX,
        INDEX,
        UNIFORM,
        STAGING
    };

    Buffer(OpenGL_Context& context, TYPE type, void* data, GLuint size);

private:
    auto alloc(void* data, GLuint size) const -> void;
    auto reserve(GLuint size) const -> void;

    template<typename U>
    auto alloc(const std::initializer_list<U>& data) const -> void {
        this->alloc((void*)data.begin(), data.size() * sizeof(U));
    }
    template<typename U>
    auto alloc(const std::vector<U>& data) const -> void {
        this->alloc((void*)data.data(), data.size() * sizeof(U));
    }

public:
    auto write(const void* data, GLint offset, GLuint size) const -> void;


    template<typename U>
    auto write(const std::initializer_list<U>& data) const -> void {
        this->write((void*)data.begin(), 0, data.size() * sizeof(U));
    }

    auto bind_base(GLuint binding_point) const -> void;
    auto bind_buffer_range(GLuint index, GLintptr offset, GLsizeiptr size) const -> void;

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
    size_t          m_size;
};

class GPUMeshData {
public:
    // auto operator=(const GPUMeshData& other) -> GPUMeshData&;
    GPUMeshData() = default;
    ~GPUMeshData() = default;

    GPUMeshData(const GPUMeshData&) = delete;
    auto operator=(const GPUMeshData&) -> GPUMeshData& = delete;

    // GPUMeshData(GPUMeshData&& other);
    auto operator=(GPUMeshData&& other) -> GPUMeshData&;

    GPUMeshData(
        OpenGL_Context& context, const VertexData& vertex_data, VertexFormat vertex_format, bool interleaved = true);

public: // private:
    Buffer*      m_vertex_buffer;
    Buffer*      m_index_buffer;
    VertexFormat m_vertex_format;

    OGLW_VertexArray m_vertex_array;
};
} // namespace opengl
} // namespace JadeFrame