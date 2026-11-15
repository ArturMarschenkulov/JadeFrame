// Tests for the parts of the renderer contract that do not need a GPU.

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include "JadeFrame/graphics/graphics_shared.h"

#if JADEFRAME_ENABLE_VULKAN
    #include "JadeFrame/graphics/vulkan/renderer.h"
#endif

using namespace JadeFrame;

namespace {

auto temporary_png(const char* name) -> std::string {
    return (std::filesystem::temp_directory_path() / name).string();
}

} // namespace

TEST(RendererError, EveryCodeHasAName) {
    EXPECT_STREQ(to_string(RendererErrorCode::UNSUPPORTED), "UNSUPPORTED");
    EXPECT_STREQ(to_string(RendererErrorCode::INVALID_STATE), "INVALID_STATE");
    EXPECT_STREQ(to_string(RendererErrorCode::FRAME_NOT_AVAILABLE), "FRAME_NOT_AVAILABLE");
    EXPECT_STREQ(to_string(RendererErrorCode::BACKEND_FAILURE), "BACKEND_FAILURE");
    EXPECT_STREQ(to_string(RendererErrorCode::IO_FAILURE), "IO_FAILURE");
}

TEST(ShaderHandle, SetUniformWithoutRendererIsAnExplicitError) {
    ShaderHandle shader;
    const f32    value = 1.0F;

    auto result = shader.set_uniform("u_value", &value, sizeof(value));

    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, RendererErrorCode::INVALID_STATE);
    EXPECT_FALSE(result.error().m_message.empty());
}

