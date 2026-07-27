#pragma once

#include "graphics_shared.h"

namespace JadeFrame {

auto create_renderer(GRAPHICS_API api, RenderSystem& system, Window* window)
    -> std::unique_ptr<IRenderer>;

} // namespace JadeFrame
