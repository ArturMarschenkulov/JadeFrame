// Controlled driver failures without a window or a real graphics context.
#include <gtest/gtest.h>
#include "JadeFrame/graphics/opengl/opengl_renderer.h"
#include "JadeFrame/graphics/shader_loader.h"
#include <cstring>
#include <map>

using namespace JadeFrame;

namespace {
template<class T>
struct Replace {
    T& slot;
    T  saved;

    Replace(T& slot, T replacement)
        : slot(slot)
        , saved(slot) {
        slot = replacement;
    }

    ~Replace() { slot = saved; }
};

struct Driver {
    static inline GLuint                fail_object = 0;
    static inline GLuint                next = 1;
    static inline std::map<GLuint, int> live;
    static inline bool                  allocation = true;
    static inline bool                  compile = true;
    static inline bool                  link = true;
    static inline GLint64               bytes = 0;
    static inline GLint                 width = 0, height = 0, unpack = 4;
    static inline int                   uploads = 0;
    static inline GLenum                error = GL_NO_ERROR;
    static inline bool                  upload = true;

    static GLenum APIENTRY get_error() { return std::exchange(error, GL_NO_ERROR); }

    static void APIENTRY create(GLsizei n, GLuint* ids) {
        for (int i = 0; i < n; ++i) {
            const auto id = next++;
            ids[i] = id == fail_object ? 0 : id;
            if (ids[i]) { live[ids[i]] = 1; }
        }
    }

    static void APIENTRY remove(GLsizei n, const GLuint* ids) {
        for (int i = 0; i < n; ++i) {
            EXPECT_EQ(live.erase(ids[i]), 1U) << "double deletion";
        }
    }

    static GLuint APIENTRY object() {
        GLuint id;
        create(1, &id);
        return id;
    }

    static GLuint APIENTRY shader(GLenum) { return object(); }

    static void APIENTRY delete_object(GLuint id) { remove(1, &id); }

    static void APIENTRY create_texture(GLenum, GLsizei n, GLuint* ids) {
        create(n, ids);
    }

    static void APIENTRY buffer_data(GLuint, GLsizeiptr size, const void*, GLenum) {
        bytes = allocation ? size : 0;
    }

    static void APIENTRY buffer_size(GLuint, GLenum, GLint64* size) { *size = bytes; }

    static void APIENTRY texture_storage(GLuint, GLsizei, GLenum, GLsizei w, GLsizei h) {
        width = allocation ? w : 0;
        height = allocation ? h : 0;
    }

    static void APIENTRY texture_size(GLuint, GLint, GLenum name, GLint* out) {
        *out = name == GL_TEXTURE_WIDTH ? width : height;
    }

    static void APIENTRY texture_upload(
        GLuint,
        GLint,
        GLint,
        GLint,
        GLsizei,
        GLsizei,
        GLenum,
        GLenum,
        const void*
    ) {
        ++uploads;
        EXPECT_EQ(unpack, 1);
        if (!upload) { error = GL_OUT_OF_MEMORY; }
    }

    static void APIENTRY integer(GLenum name, GLint* out) {
        *out = name == GL_UNPACK_ALIGNMENT ? unpack : 4096;
    }

    static void APIENTRY pixel_store(GLenum, GLint v) { unpack = v; }

    static void APIENTRY
    shader_source(GLuint, GLsizei, const GLchar* const*, const GLint*) {}

    static void APIENTRY one(GLuint) {}

    static void APIENTRY two(GLuint, GLuint) {}

    static void APIENTRY attrib_format(GLuint, GLuint, GLint, GLenum, GLboolean, GLuint) {
    }

    static void APIENTRY attrib_binding(GLuint, GLuint, GLuint) {}

    static void APIENTRY shader_info(GLuint, GLenum name, GLint* out) {
        *out = name == GL_INFO_LOG_LENGTH ? 8 : compile;
    }

    static void APIENTRY program_info(GLuint, GLenum name, GLint* out) {
        *out = name == GL_INFO_LOG_LENGTH ? 8 : link;
    }

