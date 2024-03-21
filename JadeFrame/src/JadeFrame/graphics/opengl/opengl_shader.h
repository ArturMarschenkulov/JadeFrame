#pragma once
#include <glad/glad.h>

#include "opengl_wrapper.h"
#include "../shader_loader.h"

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/vec.h"

#include <vector>
#include <string>
#include <variant>
#include <unordered_map>
#include <functional>
#include <queue>

namespace JadeFrame {
class Matrix4x4;
class OpenGL_Context;

namespace opengl {

class Shader : public IShader {
public:
    Shader() = default;
    ~Shader() = default;

    Shader(const Shader&) = delete;
    auto operator=(const Shader&) -> Shader& = delete;

    Shader(Shader&&) noexcept = delete;
    auto operator=(Shader&&) -> Shader& = delete;

    Shader(OpenGL_Context& context, const Desc& desc);

    auto bind() const -> void;
    auto unbind() const -> void;

private:
    OGLW_Program m_program;
    OGLW_Shader  m_vertex_shader;
    OGLW_Shader  m_fragment_shader;
    std::string  m_vertex_source;
    std::string  m_fragment_source;

    OpenGL_Context* m_context;

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

    std::vector<Uniform>         m_uniforms;
    std::vector<VertexAttribute> m_vertex_attributes;

public:
    std::vector<opengl::Buffer*> m_uniform_buffers;
};
} // namespace opengl
} // namespace JadeFrame