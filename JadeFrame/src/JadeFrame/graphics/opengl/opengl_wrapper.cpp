#include "pch.h"
#include "opengl_wrapper.h"
#include "opengl_buffer.h"
#include "../graphics_shared.h"
#include "../graphics_language.h"
#include "opengl_context.h"

#include "JadeFrame/graphics/reflect.h"

#define GL_SHADER_BINARY_FORMAT_SPIR_V_ARB 0x9551

namespace JadeFrame {
static auto to_gl_type(const SHADER_TYPE type) -> GLenum {
    GLenum result;
    switch (type) {
        case SHADER_TYPE::F32:
        case SHADER_TYPE::V_2_F32:
        case SHADER_TYPE::V_3_F32:
        case SHADER_TYPE::V_4_F32: result = GL_FLOAT; break;
        default:
            assert(false);
            result = 0;
            break;
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

static auto gl_type_to_jf_type(GLenum type) -> SHADER_STAGE {
    switch (type) {
        case GL_VERTEX_SHADER: return SHADER_STAGE::VERTEX;
        case GL_FRAGMENT_SHADER: return SHADER_STAGE::FRAGMENT;
        case GL_GEOMETRY_SHADER: return SHADER_STAGE::GEOMETRY;
        case GL_COMPUTE_SHADER: return SHADER_STAGE::COMPUTE;
        default: assert(false); return SHADER_STAGE::VERTEX;
    }
}

OGLW_VertexArray::OGLW_VertexArray(
    OpenGL_Context*     context,
    const VertexFormat& vertex_format
) {
    (void)context;
    glCreateVertexArrays(1, &m_ID);
    this->set_layout(vertex_format);
}

OGLW_VertexArray::~OGLW_VertexArray() { glDeleteVertexArrays(1, &m_ID); }

OGLW_VertexArray::OGLW_VertexArray(OGLW_VertexArray&& other) noexcept
    : m_ID(std::exchange(other.m_ID, 0))
    , m_vertex_format(std::exchange(other.m_vertex_format, {})) {}

auto OGLW_VertexArray::operator=(OGLW_VertexArray&& other) noexcept -> OGLW_VertexArray& {
    m_ID = std::exchange(other.m_ID, 0);
    m_vertex_format = std::exchange(other.m_vertex_format, {});
    return *this;
}

auto OGLW_VertexArray::bind_buffer(const opengl::Buffer& buffer) const -> void {
    // Check whether this vao is bound

    glVertexArrayVertexBuffer(
        m_ID, 0, buffer.m_id, 0, static_cast<GLsizei>(m_vertex_format.m_stride)
    );
}

auto OGLW_VertexArray::set_layout(const VertexFormat& vertex_format) -> void {
    m_vertex_format = vertex_format;

    size_t offset = 0;
    for (u32 i = 0; i != vertex_format.m_attributes.size(); i++) {
        const VertexAttribute& attribute = vertex_format.m_attributes[i];

        switch (attribute.type) {
            case SHADER_TYPE::F32:
            case SHADER_TYPE::V_2_F32:
            case SHADER_TYPE::V_3_F32:
            case SHADER_TYPE::V_4_F32: {

            } break;
            default: {
                assert(false && "Unsupported type");
            }
        }
        this->enable_attrib(i);
        // always unnormalized
        const bool normalized = false;
        this->set_attrib_format(i, attribute.type, normalized, offset);
        offset += get_size(attribute.type);
        this->set_attrib_binding(i, 0);
    }
}

auto OGLW_VertexArray::enable_attrib(const u32 index) const -> void {
    glEnableVertexArrayAttrib(m_ID, index);
}

auto OGLW_VertexArray::set_attrib_format(
    const u32         index,
    const SHADER_TYPE type,
    const bool        normalized,
    const size_t      offset
) const -> void {
    const u32    count = get_component_count(type);
    const GLenum gl_type = to_gl_type(type);
    glVertexArrayAttribFormat(
        m_ID, index, count, gl_type, normalized ? GL_TRUE : GL_FALSE, offset
    );
}

auto OGLW_VertexArray::set_attrib_binding(const u32 index, const u32 binding) const
    -> void {
    glVertexArrayAttribBinding(m_ID, index, binding);
}

OGLW_Shader::OGLW_Shader(const GLenum type, const std::vector<u32>& binary)
    : m_ID(glCreateShader(type))
    , m_reflected(ReflectedModule::reflect(binary, gl_type_to_jf_type(type))) {
#define JF_USE_SPIRV false
    if constexpr (JF_USE_SPIRV) {
        this->set_binary(binary);
        this->compile_binary();
    } else {
        auto s = convert_SPIRV_to_GLSL(binary);
        this->set_source(s);
        this->compile();
        
        auto success = this->get_info(GL_COMPILE_STATUS);
        switch (type) {
            case GL_VERTEX_SHADER: {
                success == GL_FALSE
                    ? Logger::warn("Vertex Shader {} failed to compile", m_ID)
                    : Logger::info("Vertex Shader {} compiled successfully", m_ID);
            }; break;
            case GL_FRAGMENT_SHADER: {
                success == GL_FALSE
                    ? Logger::warn("Fragment Shader {} failed to compile", m_ID)
                    : Logger::info("Fragment Shader {} compiled successfully", m_ID);
            }; break;
            case GL_COMPUTE_SHADER: {
                success == GL_FALSE
                    ? Logger::warn("Compute Shader {} failed to compile", m_ID)
                    : Logger::info("Compute Shader {} compiled successfully", m_ID);
            }; break;
            default: {
                Logger::err("Invalid Shader type {}", type);
            }
        }
    }
#undef JF_USE_SPIRV
}

OGLW_Shader::OGLW_Shader(OGLW_Shader&& other) noexcept
    : m_ID(std::exchange(other.m_ID, 0))
    , m_reflected(std::exchange(other.m_reflected, {})) {}

auto OGLW_Shader::operator=(OGLW_Shader&& other) noexcept -> OGLW_Shader& {
    m_ID = std::exchange(other.m_ID, 0);
    m_reflected = std::exchange(other.m_reflected, {});
    return *this;
}

OGLW_Shader::~OGLW_Shader() {
    glDeleteShader(m_ID);
    m_ID = 0;
}

auto OGLW_Shader::set_source(const std::string& source_code) -> void {
    const GLchar* source = source_code.c_str();
    glShaderSource(m_ID, 1, &source, nullptr);
}

auto OGLW_Shader::compile() -> void { glCompileShader(m_ID); }

auto OGLW_Shader::set_binary(const std::vector<u32>& binary) -> void {
    glShaderBinary(
        1, &m_ID, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, binary.data(), binary.size()
    );
}

auto OGLW_Shader::compile_binary() -> void {
    // assert(false);
    /*
        NOTE: Strangely, on the laptops I tried it out, this function doesn't work.
        I'm not sure why, but it seems to be a driver issue.
        I'm leaving this here in case I ever get around to fixing it.
    */
    glSpecializeShader(m_ID, "main", 0, nullptr, nullptr);
}

auto OGLW_Shader::get_info(GLenum pname) -> GLint {
    GLint result;
    glGetShaderiv(m_ID, pname, &result);
    return result;
}

auto OGLW_Shader::get_compile_status() -> GLint {
    GLint is_compiled = GL_FALSE;
    glGetShaderiv(m_ID, GL_COMPILE_STATUS, &is_compiled);
    return is_compiled;
}

auto OGLW_Shader::get_info_log() -> std::string {
    const u32 N = 1024;

    GLchar info_log[N];
    i32    returned_length = 0;
    glGetShaderInfoLog(m_ID, N, &returned_length, &info_log[0]);
    if (returned_length > N) {
        Logger::warn("Info log for shader {} was truncated", m_ID);
    }
    std::string result(info_log);
    return result;
}

OGLW_Program::OGLW_Program()
    : m_ID(glCreateProgram()) {}

OGLW_Program::~OGLW_Program() {
    glDeleteProgram(m_ID);
    m_ID = 0;
}

auto OGLW_Program::bind() const -> void {
    assert(m_ID != 0);
    glUseProgram(m_ID);
}

auto OGLW_Program::unbind() const -> void { glUseProgram(0); }

auto OGLW_Program::attach(const OGLW_Shader& shader) const -> void {
    glAttachShader(m_ID, shader.m_ID);
}

auto OGLW_Program::link() const -> bool {
    glLinkProgram(m_ID);

    GLint is_linked = GL_FALSE;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &is_linked);
    return is_linked == GL_TRUE;
}

auto OGLW_Program::detach(const OGLW_Shader& shader) const -> void {
    glDetachShader(m_ID, shader.m_ID);
}

auto OGLW_Program::validate() const -> bool {
    glValidateProgram(m_ID);

    GLint is_validated = GL_FALSE;
    glGetProgramiv(m_ID, GL_VALIDATE_STATUS, (i32*)&is_validated);
    return is_validated == GL_TRUE;
}

auto OGLW_Program::get_uniform_block_index(const char* name) const -> GLuint {
    return glGetUniformBlockIndex(m_ID, name);
}

auto OGLW_Program::set_uniform_block_binding(GLuint index, GLuint binding_point) const
    -> void {
    glUniformBlockBinding(m_ID, index, binding_point);
}

auto OGLW_Program::get_info(GLenum pname) const -> GLint {
    // GL_DELETE_STATUS, GL_LINK_STATUS, GL_VALIDATE_STATUS, GL_INFO_LOG_LENGTH,
    // GL_ATTACHED_SHADERS, GL_ACTIVE_ATTRIBUTES, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
    // GL_ACTIVE_UNIFORMS, GL_ACTIVE_UNIFORM_MAX_LENGTH.
    GLint result = {};
    glGetProgramiv(m_ID, pname, &result);
    return result;
}

auto OGLW_Program::get_info_log() const -> std::string {
    const u32 N = 1024;

    GLchar info_log[N];
    i32    returned_length = 0;
    glGetProgramInfoLog(m_ID, N, &returned_length, &info_log[0]);
    if (returned_length > N) {
        Logger::warn("Info log for program {} was truncated", m_ID);
    }

    std::string result(info_log);
    return result;
}

} // namespace JadeFrame