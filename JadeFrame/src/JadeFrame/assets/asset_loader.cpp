#include "asset_loader.h"

#include "JadeFrame/types.h"
#include "JadeFrame/utils/logger.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <system_error>
#include <utility>
#include <unordered_set>

namespace JadeFrame {

auto to_string(const AssetErrorCode code) -> const char* {
    switch (code) {
        case AssetErrorCode::FILE_NOT_FOUND: return "FILE_NOT_FOUND";
        case AssetErrorCode::IMPORT_FAILED: return "IMPORT_FAILED";
        case AssetErrorCode::EMPTY_SCENE: return "EMPTY_SCENE";
        case AssetErrorCode::INVALID_GEOMETRY: return "INVALID_GEOMETRY";
    }
    return "UNKNOWN";
}

namespace {

template<typename T>
auto asset_error(AssetErrorCode code, const std::string& path, std::string message)
    -> Result<T, AssetError> {
    return err<T, AssetError>(AssetError{code, path, std::move(message)});
}

// Converts one imported mesh. Everything Assimp hands out is checked before it is
// read, a post-process step that did not run must not turn into an out-of-bounds read.
auto convert_mesh(const aiMesh& mesh, const std::string& path)
    -> Result<Mesh, AssetError> {
    const auto invalid = [&](std::string message) {
        return asset_error<Mesh>(AssetErrorCode::INVALID_GEOMETRY, path, std::move(message));
    };

    const u32 num_vertices = mesh.mNumVertices;
    if (num_vertices == 0 || mesh.mVertices == nullptr) {
        return invalid(std::format("Mesh in '{}' has no vertex positions", path));
    }
    if (mesh.mNumFaces > 0 && mesh.mFaces == nullptr) {
        return invalid(std::format("Mesh in '{}' has faces without face data", path));
    }

    const bool has_normals = mesh.HasNormals();
    const bool has_uvs = mesh.HasTextureCoords(0);
    const bool has_colors = mesh.HasVertexColors(0);

    std::vector<f32> vertices;
    std::vector<f32> normals;
    std::vector<f32> uvs;
    std::vector<f32> colors;
    std::vector<u32> indices;
    vertices.reserve(static_cast<size_t>(num_vertices) * 3);
    if (has_normals) { normals.reserve(static_cast<size_t>(num_vertices) * 3); }
    if (has_uvs) { uvs.reserve(static_cast<size_t>(num_vertices) * 2); }
    if (has_colors) { colors.reserve(static_cast<size_t>(num_vertices) * 4); }
    indices.reserve(static_cast<size_t>(mesh.mNumFaces) * 3);

    for (u32 v = 0; v < num_vertices; v++) {
        const aiVector3D& pos = mesh.mVertices[v];
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);

        if (has_normals) {
            const aiVector3D& normal = mesh.mNormals[v];
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }

        if (has_uvs) {
            const aiVector3D& uv = mesh.mTextureCoords[0][v];
            uvs.push_back(uv.x);
            uvs.push_back(uv.y);
        }

        if (has_colors) {
            const aiColor4D& color = mesh.mColors[0][v];
            colors.push_back(color.r);
            colors.push_back(color.g);
            colors.push_back(color.b);
            colors.push_back(color.a);
        }
    }

    for (u32 f = 0; f < mesh.mNumFaces; ++f) {
        const aiFace& face = mesh.mFaces[f];
        if (face.mNumIndices != 3 || face.mIndices == nullptr) {
            return invalid(std::format(
                "Face {} in '{}' has {} indices, only triangles are supported",
                f,
                path,
                face.mNumIndices
            ));
        }
        for (u32 k = 0; k < 3; ++k) {
            const u32 index = face.mIndices[k];
            if (index >= num_vertices) {
                return invalid(std::format(
                    "Face {} in '{}' refers to vertex {}, but the mesh has {} vertices",
                    f,
                    path,
                    index,
                    num_vertices
                ));
            }
            indices.push_back(index);
        }
    }

    Mesh mesh_data;
    mesh_data.m_attributes[Mesh::POSITION.m_id] =
        Mesh::AttributeData{.m_attribute = Mesh::POSITION, .m_data = std::move(vertices)};
    if (!normals.empty()) {
        mesh_data.m_attributes[Mesh::NORMAL.m_id] =
            Mesh::AttributeData{.m_attribute = Mesh::NORMAL, .m_data = std::move(normals)};
    }
    if (!uvs.empty()) {
        mesh_data.m_attributes[Mesh::UV.m_id] =
            Mesh::AttributeData{.m_attribute = Mesh::UV, .m_data = std::move(uvs)};
    }
    if (!colors.empty()) {
        mesh_data.m_attributes[Mesh::COLOR.m_id] =
            Mesh::AttributeData{.m_attribute = Mesh::COLOR, .m_data = std::move(colors)};
    }
    mesh_data.m_indices = std::move(indices);

    // The mesh leaves this function only if the rest of the engine accepts it.
    auto validation = mesh_data.validate();
    if (validation.is_err()) {
        return invalid(std::format(
            "Mesh in '{}' is not a valid mesh: {}", path, validation.error().m_message
        ));
    }

