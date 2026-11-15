#pragma once

#include "graphics_shared.h"

namespace JadeFrame {

/// Whether a renderer for `api` is compiled into this build.
///
/// Fails with `BACKEND_UNSUPPORTED` for APIs JadeFrame has no renderer for and with
/// `BACKEND_DISABLED` for a renderer that was switched off at build time.
[[nodiscard]] auto check_renderer_support(GRAPHICS_API api) -> Result<void, InitError>;

/// Selects and constructs the renderer for exactly `api`; there is no fallback to
/// another backend.
///
/// The returned renderer is never null and not yet initialized: constructing a
/// renderer does not touch the graphics API, `IRenderer::initialize()` does. Fails with
/// the errors of `check_renderer_support()` or with `INVALID_ARGUMENT` if `window` is
/// null.
[[nodiscard]] auto create_renderer(GRAPHICS_API api, RenderSystem& system, Window* window)
    -> Result<std::unique_ptr<IRenderer>, InitError>;

} // namespace JadeFrame
