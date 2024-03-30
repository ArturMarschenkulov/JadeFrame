#include "pch.h"
#include "opengl_wrapper.h"
#include "opengl_buffer.h"
#include "../graphics_shared.h"
#include "opengl_context.h"

namespace JadeFrame {

OGLW_VertexArray::OGLW_VertexArray() {
    m_ID = 0;
    m_vertex_format = {};
}

OGLW_VertexArray::~OGLW_VertexArray() { glDeleteVertexArrays(1, &m_ID); }

OGLW_VertexArray::OGLW_VertexArray(OGLW_VertexArray&& other) noexcept
    : m_ID(other.m_ID)
    , m_vertex_format(other.m_vertex_format) {
    other.m_ID = 0;
    other.m_vertex_format = {};
}

auto OGLW_VertexArray::operator=(OGLW_VertexArray&& other) -> OGLW_VertexArray& {
    m_ID = other.m_ID;
    m_vertex_format = other.m_vertex_format;

    other.m_ID = 0;
    other.m_vertex_format = {};

    return *this;
}

OGLW_VertexArray::OGLW_VertexArray(
    OpenGL_Context*     context,
    const VertexFormat& vertex_format
) {
    (void)context;
    glCreateVertexArrays(1, &m_ID);
    this->set_layout(vertex_format);
}

auto OGLW_VertexArray::bind() const -> void {
    assert(m_ID != 0);
    glBindVertexArray(m_ID);
}

auto OGLW_VertexArray::unbind() const -> void { glBindVertexArray(0); }

static auto SHADER_TYPE_to_openGL_type(const SHADER_TYPE type) -> GLenum {
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

auto OGLW_VertexArray::bind_buffer(const opengl::Buffer& buffer) const -> void {
    glVertexArrayVertexBuffer(m_ID, 0, buffer.m_id, 0, m_vertex_format.m_stride);
}

auto OGLW_VertexArray::set_layout(const VertexFormat& vertex_format) -> void {
    m_vertex_format = vertex_format;

    for (u32 i = 0; i != vertex_format.m_attributes.size(); i++) {
        const VertexAttribute& attribute = vertex_format.m_attributes[i];

        switch (attribute.type) {
            case SHADER_TYPE::F32:
            case SHADER_TYPE::V_2_F32:
            case SHADER_TYPE::V_3_F32:
            case SHADER_TYPE::V_4_F32: {

            } break;
            default: {
                assert(false);
            }
        }
        this->enable_attrib(i);
        this->set_attrib_format(
            i, attribute.type, attribute.normalized, attribute.offset
        );
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
    const GLenum gl_type = SHADER_TYPE_to_openGL_type(type);
    glVertexArrayAttribFormat(
        m_ID, index, count, gl_type, normalized ? GL_TRUE : GL_FALSE, offset
    );
}

auto OGLW_VertexArray::set_attrib_binding(const u32 index, const u32 binding) const
    -> void {
    glVertexArrayAttribBinding(m_ID, index, binding);
}

// OGLW_Shader::OGLW_Shader(OGLW_Shader&& other) noexcept : m_ID(other.release()) {
// }
OGLW_Shader::OGLW_Shader(const GLenum type)
    : m_ID(glCreateShader(type)) {}

OGLW_Shader::~OGLW_Shader() { this->reset(); }

auto OGLW_Shader::release() -> GLuint {
    GLuint ret = m_ID;
    m_ID = 0;
    return ret;
}

auto OGLW_Shader::reset(GLuint ID) -> void {
    glDeleteShader(m_ID);
    m_ID = ID;
}

auto OGLW_Shader::set_source(const std::string& source_code) -> void {
    const GLchar* source = source_code.c_str();
    glShaderSource(m_ID, 1, &source, nullptr);
}

auto OGLW_Shader::compile() -> void { glCompileShader(m_ID); }

#define GL_SHADER_BINARY_FORMAT_SPIR_V_ARB 0x9551

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

auto OGLW_Shader::get_info_log(GLsizei max_length) -> std::string {
    GLchar info_log[512];
    glGetShaderInfoLog(m_ID, max_length, &max_length, &info_log[0]);
    std::string result(info_log);
    return result;
}

OGLW_Program::OGLW_Program()
    : m_ID(glCreateProgram()) {}

// OGLW_Program::OGLW_Program(OGLW_Program&& other) noexcept : m_ID(other.release()) {
// }
OGLW_Program::~OGLW_Program() { this->reset(); }

auto OGLW_Program::release() -> GLuint {
    GLuint ret = m_ID;
    m_ID = 0;
    return ret;
}

auto OGLW_Program::reset(GLuint ID) -> void {
    glDeleteProgram(m_ID);
    m_ID = ID;
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
    glGetProgramInfoLog(m_ID, N, nullptr, info_log);
    std::string result(info_log);
    return result;
}

} // namespace JadeFrame