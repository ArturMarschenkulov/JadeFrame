#include "pch.h"

#include "opengl_shader.h"
#include "opengl_context.h"
#include "JadeFrame/utils/assert.h"

#include "../graphics_shared.h"
#include "../reflect.h"

#include <array>
#include <cassert>

#include "opengl_wrapper.h"

JF_PRAGMA_NO_WARNINGS_PUSH
#undef min
#include "SPIRV-Cross/spirv_glsl.hpp"
JF_PRAGMA_NO_WARNINGS_POP

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

static auto get_vertex_attributes(const ReflectedCode& reflected_code
) -> std::vector<Shader::VertexAttribute> {
    std::vector<Shader::VertexAttribute> result;
    for (size_t i = 0; i < reflected_code.m_modules[0].m_inputs.size(); i++) {
        const auto&             input = reflected_code.m_modules[0].m_inputs[i];
        Shader::VertexAttribute attribs;
        attribs.name = input.name;
        attribs.type = input.type;
        attribs.location = input.location;
        attribs.size = static_cast<i32>(input.size);
        result.push_back(attribs);
    }
    return result;
}

static auto get_uniforms(const ReflectedCode& reflected_code
) -> std::vector<Shader::Uniform> {
    std::vector<Shader::Uniform> result;
    for (size_t i = 0; i < reflected_code.m_modules.size(); i++) {
        const auto& module = reflected_code.m_modules[i];
        for (size_t j = 0; j < module.m_uniform_buffers.size(); j++) {
            const auto& uniform_buffer = module.m_uniform_buffers[j];

            Shader::Uniform uniform;
            uniform.name = uniform_buffer.name;
            uniform.size = static_cast<i32>(uniform_buffer.size);
            uniform.location = uniform_buffer.binding;
            result.push_back(uniform);
        }
    }
    return result;
}

static auto to_opengl_shader_stage(SHADER_STAGE type) -> GLenum {
    switch (type) {
        case SHADER_STAGE::VERTEX: return GL_VERTEX_SHADER;
        case SHADER_STAGE::FRAGMENT: return GL_FRAGMENT_SHADER;
        case SHADER_STAGE::GEOMETRY: return GL_GEOMETRY_SHADER;
        case SHADER_STAGE::COMPUTE: return GL_COMPUTE_SHADER;
        default: assert(false); return 0;
    }
}

static auto get_reflected_modules(const std::vector<OGLW_Shader>& modules
) -> std::vector<ReflectedModule> {
    std::vector<ReflectedModule> reflected_modules;
    reflected_modules.resize(modules.size());
    for (u32 i = 0; i < modules.size(); i++) {
        reflected_modules[i] = modules[i].m_reflected;
    }
    return reflected_modules;
}

Shader::Shader(OpenGL_Context& context, const Desc& desc)
    : m_context(&context) {

    JF_ASSERT(
        desc.code.m_modules.size() == 2,
        "OpenGL Shaders must have 2 modules for right now"
    );

    m_shaders.resize(2);

    for (u32 i = 0; i < desc.code.m_modules.size(); i++) {
        const ShadingCode::Module&        module_ = desc.code.m_modules[i];
        const ShadingCode::Module::SPIRV& spirv = module_.m_code;
        const GLenum&                     type = to_opengl_shader_stage(module_.m_stage);
        m_shaders[i] = OGLW_Shader(type, spirv);
        m_program.attach(m_shaders[i]);
    }
    std::vector<ReflectedModule> reflected_modules = get_reflected_modules(m_shaders);
    m_reflected_interface = ReflectedModule::into_interface(reflected_modules);
    VertexFormat vf = m_reflected_interface.get_vertex_format();
    m_vertex_array = OGLW_VertexArray(&context, vf);

    Logger::warn("OpenGL Shader compiled");

    if (!m_program.link()) {
        std::string info_log = m_program.get_info_log();
        Logger::warn("{}", info_log);
    }

    if (!m_program.validate()) {
        std::string info_log = m_program.get_info_log();
        Logger::warn("{}", info_log);
    }

    for (u32 i = 0; i < m_shaders.size(); i++) { m_program.detach(m_shaders[i]); }

    // Now the shader is ready to be used

    // Here we create the various graphics objects

    // NOTE: The binding points will be somehow abstracted, since the high level will
    // mimick the vulkan model and the vulkan combines sets and bindings points, while
    // opengl has only binding points. That is while vulkan might have something like
    // this (0, 0), (1, 0) and (1, 1), this would have to be mapped to opengl's flat
    // model, something like this 0, 1, 2.
    for (size_t i = 0; i < m_reflected_interface.m_uniform_buffers.size(); i++) {
        const ReflectedModule::UniformBuffer& uniform_buffer =
            m_reflected_interface.m_uniform_buffers[i];
        u32 set = uniform_buffer.set;
        u32 binding = uniform_buffer.binding;
        u32 size = uniform_buffer.size;

        JF_ASSERT(size == sizeof(mat4x4), "Uniform buffer size is not 64 bytes");

        using namespace opengl;

        Buffer* buffer = m_context->create_buffer(Buffer::TYPE::UNIFORM, nullptr, size);
        context.bind_uniform_buffer_to_location(*buffer, binding);
        m_uniform_buffers[binding] = buffer;
    }
}

static auto gl_type_enum_to_string(GLenum type) -> std::string {
    std::string result;
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

auto Shader::write_ub(u32 binding, const void* data, size_t size, size_t offset) -> void {

    auto ub = m_uniform_buffers.find(binding);
    if (ub == m_uniform_buffers.end()) {
        Logger::err(
            "OpenGL::Shader::write_ub: Uniform buffer with binding {} not found", binding
        );
        assert(false);
    }
    opengl::Buffer* buffer = ub->second;
    m_context->bind_uniform_buffer_to_location(*buffer, binding);
    buffer->write(data, size, offset);
}
} // namespace opengl
} // namespace JadeFrame