    static void APIENTRY log(GLuint, GLsizei, GLsizei* size, GLchar* data) {
        std::memcpy(data, "failure", 7);
        *size = 7;
    }
};

#define REPLACE(name, stub)                                                              \
    Replace<decltype(::glad_##name)> guard_##name { ::glad_##name, &Driver::stub }

struct TestContext : opengl::Context {
    TestContext() { m_texture_manager.m_context = this; }
};

class OpenGLResources : public testing::Test {
protected:
    REPLACE(glCreateFramebuffers, create);
    REPLACE(glDeleteFramebuffers, remove);
    REPLACE(glCreateRenderbuffers, create);
    REPLACE(glDeleteRenderbuffers, remove);
    REPLACE(glCreateSamplers, create);
    REPLACE(glDeleteSamplers, remove);
    REPLACE(glGetError, get_error);
    REPLACE(glCreateBuffers, create);
    REPLACE(glDeleteBuffers, remove);
    REPLACE(glNamedBufferData, buffer_data);
    REPLACE(glGetNamedBufferParameteri64v, buffer_size);
    REPLACE(glCreateTextures, create_texture);
    REPLACE(glDeleteTextures, remove);
    REPLACE(glTextureStorage2D, texture_storage);
    REPLACE(glGetTextureLevelParameteriv, texture_size);
    REPLACE(glTextureSubImage2D, texture_upload);
    REPLACE(glGetIntegerv, integer);
    REPLACE(glPixelStorei, pixel_store);
    REPLACE(glCreateShader, shader);
    REPLACE(glDeleteShader, delete_object);
    REPLACE(glCreateProgram, object);
    REPLACE(glDeleteProgram, delete_object);
    REPLACE(glShaderSource, shader_source);
    REPLACE(glCompileShader, one);
    REPLACE(glAttachShader, two);
    REPLACE(glDetachShader, two);
    REPLACE(glLinkProgram, one);
    REPLACE(glGetShaderiv, shader_info);
    REPLACE(glGetProgramiv, program_info);
    REPLACE(glGetShaderInfoLog, log);
    REPLACE(glGetProgramInfoLog, log);
    REPLACE(glCreateVertexArrays, create);
    REPLACE(glDeleteVertexArrays, remove);
    REPLACE(glEnableVertexArrayAttrib, two);
    REPLACE(glVertexArrayAttribFormat, attrib_format);
    REPLACE(glVertexArrayAttribBinding, attrib_binding);

    auto SetUp() -> void override {
        Driver::next = 1;
        Driver::fail_object = 0;
        Driver::allocation = Driver::compile = Driver::link = true;
        Driver::uploads = 0;
        Driver::upload = true;
        Driver::error = GL_NO_ERROR;
        Driver::live.clear();
    }

    auto TearDown() -> void override { EXPECT_TRUE(Driver::live.empty()); }
};

#undef REPLACE
} // namespace

TEST_F(OpenGLResources, FailedBuffersNeverEnterContextRegistry) {
    TestContext context;
    int         data = 0;
    auto*       previous =
        context.create_buffer(opengl::Buffer::TYPE::VERTEX, &data, sizeof(data));
    ASSERT_NE(previous, nullptr);
    const auto id = previous->m_id;
    Driver::allocation = false;
    EXPECT_EQ(
        context.create_buffer(opengl::Buffer::TYPE::VERTEX, &data, sizeof(data)), nullptr
    );
    EXPECT_EQ(
        context.create_buffer(opengl::Buffer::TYPE::INDEX, &data, sizeof(data)), nullptr
    );
    EXPECT_EQ(context.m_bufferss.size(), 1U);
    EXPECT_EQ(previous->m_id, id);
    EXPECT_EQ(Driver::live.size(), 1U);
}

TEST_F(OpenGLResources, FailedTexturesNeverEnterManagerAndBorrowedHandlesDoNotDelete) {
    TestContext   context;
    const auto    size = v2u32::create(1, 1);
    unsigned char pixels[3]{};
    auto*         texture = context.create_texture(pixels, size, 3);
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(Driver::uploads, 1);
    EXPECT_EQ(Driver::unpack, 4);
    { NativeHandle borrowed(texture, noop_native_handle_deleter); }
    EXPECT_EQ(Driver::live.size(), 1U);
    Driver::allocation = false;
    EXPECT_EQ(context.create_texture(pixels, size, 3), nullptr);
    EXPECT_EQ(context.create_texture(pixels, size, 2), nullptr);
    EXPECT_EQ(context.m_texture_manager.m_textures.size(), 1U);
    EXPECT_EQ(Driver::live.size(), 1U);
}

TEST_F(OpenGLResources, NullTextureDataAllocatesStorageWithoutUpload) {
    TestContext context;
    ASSERT_NE(context.create_texture(nullptr, v2u32::create(1, 1), 3), nullptr);
    EXPECT_EQ(Driver::uploads, 0);
}

TEST_F(OpenGLResources, CompilationAndLinkFailureReleaseEveryNativeObject) {
    RenderSystem    system;
    OpenGL_Renderer renderer(system, nullptr);
    auto            code = GLSLCodeLoader::get_by_name("spirv_test_1");
    ASSERT_TRUE(code.is_ok());
    for (int step : {0, 1}) {
        Driver::compile = step != 0;
        Driver::link = false;
        auto result = renderer.try_create_shader(code.value());
        ASSERT_TRUE(result.is_err());
        EXPECT_NE(
            result.error().m_message.find(step == 0 ? "compilation" : "linking"),
            std::string::npos
        );
        EXPECT_NE(result.error().m_message.find("failure"), std::string::npos);
        EXPECT_TRUE(Driver::live.empty());
    }
    Driver::compile = Driver::link = true;
    auto result = renderer.try_create_shader(code.value());
    ASSERT_TRUE(result.is_ok());
    EXPECT_FALSE(Driver::live.empty());
}

TEST_F(OpenGLResources, MaterialUniformAllocationFailureRollsBackEarlierBuffers) {
    TestContext                    context;
    opengl::Shader                 shader;
    ReflectedModule::UniformBuffer first{};
    first.size = 64;
    first.binding = 0;
    ReflectedModule::UniformBuffer second{};
    second.binding = 1;
    shader.m_reflected_interface.m_uniform_buffers = {first, second};
    {
        opengl::Material material(context, shader, nullptr);
        EXPECT_FALSE(material.m_is_initialized);
        EXPECT_EQ(context.m_bufferss.size(), 1U);
    }
    EXPECT_TRUE(context.m_bufferss.empty());
    EXPECT_TRUE(context.m_bound_uniform_buffer_locations.empty());
}

TEST_F(OpenGLResources, TextureUploadFailureReleasesStorage) {
    TestContext   context;
    unsigned char pixels[4]{};
    Driver::upload = false;
    EXPECT_EQ(context.create_texture(pixels, v2u32::create(1, 1), 4), nullptr);
    EXPECT_TRUE(context.m_texture_manager.m_textures.empty());
    EXPECT_TRUE(Driver::live.empty());
}

TEST_F(OpenGLResources, ShaderObjectAllocationFailuresReleaseEarlierObjects) {
    RenderSystem    system;
    OpenGL_Renderer renderer(system, nullptr);
    auto            code = GLSLCodeLoader::get_by_name("spirv_test_1");
    ASSERT_TRUE(code.is_ok());
    for (GLuint step : {0U, 1U, 2U, 3U}) {
        Driver::fail_object = Driver::next + step;
        EXPECT_TRUE(renderer.try_create_shader(code.value()).is_err());
        EXPECT_TRUE(Driver::live.empty());
    }
}

TEST_F(OpenGLResources, NullNativeObjectsNeverEnterContextCollections) {
    TestContext context;
    int         data = 0;
    Driver::fail_object = Driver::next;
    EXPECT_EQ(
        context.create_buffer(opengl::Buffer::TYPE::VERTEX, &data, sizeof(data)), nullptr
    );
    Driver::fail_object = Driver::next;
    EXPECT_EQ(context.create_texture(nullptr, v2u32::create(1, 1), 4), nullptr);
    Driver::fail_object = Driver::next;
    EXPECT_EQ(context.create_sampler(), nullptr);
    Driver::fail_object = Driver::next;
    EXPECT_EQ(context.create_framebuffer(), nullptr);
    Driver::fail_object = Driver::next;
    EXPECT_EQ(context.create_renderbuffer(), nullptr);
    EXPECT_TRUE(context.m_bufferss.empty());
    EXPECT_TRUE(context.m_texture_manager.m_textures.empty());
    EXPECT_TRUE(context.m_texture_manager.m_samplers.empty());
    EXPECT_TRUE(context.m_framebuffers.empty());
    EXPECT_TRUE(context.m_renderbuffers.empty());
}

TEST_F(OpenGLResources, InvalidShaderInputDoesNotAllocateAndStageOrderIsPreserved) {
    RenderSystem    system;
    OpenGL_Renderer renderer(system, nullptr);
    auto            code = GLSLCodeLoader::get_by_name("spirv_test_1");
    ASSERT_TRUE(code.is_ok());
    auto malformed = code.value();
    malformed.m_modules[0].m_code = {0, 1};
    EXPECT_TRUE(renderer.try_create_shader(malformed).is_err());
    malformed = code.value();
    malformed.m_modules[1].m_stage = SHADER_STAGE::VERTEX;
    EXPECT_TRUE(renderer.try_create_shader(malformed).is_err());
    EXPECT_EQ(Driver::next, 1U);
    std::swap(code.value().m_modules[0], code.value().m_modules[1]);
    EXPECT_TRUE(renderer.try_create_shader(code.value()).is_ok());
}

TEST_F(OpenGLResources, MissingRequiredTextureIsRejectedBeforeBufferAllocation) {
    TestContext                    context;
    opengl::Shader                 shader;
    ReflectedModule::UniformBuffer uniform{};
    uniform.size = 64;
    shader.m_reflected_interface.m_uniform_buffers.push_back(uniform);
    shader.m_reflected_interface.m_sampled_images.emplace_back();
    const auto       next = Driver::next;
    opengl::Material material(context, shader, nullptr);
    EXPECT_FALSE(material.m_is_initialized);
    EXPECT_EQ(Driver::next, next);
    EXPECT_TRUE(context.m_bufferss.empty());
}
