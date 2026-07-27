#include "renderer_factory.h"

#if JADEFRAME_ENABLE_OPENGL
    #include "opengl/opengl_renderer.h"
#endif
#if JADEFRAME_ENABLE_VULKAN
    #include "vulkan/renderer.h"
#endif

namespace JadeFrame {

auto create_renderer(GRAPHICS_API api, RenderSystem& system, Window* window)
    -> std::unique_ptr<IRenderer> {
    switch (api) {
#if JADEFRAME_ENABLE_OPENGL
        case GRAPHICS_API::OPENGL:
            return std::make_unique<OpenGL_Renderer>(system, window);
#endif
#if JADEFRAME_ENABLE_VULKAN
        case GRAPHICS_API::VULKAN:
            return std::make_unique<Vulkan_Renderer>(system, window);
#endif
        default: return nullptr;
    }
}

} // namespace JadeFrame
