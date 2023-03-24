#pragma once
/*
        This is the place for the light OpenGL Wrapper classes.
        The goal is to make OpenGL objects more C++ like.
        This may or may not be temporary.

        These wrapper classes should have as little state as possible. The optimal would be to only have their ID and
   nothing more
*/
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"
#include <glad/glad.h>
#include "opengl_texture.h"

#include <vector>
#include <string>
#include <cassert>

namespace JadeFrame {




/*******************
 *	VERTEX ARRAY
 *******************/
class OGLW_VertexArray {
public:
    OGLW_VertexArray();
    ~OGLW_VertexArray();

    OGLW_VertexArray(OGLW_VertexArray&) = delete;
    auto operator=(const OGLW_VertexArray&) -> OGLW_VertexArray& = delete;

    OGLW_VertexArray(OGLW_VertexArray&& other) noexcept;
    auto operator=(OGLW_VertexArray&&) -> OGLW_VertexArray&;



    auto bind() const -> void;
    auto unbind() const -> void;

private:
    GLuint m_ID;

private:
    auto release() -> GLuint {
        GLuint ret = m_ID;
        m_ID = 0;
        return ret;
    }
    auto reset(GLuint ID = 0) -> void {
        glDeleteVertexArrays(1, &m_ID);
        m_ID = ID;
    }
};

/*******************
 *	SHADER
 *******************/

struct OGLW_Shader {
    OGLW_Shader() = default;
    ~OGLW_Shader();

    OGLW_Shader(const OGLW_Shader&) = delete;
    auto operator=(const OGLW_Shader&) -> OGLW_Shader& = delete;

    OGLW_Shader(OGLW_Shader&& other) noexcept = delete;
    auto operator=(OGLW_Shader&&) -> OGLW_Shader& = delete;

    OGLW_Shader(const GLenum type);
    // OGLW_Shader(const GLenum type, const std::string& source_code);




    auto set_source(const std::string& source_code) -> void;
    auto compile() -> void;
    auto set_binary(const std::vector<u32>& binary) -> void;
    auto compile_binary() -> void;
    auto get_info(GLenum pname) -> GLint;
    auto get_compile_status() -> GLint;
    auto get_info_log(GLsizei max_length) -> std::string;

public:
    GLuint m_ID = 0;

private:
    auto release() -> GLuint;
    auto reset(GLuint ID = 0) -> void;
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
    auto link() const -> bool;
    auto detach(const OGLW_Shader& shader) const -> void;
    auto validate() const -> bool;

    auto get_uniform_block_index(const char* name) const -> GLuint;
    auto set_uniform_block_binding(GLuint index, GLuint binding_point) const -> void;


    auto get_info(GLenum pname) const -> GLint;

    auto get_info_log() const -> std::string;

public:
    GLuint m_ID = 0;

private:
    auto release() -> GLuint;
    auto reset(GLuint ID = 0) -> void;
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

    auto bind() const -> void;
    auto unbind() const -> void;


    GLuint m_ID = 0;

private:
    auto release() -> GLuint;
    auto reset(GLuint ID = 0) -> void;
};


inline Renderbuffer::Renderbuffer() { glCreateRenderbuffers(1, &m_ID); }

inline Renderbuffer::~Renderbuffer() {}

inline auto Renderbuffer::store(GLenum internal_format, GLsizei width, GLsizei height) const -> void {
    glNamedRenderbufferStorage(m_ID, internal_format, width, height);
}

inline auto Renderbuffer::bind() const -> void { glBindRenderbuffer(GL_RENDERBUFFER, m_ID); }

inline auto Renderbuffer::unbind() const -> void { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

inline auto Renderbuffer::release() -> GLuint {
    GLuint ret = m_ID;
    m_ID = 0;
    return ret;
}
inline auto Renderbuffer::reset(GLuint ID) -> void {
    // glDeleteFramebuffers(1, &m_ID);
    glDeleteRenderbuffers(1, &m_ID);
    m_ID = ID;
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

    Framebuffer(OpenGL_Context& context);

    auto attach_texture(ATTACHMENT attachment, u32 i, const Texture& texture) const -> void;
    auto attach_renderbuffer(ATTACHMENT attachment, u32 i, const Renderbuffer& renderbuffer) const -> void;
    auto check_status() const -> GLenum;
    auto bind() const -> void;
    auto unbind() const -> void;



private:
    auto release() -> GLuint;
    auto reset(GLuint ID = 0) -> void;

public:
    GLuint m_ID = 0;
};



inline auto Framebuffer::operator=(Framebuffer&& other) noexcept -> Framebuffer& {
    m_ID = other.release();
    return *this;
}
inline Framebuffer::Framebuffer() {
    // glCreateFramebuffers(1, &m_ID);
}
inline Framebuffer::Framebuffer(OpenGL_Context& context) { glCreateFramebuffers(1, &m_ID); }
inline Framebuffer::~Framebuffer() { this->reset(); }
inline auto Framebuffer::attach_texture(ATTACHMENT attachment, u32 i, const Texture& texture) const -> void {

    assert(i < GL_MAX_COLOR_ATTACHMENTS - 1);

    i32 attachment_point = 0;
    if (attachment == ATTACHMENT::COLOR) {
        attachment_point = GL_COLOR_ATTACHMENT0 + i;
    } else if (attachment == ATTACHMENT::DEPTH) {
        attachment_point = GL_DEPTH_ATTACHMENT;
    } else if (attachment == ATTACHMENT::STENCIL) {
        attachment_point = GL_STENCIL_ATTACHMENT;
    } else if (attachment == ATTACHMENT::DEPTH_STENCIL) {
        attachment_point = GL_DEPTH_STENCIL_ATTACHMENT;
    }
    glNamedFramebufferTexture(m_ID, attachment_point, texture.m_id, 0);
}
inline auto Framebuffer::attach_renderbuffer(ATTACHMENT attachment, u32 i, const Renderbuffer& renderbuffer) const
    -> void {
    assert(i < GL_MAX_COLOR_ATTACHMENTS - 1);

    i32 attachment_point = 0;
    if (attachment == ATTACHMENT::COLOR) {
        attachment_point = GL_COLOR_ATTACHMENT0 + i;
    } else if (attachment == ATTACHMENT::DEPTH) {
        attachment_point = GL_DEPTH_ATTACHMENT;
    } else if (attachment == ATTACHMENT::STENCIL) {
        attachment_point = GL_STENCIL_ATTACHMENT;
    } else if (attachment == ATTACHMENT::DEPTH_STENCIL) {
        attachment_point = GL_DEPTH_STENCIL_ATTACHMENT;
    }
    glNamedFramebufferRenderbuffer(m_ID, attachment_point, GL_RENDERBUFFER, renderbuffer.m_ID);
}
inline auto Framebuffer::bind() const -> void { glBindFramebuffer(GL_FRAMEBUFFER, m_ID); }

inline auto Framebuffer::unbind() const -> void { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

inline auto Framebuffer::check_status() const -> GLenum {
    GLenum result = glCheckNamedFramebufferStatus(m_ID, GL_FRAMEBUFFER);
    return result;
}

inline auto Framebuffer::release() -> GLuint {
    GLuint ret = m_ID;
    m_ID = 0;
    return ret;
}
inline auto Framebuffer::reset(GLuint ID) -> void {
    glDeleteFramebuffers(1, &m_ID);
    m_ID = ID;
}
} // namespace opengl
} // namespace JadeFrame