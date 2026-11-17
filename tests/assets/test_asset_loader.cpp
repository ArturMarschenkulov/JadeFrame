#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
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

namespace {

auto fixture(const char* name) -> std::string {
    return (std::filesystem::path(JADEFRAME_ASSET_FIXTURE_DIR) / name).string();
}

auto read_fixture(const char* name) -> std::string {
    std::ifstream file(fixture(name), std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

} // namespace

TEST(AssetLoaderScene, ImportsEveryMeshAndMaterial) {
    const auto result = AssetLoader::load_scene(fixture("multi_mesh.obj"));
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    const auto& scene = result.value();
    ASSERT_EQ(scene.m_meshes.size(), 2U);
    ASSERT_EQ(scene.m_instances.size(), 2U);
    for (u32 i = 0; i < 2; ++i) {
        const auto& mesh = scene.m_meshes[i];
        EXPECT_TRUE(mesh.m_mesh.validate().is_ok());
        EXPECT_EQ(mesh.m_mesh.m_indices.size(), 3U);
        ASSERT_TRUE(mesh.m_material_index.has_value());
        ASSERT_LT(mesh.m_material_index.value(), scene.m_materials.size());
        EXPECT_EQ(scene.m_instances[i].m_mesh_index, i);
        EXPECT_TRUE(scene.m_instances[i].m_transform.cmp(mat4x4::identity()));
    }
    EXPECT_NE(scene.m_meshes[0].m_material_index.value(),
              scene.m_meshes[1].m_material_index.value());
    const auto& red = scene.m_materials[scene.m_meshes[0].m_material_index.value()];
    EXPECT_EQ(red.m_name, "red");
    EXPECT_FLOAT_EQ(red.m_base_color.r, 1.0F);
    EXPECT_FLOAT_EQ(red.m_base_color.g, 0.2F);
    EXPECT_FLOAT_EQ(red.m_base_color.b, 0.3F);
    EXPECT_FLOAT_EQ(red.m_opacity, 0.5F);
    EXPECT_EQ(red.m_diffuse_texture_path,
              (std::filesystem::path(JADEFRAME_ASSET_FIXTURE_DIR) / "textures/red.png")
                  .generic_string());
    const auto& blue = scene.m_materials[scene.m_meshes[1].m_material_index.value()];
    EXPECT_EQ(blue.m_name, "blue");
    EXPECT_FLOAT_EQ(blue.m_base_color.r, 0.1F);
    EXPECT_FLOAT_EQ(blue.m_base_color.b, 1.0F);
    EXPECT_FLOAT_EQ(blue.m_opacity, 1.0F);
    EXPECT_TRUE(blue.m_diffuse_texture_path.empty());
    EXPECT_EQ(scene.m_meshes[0].m_mesh.attribute_values(Mesh::POSITION.m_id)->front(), 0.0F);
    EXPECT_EQ(scene.m_meshes[1].m_mesh.attribute_values(Mesh::POSITION.m_id)->front(), 2.0F);
}

TEST(AssetLoaderScene, PreservesInstancesAndComposesNodeTransforms) {
    const auto result = AssetLoader::load_scene(fixture("instances.gltf"));
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    const auto& scene = result.value();
    ASSERT_EQ(scene.m_meshes.size(), 2U);
    ASSERT_EQ(scene.m_instances.size(), 3U);
    EXPECT_EQ(scene.m_instances[0].m_node_name, "first");
    EXPECT_EQ(scene.m_instances[1].m_node_name, "second");
    EXPECT_EQ(scene.m_instances[2].m_node_name, "repeated");
    EXPECT_EQ(scene.m_instances[0].m_mesh_index, scene.m_instances[2].m_mesh_index);
    EXPECT_NE(scene.m_instances[0].m_mesh_index, scene.m_instances[1].m_mesh_index);

    // Parent translates by (10,0,0), rotates +90 around Z, and scales (2,3,1).
    // Its child translations must be scaled and rotated before being translated.
    const auto parent = mat4x4::from_rows(
        v4::create(0, -3, 0, 10), v4::create(2, 0, 0, 0),
        v4::create(0, 0, 1, 0), v4::create(0, 0, 0, 1));
    auto expected_first = parent;
    expected_first[3] = v4::create(10, 2, 0, 1);
    auto expected_second = parent;
    expected_second[3] = v4::create(4, 0, 0, 1);
    auto expected_repeated = parent;
    expected_repeated[3] = v4::create(10, 0, 3, 1);
    EXPECT_TRUE(scene.m_instances[0].m_transform.cmp(expected_first));
    EXPECT_TRUE(scene.m_instances[1].m_transform.cmp(expected_second));
    EXPECT_TRUE(scene.m_instances[2].m_transform.cmp(expected_repeated));
    // Shared geometry remains local, regardless of its instance transforms.
    const auto* positions = scene.m_meshes[0].m_mesh.attribute_values(Mesh::POSITION.m_id);
    ASSERT_NE(positions, nullptr);
    EXPECT_EQ(*positions, (std::vector<f32>{0, 0, 0, 1, 0, 0, 0, 1, 0}));
}

TEST(AssetLoaderScene, PreservesBaseColorAndTexture) {
    const auto result = AssetLoader::load_scene(fixture("instances.gltf"));
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    const auto& scene = result.value();
    ASSERT_TRUE(scene.m_meshes[0].m_material_index.has_value());
    const auto& material = scene.m_materials[scene.m_meshes[0].m_material_index.value()];
    EXPECT_EQ(material.m_name, "base_red");
    EXPECT_FLOAT_EQ(material.m_base_color.r, 1.0F);
    EXPECT_FLOAT_EQ(material.m_base_color.g, 0.25F);
    EXPECT_FLOAT_EQ(material.m_base_color.b, 0.5F);
    EXPECT_FLOAT_EQ(material.m_base_color.a, 0.6F);
    EXPECT_FLOAT_EQ(material.m_opacity, 0.6F);
    EXPECT_EQ(material.m_diffuse_texture_path,
              (std::filesystem::path(JADEFRAME_ASSET_FIXTURE_DIR) / "textures/red.png")
                  .generic_string());
}

TEST(AssetLoaderScene, LegacyHelperReturnsFirstMesh) {
    const auto scene = AssetLoader::load_scene(fixture("multi_mesh.obj"));
    const auto legacy = AssetLoader::load_obj(fixture("multi_mesh.obj"));
    ASSERT_TRUE(scene.is_ok()) << scene.error().m_message;
    ASSERT_TRUE(legacy.is_ok()) << legacy.error().m_message;
    EXPECT_EQ(legacy.value().m_indices, scene.value().m_meshes[0].m_mesh.m_indices);
    EXPECT_EQ(*legacy.value().attribute_values(Mesh::POSITION.m_id),
              *scene.value().m_meshes[0].m_mesh.attribute_values(Mesh::POSITION.m_id));
}

TEST_F(AssetLoaderObj, SceneWithoutExplicitMaterialsLoads) {
    const auto result = AssetLoader::load_scene(write(
        "no_material.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    ASSERT_EQ(result.value().m_meshes.size(), 1U);
    ASSERT_EQ(result.value().m_instances.size(), 1U);
    // Assimp may synthesize a default material for formats without materials.
    const auto& index = result.value().m_meshes[0].m_material_index;
    if (index.has_value()) {
        ASSERT_LT(index.value(), result.value().m_materials.size());
        const auto& material = result.value().m_materials[index.value()];
        EXPECT_FLOAT_EQ(material.m_opacity, 1.0F);
        EXPECT_TRUE(material.m_diffuse_texture_path.empty());
    }
}

TEST_F(AssetLoaderObj, ResolvesAbsoluteAndWindowsTexturePaths) {
    const auto texture = (m_dir / "absolute.png").generic_string();
    write("paths.mtl", "newmtl absolute\nmap_Kd " + texture +
          "\nnewmtl windows\nmap_Kd textures\\red.png\n");
    const auto result = AssetLoader::load_scene(write(
        "paths.obj", "mtllib paths.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\n"
        "usemtl absolute\nf 1 2 3\nusemtl windows\nf 1 3 2\n"));
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    const auto& materials = result.value().m_materials;
    const auto absolute = std::find_if(materials.begin(), materials.end(),
        [](const auto& material) { return material.m_name == "absolute"; });
    const auto windows = std::find_if(materials.begin(), materials.end(),
        [](const auto& material) { return material.m_name == "windows"; });
    ASSERT_NE(absolute, materials.end());
    ASSERT_NE(windows, materials.end());
    EXPECT_EQ(absolute->m_diffuse_texture_path, texture);
    EXPECT_EQ(windows->m_diffuse_texture_path, (m_dir / "textures/red.png").generic_string());
}

TEST_F(AssetLoaderObj, InvalidLaterMeshIsRejected) {
    const auto path = write("invalid_later.obj",
        "o triangle\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"
        "o line\nl 1 2\n");
    const auto result = AssetLoader::load_scene(path);
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, AssetErrorCode::INVALID_GEOMETRY);
    // The compatibility helper also validates the complete scene.
    expect_error(path, AssetErrorCode::INVALID_GEOMETRY);
}

TEST_F(AssetLoaderObj, InvalidMaterialReferenceIsRejected) {
    auto content = read_fixture("instances.gltf");
    const auto offset = content.find("\"material\": 0");
    ASSERT_NE(offset, std::string::npos);
    content.replace(offset, std::string("\"material\": 0").size(), "\"material\": 999");
    const auto result = AssetLoader::load_scene(write("bad_material.gltf", content));
    // Assimp rejects this invalid index before scene conversion.
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, AssetErrorCode::IMPORT_FAILED);
}

TEST_F(AssetLoaderObj, InvalidNodeMeshReferenceIsRejected) {
    auto content = read_fixture("instances.gltf");
    const auto offset = content.find("\"mesh\": 0");
    ASSERT_NE(offset, std::string::npos);
    content.replace(offset, std::string("\"mesh\": 0").size(), "\"mesh\": 999");
    const auto result = AssetLoader::load_scene(write("bad_node.gltf", content));
    ASSERT_TRUE(result.is_err());
    EXPECT_EQ(result.error().m_code, AssetErrorCode::IMPORT_FAILED);
}

TEST_F(AssetLoaderObj, EmbeddedTextureReferenceIsNotAFilePath) {
    auto content = read_fixture("instances.gltf");
    const auto offset = content.find("textures/red.png");
    ASSERT_NE(offset, std::string::npos);
    content.replace(offset, std::string("textures/red.png").size(),
        "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+a9YQAAAAASUVORK5CYII=");
    const auto result = AssetLoader::load_scene(write("embedded.gltf", content));
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    const auto& scene = result.value();
    ASSERT_TRUE(scene.m_meshes[0].m_material_index.has_value());
    EXPECT_EQ(scene.m_materials[scene.m_meshes[0].m_material_index.value()]
                  .m_diffuse_texture_path, "*0");
}

TEST_F(AssetLoaderObj, ResolvesTextureAgainstRelativeSourceDirectory) {
    const auto path = write("relative.gltf", read_fixture("instances.gltf"));
    const auto relative = std::filesystem::relative(path).generic_string();
    const auto result = AssetLoader::load_scene(relative);
    ASSERT_TRUE(result.is_ok()) << result.error().m_message;
    const auto& scene = result.value();
    ASSERT_TRUE(scene.m_meshes[0].m_material_index.has_value());
    EXPECT_EQ(scene.m_materials[scene.m_meshes[0].m_material_index.value()]
                  .m_diffuse_texture_path,
              (std::filesystem::path(relative).parent_path() / "textures/red.png")
                  .lexically_normal().generic_string());
}
