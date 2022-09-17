#include "pch.h"
#include "graphics_shared.h"
JF_PRAGMA_PUSH
#pragma warning(disable : 4006)
#include "shaderc/shaderc.hpp"
JF_PRAGMA_POP
namespace JadeFrame {

VertexAttribute::VertexAttribute(const std::string& name, SHADER_TYPE type, bool normalized)
    : name(name)
    , type(type)
    , size(SHADER_TYPE_get_size(type))
    , offset(0)
    , normalized(normalized) {}

VertexFormat::VertexFormat(const std::initializer_list<VertexAttribute>& attributes)
    : m_attributes(attributes) {
    this->calculate_offset_and_stride(m_attributes);
}
auto VertexFormat::calculate_offset_and_stride(std::vector<VertexAttribute>& attributes) -> void {
    size_t offset = 0;
    m_stride = 0;
    for (VertexAttribute& attribute : attributes) {
        attribute.offset = offset;
        offset += attribute.size;
        m_stride += attribute.size;
    }
}



auto string_to_SPIRV(const std::string& code, SHADER_STAGE stage) -> std::vector<u32> {
#if 1
    namespace shc = shaderc;
    // using namespace shaderc;

    shaderc_shader_kind kind = {};
    switch (stage) {
        case SHADER_STAGE::VERTEX: {
            kind = shaderc_vertex_shader;
        } break;
        case SHADER_STAGE::FRAGMENT: {
            kind = shaderc_fragment_shader;
        } break;
        default: assert(false);
    }

    shc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
    // options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
    options.SetWarningsAsErrors();
    options.SetGenerateDebugInfo();
    const bool optimize = false;
    if constexpr (optimize == true) { options.SetOptimizationLevel(shaderc_optimization_level_size); }
    shc::Compiler              compiler;
    shc::SpvCompilationResult  comp_result = compiler.CompileGlslToSpv(code, kind, "", options);
    shaderc_compilation_status comp_status = comp_result.GetCompilationStatus();
    if (comp_status != shaderc_compilation_status_success) {
        assert(false);
        return std::vector<u32>();
    }

    std::vector<u32> result = {comp_result.cbegin(), comp_result.cend()};
    return result;
#endif
    return {};
}


class RenderCommandQueue {
public:
    typedef void (*RenderCommandFn)(void*);
    RenderCommandQueue() {
        const auto buffer_size = 10 * 1024 * 1024;
        m_command_buffer = new u8[buffer_size];
        m_command_buffer_ptr = m_command_buffer;
        std::memset(m_command_buffer, 0, buffer_size);
    }
    ~RenderCommandQueue() { delete[] m_command_buffer; }
    auto allocate(RenderCommandFn func, u32 size) {
        // TODO: alignment
        *(RenderCommandFn*)m_command_buffer_ptr = func;
        m_command_buffer_ptr += sizeof(RenderCommandFn);

        *(u32*)m_command_buffer_ptr = size;
        m_command_buffer_ptr += sizeof(u32);

        void* memory = m_command_buffer_ptr;
        m_command_buffer_ptr += size;

        m_command_count++;
        return memory;
    }
    auto execute() -> void {

        u8* buffer = m_command_buffer;

        for (uint32_t i = 0; i < m_command_count; i++) {
            RenderCommandFn function = *(RenderCommandFn*)buffer;
            buffer += sizeof(RenderCommandFn);

            u32 size = *(u32*)buffer;
            buffer += sizeof(u32);
            function(buffer);
            buffer += size;
        }

        m_command_buffer_ptr = m_command_buffer;
        m_command_count = 0;
    }

private:
    u8* m_command_buffer;
    u8* m_command_buffer_ptr;
    u32 m_command_count = 0;
};
} // namespace JadeFrame