#include "pch.h"

#include "opengl_shader.h"


#include "JadeFrame/math/mat_4.h"

#include "../shader_loader.h"

#include "../graphics_shared.h"
#include <future>

#include <array>
#include <cassert>
#include <tuple>

#include "opengl_wrapper.h"

#include "SPIRV-Cross/spirv_glsl.hpp"
#include "SPIRV-Cross/spirv_hlsl.hpp"
#include "SPIRV-Cross/spirv_msl.hpp"

namespace JadeFrame {
static auto SHADER_TYPE_from_openGL_enum(const GLenum type) -> SHADER_TYPE {
    switch (type) {
        case GL_FLOAT: return SHADER_TYPE::FLOAT; break;
        case GL_FLOAT_VEC2: return SHADER_TYPE::FLOAT_2; break;
        case GL_FLOAT_VEC3: return SHADER_TYPE::FLOAT_3; break;
        case GL_FLOAT_VEC4: return SHADER_TYPE::FLOAT_4; break;
        case GL_FLOAT_MAT4: return SHADER_TYPE::MAT_4; break;
        case GL_SAMPLER_2D: return SHADER_TYPE::SAMPLER_2D; break;
        default: assert(false); return {};
    }
}
// static auto check_glsl_variables(const std::unordered_map<std::string, OpenGL_Shader::GL_Variable>& ) -> void {
//
// }
namespace opengl {

Shader::Shader(const DESC& desc)
    : m_program()
    , m_vertex_shader(GL_VERTEX_SHADER)
    , m_fragment_shader(GL_FRAGMENT_SHADER) {

    auto vertex_shader = std::get<std::string>(desc.code.m_modules[0].m_code);
    auto fragment_shader = std::get<std::string>(desc.code.m_modules[1].m_code);
    m_vertex_source = vertex_shader;
    m_fragment_source = fragment_shader;
    // Logger::trace("vertex shader: {}", vertex_shader);
    // Logger::trace("fragment shader: {}", fragment_shader);
    if constexpr (false) {

        /*
            NOTE: Strangely, on the laptops I tried it out, this function `glSpecializeShader` doesn't work.
            I'm not sure why, but it seems to be a driver issue.
            I'm leaving this here in case I ever get around to fixing it.
        */
        std::future<std::vector<u32>> vert_shader_spirv = std::async(
            std::launch::async, string_to_SPIRV, m_vertex_source.c_str(), desc.code.m_modules[0].m_stage,
            GRAPHICS_API::OPENGL);
        std::future<std::vector<u32>> frag_shader_spirv = std::async(
            std::launch::async, string_to_SPIRV, m_fragment_source.c_str(), desc.code.m_modules[1].m_stage,
            GRAPHICS_API::OPENGL);

        std::vector<u32> mvert_shader_spirv = vert_shader_spirv.get();
        std::vector<u32> mfrag_shader_spirv = frag_shader_spirv.get();


        // NOTE: On some machines the drives won't allow it!!
        m_fragment_shader.set_binary(mfrag_shader_spirv);
        m_fragment_shader.compile_binary();

        m_vertex_shader.set_binary(mvert_shader_spirv);
        m_vertex_shader.compile_binary();

        //__debugbreak();
    } else {
        m_vertex_shader.set_source(vertex_shader);
        m_vertex_shader.compile();

        m_fragment_shader.set_source(fragment_shader);
        m_fragment_shader.compile();

        auto s0 = std::async(
            std::launch::async, string_to_SPIRV, m_vertex_source.c_str(), desc.code.m_modules[0].m_stage,
            GRAPHICS_API::OPENGL);
        auto s1 = std::async(
            std::launch::async, string_to_SPIRV, m_fragment_source.c_str(), desc.code.m_modules[1].m_stage,
            GRAPHICS_API::OPENGL);

        ShadingCode code;
        code.m_modules.resize(2);
        code.m_modules[0].m_stage = desc.code.m_modules[0].m_stage;
        code.m_modules[0].m_code = s0.get();
        code.m_modules[1].m_stage = desc.code.m_modules[1].m_stage;
        code.m_modules[1].m_code = s1.get();

        auto s = reflect(code);

        // std::vector<u32> vs = string_to_SPIRV(vertex_shader.c_str(), desc.code.m_modules[0].m_stage);

        // spirv_cross::CompilerGLSL::Options options;
        // options.version = 450;
        // options.es = false;
        // options.vulkan_semantics = true;
        // spirv_cross::CompilerGLSL glsl(vs);
        // glsl.set_common_options(options);
        // Logger::trace("{}", glsl.compile());
        // Logger::trace("====================");
        // spirv_cross::CompilerHLSL hlsl(vs);
        // Logger::trace("{}", hlsl.compile());
    }

    m_program.attach(m_vertex_shader);
    m_program.attach(m_fragment_shader);

    const bool is_linked = m_program.link();
    if (is_linked == false) {
        std::string info_log = m_program.get_info_log();
        Logger::log("{}", info_log);
    }

    const bool is_validated = m_program.validate();
    if (is_validated == false) {
        std::string info_log = m_program.get_info_log();
        Logger::log("{}", info_log);
    }

    m_program.detach(m_vertex_shader);
    m_program.detach(m_fragment_shader);




    m_uniforms = this->query_uniforms(GL_ACTIVE_UNIFORMS);
    m_attributes = this->query_uniforms(GL_ACTIVE_ATTRIBUTES);
}

auto Shader::bind() const -> void { m_program.bind(); }
auto Shader::unbind() const -> void { m_program.unbind(); }

auto Shader::get_uniform_location(const std::string& name) const -> GLint {
    if (m_uniforms.contains(name)) { return m_uniforms.at(name).location; }
    assert(false);
    return -1;
}


static auto to_string_gl_type(GLenum type) -> std::string {
    std::string result = "";
    switch (type) {
        case GL_FLOAT: result = "float"; break;
        case GL_FLOAT_VEC2: result = "vec2"; break;
        case GL_FLOAT_VEC3: result = "vec3"; break;
        case GL_FLOAT_VEC4: result = "vec4"; break;
        case GL_INT: result = "int"; break;
        case GL_INT_VEC2: result = "ivec2"; break;
        case GL_INT_VEC3: result = "ivec3"; break;
        case GL_INT_VEC4: result = "ivec4"; break;
        case GL_UNSIGNED_INT: result = "uint"; break;
        case GL_UNSIGNED_INT_VEC2: result = "uvec2"; break;
        case GL_UNSIGNED_INT_VEC3: result = "uvec3"; break;
        case GL_UNSIGNED_INT_VEC4: result = "uvec4"; break;
        case GL_BOOL: result = "bool"; break;
        case GL_BOOL_VEC2: result = "bvec2"; break;
        case GL_BOOL_VEC3: result = "bvec3"; break;
        case GL_BOOL_VEC4: result = "bvec4"; break;
        case GL_FLOAT_MAT2: result = "mat2"; break;
        case GL_FLOAT_MAT3: result = "mat3"; break;
        case GL_FLOAT_MAT4: result = "mat4"; break;
        case GL_FLOAT_MAT2x3: result = "mat2x3"; break;
        case GL_FLOAT_MAT2x4: result = "mat2x4"; break;
        case GL_FLOAT_MAT3x2: result = "mat3x2"; break;
        case GL_FLOAT_MAT3x4: result = "mat3x4"; break;
        case GL_FLOAT_MAT4x2: result = "mat4x2"; break;
        case GL_FLOAT_MAT4x3: result = "mat4x3"; break;
        case GL_SAMPLER_1D: result = "sampler1D"; break;
        case GL_SAMPLER_2D: result = "sampler2D"; break;
        case GL_SAMPLER_3D: result = "sampler3D"; break;
        case GL_SAMPLER_CUBE: result = "samplerCube"; break;
        case GL_SAMPLER_1D_SHADOW: result = "sampler1DShadow"; break;
        case GL_SAMPLER_2D_SHADOW: result = "sampler2DShadow"; break;
        case GL_SAMPLER_1D_ARRAY: result = "sampler1DArray"; break;
        case GL_SAMPLER_2D_ARRAY: result = "sampler2DArray"; break;
        case GL_SAMPLER_1D_ARRAY_SHADOW: result = "sampler1DArrayShadow"; break;
        case GL_SAMPLER_2D_ARRAY_SHADOW: result = "sampler2DArray"; break;
        case GL_SAMPLER_2D_MULTISAMPLE: result = "sampler2DMS"; break;
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: result = "sampler2DMSArray"; break;
        default: Logger::err("Unknown type {}", type); assert(false);
    }
    return result;
}

auto Shader::query_uniforms(const GLenum variable_type) const -> std::unordered_map<std::string, GL_Variable> {
    // variable_type = GL_ACTIVE_UNIFORMS | GL_ACTIVE_ATTRIBUTES

    GLint                                        num_variables = m_program.get_info(variable_type);
    std::vector<GL_Variable>                     variables(num_variables);
    std::unordered_map<std::string, GL_Variable> variable_map;

    for (i32 i = 0; i < num_variables; ++i) {
        char   buffer[128];
        GLenum gl_type;
        switch (variable_type) {
            case GL_ACTIVE_UNIFORMS: {
                glGetActiveUniform(m_program.m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
                auto s = to_string_gl_type(gl_type);
                GLint location = m_program.get_uniform_location(buffer);
                if (location == -1) {

                } else {
                    variables[i].location = location;
                }
            } break;
            case GL_ACTIVE_ATTRIBUTES: {
                glGetActiveAttrib(m_program.m_ID, i, sizeof(buffer), 0, &variables[i].size, &gl_type, buffer);
                variables[i].location = m_program.get_attribute_location(buffer);
            } break;
            default:
                assert(false);
                gl_type = -1;
                break;
        }
        variables[i].name = std::string(buffer);
        variables[i].type = SHADER_TYPE_from_openGL_enum(gl_type);

        { // TODO: It initializes the types for the variant type, for error checking. Consider whether this is
          // neccessary.
            GL_ValueVariant value_init;
            switch (variables[i].type) {
                case SHADER_TYPE::SAMPLER_2D: value_init = i32(); break;
                case SHADER_TYPE::FLOAT: value_init = f32(); break;
                case SHADER_TYPE::FLOAT_2: value_init = v2(); break;
                case SHADER_TYPE::FLOAT_3: value_init = v3(); break;
                case SHADER_TYPE::FLOAT_4: value_init = v4(); break;
                case SHADER_TYPE::MAT_4: value_init = Matrix4x4(); break;
                default: assert(false); break;
            }
            variables[i].value = value_init;
        }

        variable_map[variables[i].name] = variables[i];
    }
    return variable_map;
}
} // namespace opengl
} // namespace JadeFrame