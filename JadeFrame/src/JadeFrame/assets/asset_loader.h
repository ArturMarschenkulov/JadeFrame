#pragma once

#include <string>

#include "JadeFrame/graphics/mesh.h"
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

class AssetLoader {
public:
    /// Loads the first mesh of an OBJ file.
    ///
    /// Failures are returned and not logged, reporting them is up to the caller.
    [[nodiscard]] static auto load_obj(const std::string& path)
        -> Result<Mesh, AssetError>;
};

} // namespace JadeFrame
