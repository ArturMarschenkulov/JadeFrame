#pragma once
/*
        This is the place for the light OpenGL Wrapper classes.
        The goal is to make OpenGL objects more C++ like.
        This may or may not be temporary.

        These wrapper classes should have as little state as possible. The optimal would
   be to only have their ID and nothing more


   https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions/blob/master/README.md
*/

#include <vector>
#include <string>
#include <cassert>

#include <glad/glad.h>

#include "JadeFrame/prelude.h"
#include "JadeFrame/graphics/reflect.h"
#include "../graphics_shared.h"
#include "opengl_texture.h"

namespace JadeFrame {

/*******************
 *	VERTEX ARRAY
 *******************/
class VertexFormat;

namespace opengl {
class Buffer;
}
class OpenGL_Context;

class OGLW_VertexArray {
public:
    OGLW_VertexArray() = default;
    ~OGLW_VertexArray();
    OGLW_VertexArray(OGLW_VertexArray&) = delete;
    auto operator=(const OGLW_VertexArray&) -> OGLW_VertexArray& = delete;
    OGLW_VertexArray(OGLW_VertexArray&& other) noexcept;
    auto operator=(OGLW_VertexArray&&) noexcept -> OGLW_VertexArray&;

    OGLW_VertexArray(OpenGL_Context* context, const VertexFormat& vertex_format);

    auto bind_buffer(const opengl::Buffer& buffer) const -> void;
    auto set_layout(const VertexFormat& vertex_format) -> void;

private:
    auto enable_attrib(const u32 index) const -> void;
    auto set_attrib_format(
        const u32         index,
        const SHADER_TYPE type,
        const bool        count,
        const size_t      offset
    ) const -> void;
    auto set_attrib_binding(const u32 index, const u32 binding) const -> void;

public:
    GLuint m_ID;

    VertexFormat m_vertex_format;
};

/*******************
 *	SHADER
 *******************/

struct OGLW_Shader {
    OGLW_Shader() = default;
    ~OGLW_Shader();

    OGLW_Shader(const OGLW_Shader&) = delete;
    auto operator=(const OGLW_Shader&) -> OGLW_Shader& = delete;

    OGLW_Shader(OGLW_Shader&& other) noexcept;
    auto operator=(OGLW_Shader&&) noexcept -> OGLW_Shader&;

    explicit OGLW_Shader(const GLenum type, const std::vector<u32>& binary);

    auto set_source(const std::string& source_code) -> void;
    auto compile() -> void;
    auto set_binary(const std::vector<u32>& binary) -> void;
    auto compile_binary() -> void;
    auto get_info(GLenum pname) -> GLint;
    auto get_compile_status() -> GLint;
    auto get_info_log() -> std::string;

public:
    GLuint          m_ID = 0;
    ReflectedModule m_reflected;
};

/*******************
 *	PROGRAM
 *******************/
struct OGLW_Program {
    OGLW_Program();
    ~OGLW_Program();

    OGLW_Program(const OGLW_Program&) = delete;
    auto operator=(const OGLW_Program&) -> OGLW_Program& = delete;

    OGLW_Program(OGLW_Program&& other) noexcept = delete;
    auto operator=(OGLW_Program&&) -> OGLW_Program& = delete;

    auto bind() const -> void;
    auto unbind() const -> void;
    auto attach(const OGLW_Shader& shader) const -> void;
    auto detach(const OGLW_Shader& shader) const -> void;

    [[nodiscard]] auto link() const -> bool;
    [[nodiscard]] auto validate() const -> bool;

    auto get_uniform_block_index(const char* name) const -> GLuint;
    auto set_uniform_block_binding(GLuint index, GLuint binding_point) const -> void;

    [[nodiscard]] auto get_info(GLenum pname) const -> GLint;

    [[nodiscard]] auto get_info_log() const -> std::string;

public:
    GLuint m_ID = 0;
};

namespace opengl {
/*******************
 *	RENDERBUFFER
 *******************/

class Renderbuffer {
public:
    Renderbuffer();
    ~Renderbuffer();

    Renderbuffer(const Renderbuffer&) = delete;
    auto operator=(const Renderbuffer&) noexcept -> Renderbuffer& = delete;

    auto operator=(Renderbuffer&&) noexcept -> Renderbuffer& = delete;

    auto store(GLenum internal_format, GLsizei width, GLsizei height) const -> void;

    GLuint m_ID = 0;

private:
    auto reset(GLuint ID = 0) -> void;
};

inline Renderbuffer::Renderbuffer() { glCreateRenderbuffers(1, &m_ID); }

inline Renderbuffer::~Renderbuffer() {
    glDeleteRenderbuffers(1, &m_ID);
    m_ID = 0;
}

inline auto
Renderbuffer::store(GLenum internal_format, GLsizei width, GLsizei height) const -> void {
    glNamedRenderbufferStorage(m_ID, internal_format, width, height);
}

/*******************
 *	FRAMEBUFFER
 *******************/

enum class ATTACHMENT {
    COLOR,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL,
};

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    auto operator=(const Framebuffer&) noexcept -> Framebuffer& = delete;

    auto operator=(Framebuffer&&) noexcept -> Framebuffer&;

    explicit Framebuffer(OpenGL_Context& context);

    auto attach(ATTACHMENT attachment, u32 i, const Texture& texture) const -> void;
    auto
    attach(ATTACHMENT attachment, u32 i, const Renderbuffer& renderbuffer) const -> void;

    [[nodiscard]] auto check_status() const -> GLenum;

public:
    GLuint m_ID = 0;
};

inline auto Framebuffer::operator=(Framebuffer&& other) noexcept -> Framebuffer& {
    m_ID = std::exchange(other.m_ID, 0);
    return *this;
}

inline Framebuffer::Framebuffer() {
    // glCreateFramebuffers(1, &m_ID);
}

inline Framebuffer::Framebuffer(OpenGL_Context&) { glCreateFramebuffers(1, &m_ID); }

inline Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_ID);
    m_ID = 0;
}

inline auto to_GLenum(ATTACHMENT attachment) -> GLenum {
    switch (attachment) {
        case ATTACHMENT::COLOR: return GL_COLOR_ATTACHMENT0;
        case ATTACHMENT::DEPTH: return GL_DEPTH_ATTACHMENT;
        case ATTACHMENT::STENCIL: return GL_STENCIL_ATTACHMENT;
        case ATTACHMENT::DEPTH_STENCIL: return GL_DEPTH_STENCIL_ATTACHMENT;
    }
    return 0;
}

inline auto
Framebuffer::attach(ATTACHMENT attachment, u32 i, const Texture& texture) const -> void {

    assert(i < GL_MAX_COLOR_ATTACHMENTS - 1);

    GLenum attach = to_GLenum(attachment);
    attach == GL_COLOR_ATTACHMENT0 ? attach += i : attach;
    glNamedFramebufferTexture(m_ID, attach, texture.m_id, 0);
}

inline auto
Framebuffer::attach(ATTACHMENT attachment, u32 i, const Renderbuffer& renderbuffer) const
    -> void {
    assert(i < GL_MAX_COLOR_ATTACHMENTS - 1);

    GLenum attach = to_GLenum(attachment);
    attach == GL_COLOR_ATTACHMENT0 ? attach += i : attach;
    glNamedFramebufferRenderbuffer(m_ID, attach, GL_RENDERBUFFER, renderbuffer.m_ID);
}

inline auto Framebuffer::check_status() const -> GLenum {
    GLenum result = glCheckNamedFramebufferStatus(m_ID, GL_FRAMEBUFFER);
    return result;
}

} // namespace opengl
} // namespace JadeFrame