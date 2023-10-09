#include "pch.h"

#include "opengl_shader.h"
#include "JadeFrame/utils/assert.h"


#include "JadeFrame/math/mat_4.h"

#include "../shader_loader.h"

#include "../graphics_shared.h"
#include "../reflect.h"
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
        case GL_FLOAT: return SHADER_TYPE::F32; break;
        case GL_FLOAT_VEC2: return SHADER_TYPE::V_2_F32; break;
        case GL_FLOAT_VEC3: return SHADER_TYPE::V_3_F32; break;
        case GL_FLOAT_VEC4: return SHADER_TYPE::V_4_F32; break;
        case GL_FLOAT_MAT4: return SHADER_TYPE::M_4_4_F32; break;
        case GL_SAMPLER_2D: return SHADER_TYPE::SAMPLER_2D; break;
        default: assert(false); return {};
    }
}
namespace opengl {

Shader::Shader(OpenGL_Context& context, const Desc& desc)
    : m_program()
    , m_vertex_shader(GL_VERTEX_SHADER)
    , m_fragment_shader(GL_FRAGMENT_SHADER) {
    m_context = &context;


    JF_ASSERT(desc.code.m_modules.size() == 2, "OpenGL Shaders must have 2 modules for right now");



    using SPIRV = std::vector<JadeFrame::u32>;

    std::array<std::string, 2> glsl_sources;
    // from SPIRV to GLSL
    spirv_cross::CompilerGLSL::Options options;
    options.version = 450;
    options.es = false;
    options.vulkan_semantics = true;
    for (size_t i = 0; i < desc.code.m_modules.size(); i++) {
        auto& spirv = desc.code.m_modules[i].m_code;

        spirv_cross::CompilerGLSL glsl(spirv);
        glsl.set_common_options(options);
        glsl_sources[i] = glsl.compile();
    }

    m_vertex_source = glsl_sources[0];
    m_fragment_source = glsl_sources[1];

    m_vertex_shader.set_source(m_vertex_source);
    m_vertex_shader.compile();

    m_fragment_shader.set_source(m_fragment_source);
    m_fragment_shader.compile();


    // // NOTE: On some machines the drives won't allow it!!
    // m_fragment_shader.set_binary(mfrag_shader_spirv);
    // m_fragment_shader.compile_binary();

    // m_vertex_shader.set_binary(mvert_shader_spirv);
    // m_vertex_shader.compile_binary();


    // ShadingCode code;
    // code.m_modules.resize(2);
    // code.m_modules[0].m_stage = desc.code.m_modules[0].m_stage;
    // code.m_modules[0].m_code = s0.get();
    // code.m_modules[1].m_stage = desc.code.m_modules[1].m_stage;
    // code.m_modules[1].m_code = s1.get();

    // auto s = reflect(code);

    // std::vector<u32> vs =
    //     string_to_SPIRV(vertex_shader.c_str(), desc.code.m_modules[0].m_stage, GRAPHICS_API::OPENGL);

    // spirv_cross::CompilerGLSL glsl(spirvs[0]);
    // glsl.set_common_options(options);
    // Logger::trace("{}", std::get<std::string>(desc.code.m_modules[0].m_code));
    // Logger::trace("====================");
    // Logger::trace("{}", glsl_sources[0]);
    // Logger::trace("====================");
    // spirv_cross::CompilerHLSL hlsl(spirvs[0]);
    // Logger::trace("{}", hlsl.compile());

    auto ref = reflect(desc.code);
    for (size_t i = 0; i < ref.m_modules[0].m_inputs.size(); i++) {
        auto& input = ref.m_modules[0].m_inputs[i];

        Shader::VertexAttribute attribs;
        attribs.name = input.name;
        attribs.type = input.type;
        attribs.location = input.location;
        attribs.size = input.size;
        m_vertex_attributes.push_back(attribs);
    }

    for (size_t i = 0; i < ref.m_modules.size(); i++) {
        auto& module = ref.m_modules[i];
        for (size_t j = 0; j < module.m_uniform_buffers.size(); j++) {
            auto& uniform_buffer = module.m_uniform_buffers[j];

            Shader::Uniform uniform;
            uniform.name = uniform_buffer.name;
            // uniform.type = uniform_buffer.type;
            uniform.size = uniform_buffer.size;
            uniform.location = uniform_buffer.binding;
        }
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
}

auto Shader::bind() const -> void { m_program.bind(); }
auto Shader::unbind() const -> void { m_program.unbind(); }


static auto gl_type_enum_to_string(GLenum type) -> std::string {
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
} // namespace opengl
} // namespace JadeFrame