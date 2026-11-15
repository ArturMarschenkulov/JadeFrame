#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "JadeFrame/assets/asset_loader.h"

using namespace JadeFrame;

namespace {

// Writes OBJ fixtures into a directory of its own, which is removed again afterwards.
class AssetLoaderObj : public ::testing::Test {
protected:
    void SetUp() override {
        const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        m_dir = std::filesystem::temp_directory_path() /
                (std::string("jadeframe_asset_loader_") + info->name());
        std::filesystem::remove_all(m_dir);
        std::filesystem::create_directories(m_dir);
    }

    void TearDown() override {
        std::error_code ignored;
        std::filesystem::remove_all(m_dir, ignored);
    }

    auto write(const std::string& name, std::string_view content) const -> std::string {
        const std::filesystem::path path = m_dir / name;
        std::ofstream               file(path, std::ios::binary);
        file << content;
        return path.string();
    }

    static void expect_error(const std::string& path, AssetErrorCode code) {
        const auto result = AssetLoader::load_obj(path);
        ASSERT_TRUE(result.is_err()) << "Loading '" << path << "' should fail";
        EXPECT_EQ(result.error().m_code, code)
            << to_string(result.error().m_code) << ": " << result.error().m_message;
        EXPECT_EQ(result.error().m_path, path);
        EXPECT_FALSE(result.error().m_message.empty());
    }

    std::filesystem::path m_dir;
};

} // namespace

TEST_F(AssetLoaderObj, LoadsTriangle) {
    const std::string path = write(
        "triangle.obj",
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vn 0 0 1\n"
        "f 1//1 2//1 3//1\n"
    );

    const auto result = AssetLoader::load_obj(path);
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    const Mesh& mesh = result.value();
    ASSERT_NE(mesh.attribute_values(Mesh::POSITION.m_id), nullptr);
    EXPECT_EQ(mesh.attribute_values(Mesh::POSITION.m_id)->size(), 9U);
    ASSERT_NE(mesh.attribute_values(Mesh::NORMAL.m_id), nullptr);
    EXPECT_EQ(mesh.attribute_values(Mesh::NORMAL.m_id)->size(), 9U);
    EXPECT_EQ(mesh.m_indices.size(), 3U);
    EXPECT_TRUE(mesh.validate().is_ok());
}

TEST_F(AssetLoaderObj, TriangulatesQuad) {
    const std::string path = write(
        "quad.obj",
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 1 0\n"
        "f 1 2 3 4\n"
    );

    const auto result = AssetLoader::load_obj(path);
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    EXPECT_EQ(result.value().m_indices.size(), 6U);
    EXPECT_TRUE(result.value().validate().is_ok());
}

TEST_F(AssetLoaderObj, MissingFileIsReported) {
    expect_error((m_dir / "does_not_exist.obj").string(), AssetErrorCode::FILE_NOT_FOUND);
}

TEST_F(AssetLoaderObj, DirectoryIsNotAFile) {
    expect_error(m_dir.string(), AssetErrorCode::FILE_NOT_FOUND);
}

TEST_F(AssetLoaderObj, EmptyPathIsReported) {
    expect_error("", AssetErrorCode::FILE_NOT_FOUND);
}

TEST_F(AssetLoaderObj, FaceWithUnknownVertexFailsImport) {
    const std::string path = write(
        "bad_index.obj",
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "f 1 2 99\n"
    );
    expect_error(path, AssetErrorCode::IMPORT_FAILED);
}

TEST_F(AssetLoaderObj, BinaryGarbageFailsImport) {
    std::string garbage;
    for (int i = 0; i < 4096; ++i) {
        garbage.push_back(static_cast<char>((i * 131 + 7) % 256));
    }
    const auto result = AssetLoader::load_obj(write("garbage.obj", garbage));
    // Depending on how much of the noise the importer skips, it either gives up or
    // ends up with nothing. Both are fine, a mesh or a crash is not.
    ASSERT_TRUE(result.is_err());
    EXPECT_TRUE(
        result.error().m_code == AssetErrorCode::IMPORT_FAILED ||
        result.error().m_code == AssetErrorCode::EMPTY_SCENE
    ) << to_string(result.error().m_code);
}

TEST_F(AssetLoaderObj, ZeroByteFileIsRejected) {
    const auto result = AssetLoader::load_obj(write("zero.obj", ""));
    ASSERT_TRUE(result.is_err());
    EXPECT_TRUE(
        result.error().m_code == AssetErrorCode::IMPORT_FAILED ||
        result.error().m_code == AssetErrorCode::EMPTY_SCENE
    ) << to_string(result.error().m_code);
}

TEST_F(AssetLoaderObj, SceneWithoutGeometryIsEmpty) {
    const std::string path = write(
        "empty_scene.obj",
        "# An object, but nothing in it\n"
        "o nothing\n"
    );
    expect_error(path, AssetErrorCode::EMPTY_SCENE);
}

TEST_F(AssetLoaderObj, LineGeometryIsInvalid) {
    const std::string path = write(
        "lines.obj",
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "l 1 2\n"
        "l 2 3\n"
    );
    expect_error(path, AssetErrorCode::INVALID_GEOMETRY);
}

TEST(AssetError, CodesHaveNames) {
    EXPECT_STREQ(to_string(AssetErrorCode::FILE_NOT_FOUND), "FILE_NOT_FOUND");
    EXPECT_STREQ(to_string(AssetErrorCode::IMPORT_FAILED), "IMPORT_FAILED");
    EXPECT_STREQ(to_string(AssetErrorCode::EMPTY_SCENE), "EMPTY_SCENE");
    EXPECT_STREQ(to_string(AssetErrorCode::INVALID_GEOMETRY), "INVALID_GEOMETRY");
}
