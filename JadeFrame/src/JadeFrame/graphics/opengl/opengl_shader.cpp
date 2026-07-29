#include <cassert>

#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/logger.h"
#include "../graphics_shared.h"
#include "../reflect.h"

#include "opengl_shader.h"
#include "opengl_context.h"
#include "opengl_wrapper.h"

JF_PRAGMA_NO_WARNINGS_PUSH
#undef min
#include "SPIRV-Cross/spirv_glsl.hpp"
JF_PRAGMA_NO_WARNINGS_POP

namespace JadeFrame {

namespace opengl {

static auto to_opengl(SHADER_STAGE type) -> GLenum {
    switch (type) {
        case SHADER_STAGE::VERTEX: return GL_VERTEX_SHADER;
        case SHADER_STAGE::FRAGMENT: return GL_FRAGMENT_SHADER;
        case SHADER_STAGE::GEOMETRY: return GL_GEOMETRY_SHADER;
        case SHADER_STAGE::COMPUTE: return GL_COMPUTE_SHADER;
        default: assert(false); return 0;
    }
}

static auto get_reflected_modules(const std::vector<OGLW_Shader>& modules)
    -> std::vector<ReflectedModule> {
    std::vector<ReflectedModule> reflected_modules;
    reflected_modules.resize(modules.size());
    for (u32 i = 0; i < modules.size(); i++) {
        reflected_modules[i] = modules[i].m_reflected;
    }
    return reflected_modules;
}

Shader::Shader(opengl::Context& context, const Desc& desc)
    : m_context(&context) {

    JF_ASSERT(
        desc.code.m_modules.size() == 2,
        "OpenGL Shaders must have 2 modules for right now"
    );

    m_shaders.resize(2);

    for (u32 i = 0; i < desc.code.m_modules.size(); i++) {
        const ShadingCode::Module&        module_ = desc.code.m_modules[i];
        const ShadingCode::Module::SPIRV& spirv = module_.m_code;
        const GLenum&                     type = to_opengl(module_.m_stage);
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
    Logger::debug("OpenGL Shader successfully compiled and linked");
    // Now the shader is ready to be used

    // Here we create the various graphics objects
}

Material::Material(opengl::Context& context, Shader& shader, Texture* texture)
    : m_context(&context) {
    (void)texture;

    // NOTE: The binding points will be somehow abstracted, since the high level will
    // mimick the vulkan model and the vulkan combines sets and bindings points, while
    // opengl has only binding points. That is while vulkan might have something like
    // this (0, 0), (1, 0) and (1, 1), this would have to be mapped to opengl's flat
    // model, something like this 0, 1, 2.
    for (size_t i = 0; i < shader.m_reflected_interface.m_uniform_buffers.size(); i++) {
        const ReflectedModule::UniformBuffer& uniform_buffer =
            shader.m_reflected_interface.m_uniform_buffers[i];
        u32 binding = uniform_buffer.binding;
        u32 size = uniform_buffer.size;

        JF_ASSERT(size == sizeof(mat4x4), "Uniform buffer size is not 64 bytes");

        using namespace opengl;

        Buffer* buffer = m_context->create_buffer(Buffer::TYPE::UNIFORM, nullptr, size);
        context.bind_uniform_buffer_to_location(*buffer, binding);
        m_uniform_buffers[binding] = buffer;
    }
}

auto Material::write_ub(u32 binding, const void* data, size_t size, size_t offset)
    -> void {

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