TEST(WritePng, RejectsImagesThatCannotBeWritten) {
    const std::string path = temporary_png("jadeframe_write_png_invalid.png");
    std::filesystem::remove(path);

    Image empty;
    EXPECT_FALSE(write_png(empty, path.c_str()));

    Image truncated;
    truncated.width = 2;
    truncated.height = 2;
    truncated.num_components = 3;
    truncated.data.resize(3); // one texel instead of four
    EXPECT_FALSE(write_png(truncated, path.c_str()));

    Image valid;
    valid.width = 1;
    valid.height = 1;
    valid.num_components = 3;
    valid.data = {1, 2, 3};
    EXPECT_FALSE(write_png(valid, nullptr));
    EXPECT_FALSE(write_png(valid, "/nonexistent-directory/jadeframe.png"));

    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST(WritePng, RoundTripKeepsSizeAndBottomUpRowOrder) {
    // 2x2 image, bottom row first: red, green / blue, white.
    Image image;
    image.width = 2;
    image.height = 2;
    image.num_components = 3;
    image.data = {
        255, 0, 0,   0, 255, 0,     // bottom row
        0,   0, 255, 255, 255, 255, // top row
    };
    const std::string path = temporary_png("jadeframe_write_png_roundtrip.png");

    ASSERT_TRUE(write_png(image, path.c_str()));

    // `load_from_path` also yields the bottom row first, but always 4 components.
    Image loaded = Image::load_from_path(path);
    std::filesystem::remove(path);
    ASSERT_EQ(loaded.width, 2);
    ASSERT_EQ(loaded.height, 2);
    ASSERT_EQ(loaded.num_components, 4);
    const std::vector<u8> expected = {
        255, 0, 0,   255, 0,   255, 0,   255, // bottom row
        0,   0, 255, 255, 255, 255, 255, 255, // top row
    };
    EXPECT_EQ(loaded.data, expected);
}

#if JADEFRAME_ENABLE_VULKAN

namespace {
constexpr VkExtent2D FRAMEBUFFER = {800, 600};
constexpr VkExtent2D MAX_DIMENSIONS = {16384, 16384};
} // namespace

TEST(VulkanViewport, FullFramebufferIsFlippedAndCoversEverything) {
    const auto state = vulkan::to_viewport_state(
        vulkan::ViewportRect{0, 0, 800, 600}, FRAMEBUFFER, MAX_DIMENSIONS
    );

    EXPECT_FLOAT_EQ(state.viewport.x, 0.0F);
    EXPECT_FLOAT_EQ(state.viewport.y, 600.0F);
    EXPECT_FLOAT_EQ(state.viewport.width, 800.0F);
    EXPECT_FLOAT_EQ(state.viewport.height, -600.0F);
    EXPECT_FLOAT_EQ(state.viewport.minDepth, 0.0F);
    EXPECT_FLOAT_EQ(state.viewport.maxDepth, 1.0F);

    EXPECT_EQ(state.scissor.offset.x, 0);
    EXPECT_EQ(state.scissor.offset.y, 0);
    EXPECT_EQ(state.scissor.extent.width, 800U);
    EXPECT_EQ(state.scissor.extent.height, 600U);
}

TEST(VulkanViewport, OriginIsTheLowerLeftCorner) {
    // A 200x100 rectangle whose lower-left corner is 10 px from the left and 20 px from
    // the bottom. From the top that is rows 480 to 580.
    const auto state = vulkan::to_viewport_state(
        vulkan::ViewportRect{10, 20, 200, 100}, FRAMEBUFFER, MAX_DIMENSIONS
    );

    EXPECT_FLOAT_EQ(state.viewport.x, 10.0F);
    EXPECT_FLOAT_EQ(state.viewport.y, 580.0F);
    EXPECT_FLOAT_EQ(state.viewport.width, 200.0F);
    EXPECT_FLOAT_EQ(state.viewport.height, -100.0F);

    EXPECT_EQ(state.scissor.offset.x, 10);
    EXPECT_EQ(state.scissor.offset.y, 480);
    EXPECT_EQ(state.scissor.extent.width, 200U);
    EXPECT_EQ(state.scissor.extent.height, 100U);
}

TEST(VulkanViewport, ScissorIsClippedToTheFramebufferButTheViewportIsNot) {
    // E.g. the window was resized and the swapchain was not recreated yet.
    const auto state = vulkan::to_viewport_state(
        vulkan::ViewportRect{400, 300, 1000, 1000}, FRAMEBUFFER, MAX_DIMENSIONS
    );

    EXPECT_FLOAT_EQ(state.viewport.width, 1000.0F);
    EXPECT_FLOAT_EQ(state.viewport.height, -1000.0F);

    EXPECT_EQ(state.scissor.offset.x, 400);
    EXPECT_EQ(state.scissor.offset.y, 0);
    EXPECT_EQ(state.scissor.extent.width, 400U);
    EXPECT_EQ(state.scissor.extent.height, 300U);
}

TEST(VulkanViewport, EmptyOrOutsideRectanglesDrawNothingButStayValid) {
    const std::vector<vulkan::ViewportRect> rects = {
        {   0,    0,   0,   0}, // minimized window
        { 100,  100,   0,  50},
        { 800,    0, 100, 100}, // right of the framebuffer
        {   0,  600, 100, 100}, // above the framebuffer
        {5000, 5000,  10,  10},
    };
    for (const auto& rect : rects) {
        const auto state = vulkan::to_viewport_state(rect, FRAMEBUFFER, MAX_DIMENSIONS);

        // Vulkan requires a viewport width > 0 and a non-zero height.
        EXPECT_GE(state.viewport.width, 1.0F);
        EXPECT_LE(state.viewport.height, -1.0F);
        // Nothing may be rasterized.
        EXPECT_EQ(state.scissor.extent.width * state.scissor.extent.height, 0U);
        // The scissor has to stay inside the render area.
        EXPECT_GE(state.scissor.offset.x, 0);
        EXPECT_GE(state.scissor.offset.y, 0);
        EXPECT_LE(
            static_cast<u32>(state.scissor.offset.x) + state.scissor.extent.width,
            FRAMEBUFFER.width
        );
        EXPECT_LE(
            static_cast<u32>(state.scissor.offset.y) + state.scissor.extent.height,
            FRAMEBUFFER.height
        );
    }
}

TEST(VulkanViewport, ExtremeValuesStayInsideTheDeviceLimits) {
    const u32  max = std::numeric_limits<u32>::max();
    const auto state = vulkan::to_viewport_state(
        vulkan::ViewportRect{max, max, max, max}, FRAMEBUFFER, MAX_DIMENSIONS
    );

    const f32 limit = static_cast<f32>(MAX_DIMENSIONS.width);
    EXPECT_LE(state.viewport.width, limit);
    EXPECT_GE(state.viewport.height, -limit);
    // viewportBoundsRange is at least [-2 * max, 2 * max - 1].
    EXPECT_LE(state.viewport.x + state.viewport.width, 2.0F * limit - 1.0F);
    EXPECT_GE(state.viewport.y + state.viewport.height, -2.0F * limit);
    EXPECT_EQ(state.scissor.extent.width * state.scissor.extent.height, 0U);
}

TEST(VulkanReadback, ConvertsBgraToBottomUpRgb) {
    // 2x2, top row first: (1,2,3) (4,5,6) / (7,8,9) (10,11,12) as RGB, stored as BGRA.
    const std::vector<u8> texels = {
        3, 2, 1, 255, 6,  5,  4,  255, // top row
        9, 8, 7, 255, 12, 11, 10, 255, // bottom row
    };
    std::vector<u8> rgb;

    ASSERT_TRUE(vulkan::readback_to_rgb(texels, 2, 2, VK_FORMAT_B8G8R8A8_SRGB, rgb));

    const std::vector<u8> expected = {
        7, 8, 9, 10, 11, 12, // bottom row comes first
        1, 2, 3, 4,  5,  6,
    };
    EXPECT_EQ(rgb, expected);
}

TEST(VulkanReadback, ConvertsRgbaToBottomUpRgb) {
    const std::vector<u8> texels = {
        1, 2, 3, 255, // top row
        4, 5, 6, 255, // bottom row
    };
    std::vector<u8> rgb;

    ASSERT_TRUE(vulkan::readback_to_rgb(texels, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, rgb));

    const std::vector<u8> expected = {4, 5, 6, 1, 2, 3};
    EXPECT_EQ(rgb, expected);
}

TEST(VulkanReadback, RejectsUnsupportedFormatsAndShortInput) {
    const std::vector<u8> texels(4 * 4, 0);
    std::vector<u8>       rgb = {42};

    // 10 bit and float formats are not 4x8 bit and must not be reinterpreted.
    EXPECT_FALSE(
        vulkan::readback_to_rgb(texels, 2, 2, VK_FORMAT_A2B10G10R10_UNORM_PACK32, rgb)
    );
    EXPECT_FALSE(
        vulkan::readback_to_rgb(texels, 2, 2, VK_FORMAT_R16G16B16A16_SFLOAT, rgb)
    );
    // Less data than width * height texels.
    EXPECT_FALSE(vulkan::readback_to_rgb(texels, 4, 4, VK_FORMAT_B8G8R8A8_SRGB, rgb));

    EXPECT_EQ(rgb, std::vector<u8>{42});
}

#endif // JADEFRAME_ENABLE_VULKAN
