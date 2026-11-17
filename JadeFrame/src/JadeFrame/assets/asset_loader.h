#pragma once

#include <string>
#include <vector>

#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/utils/option.h"
#include "JadeFrame/utils/result.h"

namespace JadeFrame {

enum class AssetErrorCode : u8 {
    /// The path does not name a readable regular file.
    FILE_NOT_FOUND,
    /// The importer could not parse the file.
    IMPORT_FAILED,
    /// The file was imported, but it contains no mesh.
    EMPTY_SCENE,
    /// The imported mesh cannot be represented as an indexed triangle mesh.
    INVALID_GEOMETRY,
};

struct AssetError {
    AssetErrorCode m_code;
    /// The path that was passed to the loader.
    std::string m_path;
    /// Human readable description, including the importer's own diagnostic if any.
    std::string m_message;
};

[[nodiscard]] auto to_string(AssetErrorCode code) -> const char*;

/// CPU metadata only. Missing properties default to white, opaque, and no texture.
struct AssetMaterial {
    std::string m_name;
    RGBAColor m_base_color = RGBAColor::solid_white();
    f32 m_opacity = 1.0F;
    /// First base-color/diffuse texture. External paths are resolved against the
    /// asset directory (lexically normalized); embedded references like *0 stay
    /// unchanged. Empty means absent. No image data is loaded or retained.
    std::string m_diffuse_texture_path;
};

struct AssetMesh {
    std::string m_name;
    Mesh m_mesh;
    /// Index into AssetScene::m_materials; absent for missing/invalid materials.
    Option<u32> m_material_index;
};

struct AssetMeshInstance {
    std::string m_node_name;
    u32 m_mesh_index = 0;
    /// Mesh-local to scene-space, including all ancestor transformations.
    mat4x4 m_transform = mat4x4::identity();
};

/// Meshes remain in local space and are stored once, in source mesh order.
/// Instances flatten the node hierarchy and preserve every mesh reference.
struct AssetScene {
    std::vector<AssetMesh> m_meshes;
    std::vector<AssetMaterial> m_materials;
    std::vector<AssetMeshInstance> m_instances;
};

class AssetLoader {
public:
    /// Imports all meshes, materials, and instances of an Assimp-supported file.
    /// Failures are returned and not logged, reporting them is up to the caller.
    [[nodiscard]] static auto load_scene(const std::string& path)
        -> Result<AssetScene, AssetError>;

    /// Compatibility helper: imports the scene and returns its first local-space
    /// mesh, discarding materials and instances. Use load_scene for complete assets.
    [[nodiscard]] static auto load_obj(const std::string& path)
        -> Result<Mesh, AssetError>;
};

} // namespace JadeFrame
