#pragma once
#include <vector>
#include <string>
#include <map>

#include <glad/glad.h>

#include "opengl_wrapper.h"

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
    ~Shader();

    Shader(const Shader&) = delete;
    auto operator=(const Shader&) -> Shader& = delete;

    Shader(Shader&&) noexcept = delete;
    auto operator=(Shader&&) -> Shader& = delete;

    Shader(opengl::Context& context, const Desc& desc);

public:
    bool                     m_is_initialized = false;
    std::string              m_error;
    OGLW_Program             m_program;
    std::vector<OGLW_Shader> m_shaders;

    Context* m_context = nullptr;

public:
    OGLW_VertexArray m_vertex_array;

    ReflectedModule m_reflected_interface;
};

class Material {
public:
    Material() = default;
    ~Material();
    Material(const Material&) = delete;
    auto operator=(const Material&) -> Material& = delete;
    Material(Material&&) noexcept = delete;
    auto operator=(Material&&) -> Material& = delete;

    Material(opengl::Context& context, Shader& shader, Texture* texture);

public:
    auto write_ub(u32 index, const void* data, size_t size, size_t offset) -> void;

public:
    bool     m_is_initialized = false;
    Context* m_context = nullptr;

    std::map<u32, opengl::Buffer*> m_uniform_buffers;
};
} // namespace opengl
} // namespace JadeFrame