#include "asset_loader.h"

#include "JadeFrame/types.h"
#include "JadeFrame/utils/logger.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <format>
#include <system_error>
#include <utility>

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

auto asset_error(AssetErrorCode code, const std::string& path, std::string message)
    -> Result<Mesh, AssetError> {
    return err<Mesh, AssetError>(AssetError{code, path, std::move(message)});
}

// Converts one imported mesh. Everything Assimp hands out is checked before it is
// read, a post-process step that did not run must not turn into an out-of-bounds read.
auto convert_mesh(const aiMesh& mesh, const std::string& path)
    -> Result<Mesh, AssetError> {
    const auto invalid = [&](std::string message) {
        return asset_error(AssetErrorCode::INVALID_GEOMETRY, path, std::move(message));
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

} // namespace

auto AssetLoader::load_obj(const std::string& path) -> Result<Mesh, AssetError> {
    Logger::info("Loading OBJ file: {}", path);

    std::error_code fs_error;
    if (!std::filesystem::is_regular_file(path, fs_error)) {
        return asset_error(
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
        return asset_error(
            AssetErrorCode::IMPORT_FAILED,
            path,
            std::format("Cannot import '{}': {}", path, importer.GetErrorString())
        );
    }
    // A scene without meshes is flagged as incomplete by Assimp as well, so this check
    // has to come first to keep the two cases apart.
    if (scene->mNumMeshes == 0 || scene->mMeshes == nullptr) {
        return asset_error(
            AssetErrorCode::EMPTY_SCENE,
            path,
            std::format("'{}' does not contain a mesh", path)
        );
    }
    if ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U || scene->mRootNode == nullptr) {
        return asset_error(
            AssetErrorCode::IMPORT_FAILED,
            path,
            std::format("'{}' was imported only partially", path)
        );
    }

    if (scene->mNumMeshes > 1) {
        Logger::warn(
            "'{}' contains {} meshes, only the first one is loaded. Multi-mesh support "
            "is not implemented yet.",
            path,
            scene->mNumMeshes
        );
    }

    const aiMesh* mesh = scene->mMeshes[0];
    if (mesh == nullptr) {
        return asset_error(
            AssetErrorCode::INVALID_GEOMETRY,
            path,
            std::format("The first mesh of '{}' is missing", path)
        );
    }

    auto result = convert_mesh(*mesh, path);
    if (result.is_ok()) {
        const Mesh& loaded = result.value();
        Logger::info(
            "Loaded '{}': {} vertices, {} indices",
            path,
            mesh->mNumVertices,
            loaded.m_indices.size()
        );
    }
    return result;
}

} // namespace JadeFrame
