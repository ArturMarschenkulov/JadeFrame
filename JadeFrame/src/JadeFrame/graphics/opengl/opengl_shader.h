#pragma once
#include <glad/glad.h>

#include "opengl_wrapper.h"

#include <vector>
#include <string>

namespace JadeFrame {
class OpenGL_Context;

namespace opengl {

class Shader : public IShader {
public:
    struct Uniform {
        SHADER_TYPE type;
        std::string name;
        i32         size;
        u32         location;
    };

    struct VertexAttribute {
        SHADER_TYPE type;
        std::string name;
        i32         size;
        u32         location;
    };

    Shader() = default;
    ~Shader() = default;

    Shader(const Shader&) = delete;
    auto operator=(const Shader&) -> Shader& = delete;

    Shader(Shader&&) noexcept = delete;
    auto operator=(Shader&&) -> Shader& = delete;

    Shader(OpenGL_Context& context, const Desc& desc);

    auto write_ub(u32 index, const void* data, size_t size, size_t offset) -> void;

public:
    OGLW_Program             m_program;
    std::vector<OGLW_Shader> m_shaders;

    OpenGL_Context* m_context;

public:
    std::vector<opengl::Buffer*> m_uniform_buffers;

    OGLW_VertexArray m_vertex_array;

    ReflectedModule m_reflected_interface;
};
} // namespace opengl
} // namespace JadeFrame