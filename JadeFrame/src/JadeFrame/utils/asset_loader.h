#pragma once
#include "JadeFrame/graphics/mesh.h"

namespace JadeFrame {
class AssetLoader {
public:
    static auto load_obj(const std::string& path) -> Mesh;
};
} // namespace JadeFrame