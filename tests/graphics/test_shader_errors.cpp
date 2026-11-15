#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "JadeFrame/graphics/graphics_language.h"
#include "JadeFrame/graphics/shader_loader.h"

using namespace JadeFrame;

namespace {

const char* const g_valid_vertex = R"(
#version 450
layout(location = 0) in vec3 v_position;
layout(std140, set = 0, binding = 0) uniform Camera {
    mat4 view_projection;
} u_camera;
void main() { gl_Position = u_camera.view_projection * vec4(v_position, 1.0); }
)";

const char* const g_set_out_of_range_vertex = R"(
#version 450
layout(location = 0) in vec3 v_position;
layout(std140, set = 7, binding = 0) uniform Camera {
    mat4 view_projection;
} u_camera;
void main() { gl_Position = u_camera.view_projection * vec4(v_position, 1.0); }
)";

} // namespace

TEST(ShaderLoader, UnknownNameIsReported) {
    const auto result = GLSLCodeLoader::get_by_name("no_such_shader");
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, ShaderErrorCode::UNKNOWN_SHADER_NAME);
    EXPECT_NE(result.error().m_message.find("no_such_shader"), std::string::npos);
}

TEST(ShaderLoader, EmptyNameIsReported) {
    const auto result = GLSLCodeLoader::get_by_name("");
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, ShaderErrorCode::UNKNOWN_SHADER_NAME);
}

TEST(ShaderLoader, BuiltInShadersCompile) {
    for (const char* name :
         {"with_texture_0",
          "spirv_test_1",
          "depth_testing_0",
          "light_server",
          "light_client",
          "framebuffer_test"}) {
        const auto result = GLSLCodeLoader::get_by_name(name);
        ASSERT_TRUE(result.is_ok()) << name << ": " << result.error().m_message;
        const ShadingCode& code = result.value();
        EXPECT_EQ(code.m_shading_language, SHADING_LANGUAGE::GLSL_VULKAN);
        ASSERT_EQ(code.m_modules.size(), 2U) << name;
        EXPECT_EQ(code.m_modules[0].m_stage, SHADER_STAGE::VERTEX);
        EXPECT_EQ(code.m_modules[1].m_stage, SHADER_STAGE::FRAGMENT);
        EXPECT_FALSE(code.m_modules[0].m_code.empty()) << name;
        EXPECT_FALSE(code.m_modules[1].m_code.empty()) << name;
    }
}

TEST(ShaderCompilation, ValidGlslCompiles) {
    const auto result =
        GLSL_to_SPIRV(g_valid_vertex, SHADER_STAGE::VERTEX, GRAPHICS_API::VULKAN);
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    EXPECT_FALSE(result.value().empty());
}

TEST(ShaderCompilation, InvalidGlslIsReportedWithDiagnostics) {
    const std::string source = R"(
#version 450
void main() { gl_Position = this_is_not_declared; }
)";
    const auto result = GLSL_to_SPIRV(source, SHADER_STAGE::VERTEX, GRAPHICS_API::VULKAN);
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, ShaderErrorCode::COMPILATION_FAILED);
    // The compiler's own diagnostic has to survive, it names the offending symbol.
    EXPECT_NE(result.error().m_message.find("this_is_not_declared"), std::string::npos)
        << result.error().m_message;
}

TEST(ShaderCompilation, NonGlslInputIsReported) {
    const std::vector<std::string> sources = {"", "not a shader at all {"};
    for (const std::string& source : sources) {
        const auto result =
            GLSL_to_SPIRV(source, SHADER_STAGE::FRAGMENT, GRAPHICS_API::VULKAN);
        ASSERT_TRUE(result.is_err()) << "source: '" << source << "'";
        EXPECT_EQ(result.error().m_code, ShaderErrorCode::COMPILATION_FAILED);
        EXPECT_FALSE(result.error().m_message.empty());
    }
}

TEST(ShaderCompilation, UnsupportedStageIsReported) {
    const auto result =
        GLSL_to_SPIRV(g_valid_vertex, SHADER_STAGE::GEOMETRY, GRAPHICS_API::VULKAN);
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, ShaderErrorCode::UNSUPPORTED_STAGE);
}

TEST(ShaderCompilation, UnsupportedApiIsReported) {
    const auto result =
        GLSL_to_SPIRV(g_valid_vertex, SHADER_STAGE::VERTEX, GRAPHICS_API::D3D11);
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, ShaderErrorCode::UNSUPPORTED_API);
}

TEST(ShaderRemap, ValidModuleIsRemappedForOpenGL) {
    const auto spirv =
        GLSL_to_SPIRV(g_valid_vertex, SHADER_STAGE::VERTEX, GRAPHICS_API::VULKAN);
    ASSERT_TRUE(spirv.is_ok()) << spirv.error().m_message;

    std::string source;
    const auto  result = remap_for_opengl(spirv.value(), SHADER_STAGE::VERTEX, &source);
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    EXPECT_FALSE(result.value().empty());
    EXPECT_FALSE(source.empty());
}

TEST(ShaderRemap, MalformedSpirvIsReported) {
    const std::vector<std::vector<u32>> modules = {
        {},
        {0xDEADBEEF, 1, 2, 3, 4, 5, 6, 7},
        // A valid header followed by an instruction that claims more words than exist.
        {0x07230203, 0x00010000, 0, 16, 0, 0xFFFF0011},
    };
    for (const auto& module : modules) {
        const auto result = remap_for_opengl(module, SHADER_STAGE::VERTEX, nullptr);
        ASSERT_TRUE(result.is_err()) << "module of " << module.size() << " words";
        EXPECT_EQ(result.error().m_code, ShaderErrorCode::INVALID_SPIRV);
        EXPECT_FALSE(result.error().m_message.empty());
    }
}

TEST(ShaderRemap, DescriptorSetOutOfRangeIsReported) {
    const auto spirv = GLSL_to_SPIRV(
        g_set_out_of_range_vertex, SHADER_STAGE::VERTEX, GRAPHICS_API::VULKAN
    );
    ASSERT_TRUE(spirv.is_ok()) << spirv.error().m_message;

    const auto result = remap_for_opengl(spirv.value(), SHADER_STAGE::VERTEX, nullptr);
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, ShaderErrorCode::INVALID_SPIRV);
    EXPECT_NE(result.error().m_message.find('7'), std::string::npos);
}