    return ok<Mesh, AssetError>(std::move(mesh_data));
}

auto convert_matrix(const aiMatrix4x4& matrix) -> mat4x4 {
    return mat4x4::from_rows(
        v4::create(matrix.a1, matrix.a2, matrix.a3, matrix.a4),
        v4::create(matrix.b1, matrix.b2, matrix.b3, matrix.b4),
        v4::create(matrix.c1, matrix.c2, matrix.c3, matrix.c4),
        v4::create(matrix.d1, matrix.d2, matrix.d3, matrix.d4)
    );
}

auto convert_material(const aiMaterial* material, const std::string& path)
    -> AssetMaterial {
    AssetMaterial result;
    if (!material) { return result; }
    aiString name;
    if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
        result.m_name = name.C_Str();
    }
    aiColor4D color;
    if (material->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS ||
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        result.m_base_color = RGBAColor::from_rgba(color.r, color.g, color.b, color.a);
    }
    material->Get(AI_MATKEY_OPACITY, result.m_opacity);
    aiString texture;
    if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texture) == AI_SUCCESS ||
        material->GetTexture(aiTextureType_DIFFUSE, 0, &texture) == AI_SUCCESS) {
        std::string texture_path = texture.C_Str();
        if (!texture_path.empty() && texture_path.front() != '*') {
            // Asset files commonly use Windows separators even on Unix hosts.
            std::replace(texture_path.begin(), texture_path.end(), '\\', '/');
            texture_path = (std::filesystem::path(path).parent_path() / texture_path)
                               .lexically_normal().generic_string();
        }
        result.m_diffuse_texture_path = std::move(texture_path);
    }
    return result;
}

} // namespace

auto AssetLoader::load_scene(const std::string& path) -> Result<AssetScene, AssetError> {
    Logger::info("Loading scene: {}", path);

    std::error_code fs_error;
    if (!std::filesystem::is_regular_file(path, fs_error)) {
        return asset_error<AssetScene>(
            AssetErrorCode::FILE_NOT_FOUND,
            path,
            fs_error ? std::format("Cannot open '{}': {}", path, fs_error.message())
                     : std::format("'{}' is not a file", path)
        );
    }

    Assimp::Importer importer;
    const u32        flags = aiProcess_Triangulate;
    const aiScene*   scene = importer.ReadFile(path, flags);
    if (scene == nullptr) {
        return asset_error<AssetScene>(
            AssetErrorCode::IMPORT_FAILED,
            path,
            std::format("Cannot import '{}': {}", path, importer.GetErrorString())
        );
    }
    // A scene without meshes is flagged as incomplete by Assimp as well, so this check
    // has to come first to keep the two cases apart.
    if (scene->mNumMeshes == 0 || scene->mMeshes == nullptr) {
        return asset_error<AssetScene>(
            AssetErrorCode::EMPTY_SCENE,
            path,
            std::format("'{}' does not contain a mesh", path)
        );
    }
    if ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U || scene->mRootNode == nullptr) {
        return asset_error<AssetScene>(
            AssetErrorCode::IMPORT_FAILED,
            path,
            std::format("'{}' was imported only partially", path)
        );
    }

    AssetScene loaded;
    loaded.m_materials.reserve(scene->mNumMaterials);
    for (u32 i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* material = scene->mMaterials ? scene->mMaterials[i] : nullptr;
        loaded.m_materials.push_back(convert_material(material, path));
    }

    loaded.m_meshes.reserve(scene->mNumMeshes);
    for (u32 i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        if (mesh == nullptr) {
            return asset_error<AssetScene>(
                AssetErrorCode::INVALID_GEOMETRY, path,
                std::format("Mesh {} of '{}' is missing", i, path)
            );
        }
        auto result = convert_mesh(*mesh, path);
        if (result.is_err()) { return err<AssetScene, AssetError>(result.error()); }
        AssetMesh converted;
        converted.m_name = mesh->mName.C_Str();
        converted.m_mesh = std::move(result.value());
        if (mesh->mMaterialIndex < scene->mNumMaterials && scene->mMaterials &&
            scene->mMaterials[mesh->mMaterialIndex]) {
            converted.m_material_index = some<u32>(mesh->mMaterialIndex);
        }
        loaded.m_meshes.push_back(std::move(converted));
    }

    // Iterative traversal avoids using the C++ call stack for deep hierarchies.
    struct PendingNode {
        const aiNode* m_node;
        mat4x4 m_parent_transform;
    };
    std::vector<PendingNode> pending{{scene->mRootNode, mat4x4::identity()}};
    std::unordered_set<const aiNode*> visited;
    while (!pending.empty()) {
        const auto [node, parent_transform] = pending.back();
        pending.pop_back();
        if (!node || !visited.insert(node).second ||
            (node->mNumMeshes && !node->mMeshes) ||
            (node->mNumChildren && !node->mChildren)) {
            return asset_error<AssetScene>(
                AssetErrorCode::IMPORT_FAILED, path, "Invalid scene node hierarchy"
            );
        }
        const mat4x4 transform = parent_transform * convert_matrix(node->mTransformation);
        for (u32 i = 0; i < node->mNumMeshes; ++i) {
            const u32 index = node->mMeshes[i];
            if (index >= loaded.m_meshes.size()) {
                return asset_error<AssetScene>(
                    AssetErrorCode::INVALID_GEOMETRY, path,
                    std::format("Node '{}' refers to missing mesh {}", node->mName.C_Str(), index)
                );
            }
            loaded.m_instances.push_back({node->mName.C_Str(), index, transform});
        }
        // Reverse push retains source node order in a depth-first traversal.
        for (u32 i = node->mNumChildren; i > 0; --i) {
            pending.push_back({node->mChildren[i - 1], transform});
        }
    }
    return ok<AssetScene, AssetError>(std::move(loaded));
}

auto AssetLoader::load_obj(const std::string& path) -> Result<Mesh, AssetError> {
    auto result = load_scene(path);
    if (result.is_err()) { return err<Mesh, AssetError>(result.error()); }
    return ok<Mesh, AssetError>(std::move(result.value().m_meshes.front().m_mesh));
}

} // namespace JadeFrame
