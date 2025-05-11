#include "asset_loader.h"
#include "JadeFrame/types.h"
#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/logger.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace JadeFrame {
auto AssetLoader::load_obj(const std::string& path) -> Mesh {
    // assert(false && "Not implemented");
    Assimp::Importer importer;
    auto             flags = aiProcess_Triangulate | aiProcess_GenUVCoords;
    const aiScene*   scene = importer.ReadFile(path, flags);
    if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) ||
        (scene->mRootNode == nullptr)) {
        Logger::err("Assimp error: {}", importer.GetErrorString());
        JF_ASSERT(false, "Assimp error");
    }

    if (scene->mNumMeshes == 0) {
        Logger::err("No meshes found in file: {}", path);
        JF_ASSERT(false, "No meshes found in file");
    } else if (scene->mNumMeshes == 1) {
        Logger::warn("Found 1 mesh in file: {}", path);
    } else {
        Logger::info("Found {} meshes in file: {}", scene->mNumMeshes, path);
    }

    for (u32 m = 0; m < scene->mNumMeshes; m++) {
        const aiMesh*    mesh = scene->mMeshes[m];
        std::vector<f32> vertices;
        std::vector<f32> normals;
        std::vector<f32> uvs;
        std::vector<f32> colors;
        std::vector<u32> indices;
        for (u32 v = 0; v < mesh->mNumVertices; v++) {
            aiVector3D pos = mesh->mVertices[v];
            vertices.push_back(pos.x);
            vertices.push_back(pos.y);
            vertices.push_back(pos.z);

            if (mesh->HasNormals()) {
                aiVector3D normal = mesh->mNormals[v];
                normals.push_back(normal.x);
                normals.push_back(normal.y);
                normals.push_back(normal.z);
            }

            if (mesh->HasTextureCoords(0)) {
                aiVector3D uv = mesh->mTextureCoords[0][v];
                uvs.push_back(uv.x);
                uvs.push_back(uv.y);
            }

            if (mesh->HasVertexColors(0)) {
                aiColor4D color = mesh->mColors[0][v];
                colors.push_back(color.r);
                colors.push_back(color.g);
                colors.push_back(color.b);
                colors.push_back(color.a);
            }
        }

        // Iterate over the faces of the mesh
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
            // Get the face
            aiFace face = mesh->mFaces[j];
            // Add the indices of the face to the vector
            for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                indices.push_back(face.mIndices[k]);
            }
        }
        Logger::info("num pos: {}", vertices.size() / 3);
        Logger::info("num normals: {}", normals.size() / 3);
        Logger::info("num uvs: {}", uvs.size() / 2);
        Logger::info("num colors: {}", colors.size() / 4);

        Mesh mesh_data;
        mesh_data.m_attributes[Mesh::POSITION.m_id] =
            Mesh::AttributeData{Mesh::POSITION, vertices};
        // if (!normals.empty()) {
        //     mesh_data.m_attributes[Mesh::NORMAL.m_id] =
        //         Mesh::AttributeData{Mesh::NORMAL, normals};
        // }
        if (!uvs.empty()) {
            mesh_data.m_attributes[Mesh::UV.m_id] = Mesh::AttributeData{Mesh::UV, uvs};
        }
        if (!colors.empty()) {
            mesh_data.m_attributes[Mesh::COLOR.m_id] =
                Mesh::AttributeData{Mesh::COLOR, colors};
        }
        if (!indices.empty()) { mesh_data.m_indices = indices; }

        return mesh_data;

        break;
    }
    return {};
}
} // namespace JadeFrame