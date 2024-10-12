#include "asset_loader.h"
// #include "JadeFrame/utils/logger.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace JadeFrame {
auto AssetLoader::load_obj(const char* path) -> void {
    // assert(false && "Not implemented");
    Assimp::Importer importer;
    const aiScene*   scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals
    );
}
} // namespace JadeFrame