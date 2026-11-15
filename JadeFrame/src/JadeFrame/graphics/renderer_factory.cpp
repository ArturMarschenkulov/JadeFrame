#include "renderer_factory.h"

#if JADEFRAME_ENABLE_OPENGL
    #include "opengl/opengl_renderer.h"
#endif
#if JADEFRAME_ENABLE_VULKAN
    #include "vulkan/renderer.h"
#endif

namespace JadeFrame {

auto check_renderer_support(GRAPHICS_API api) -> Result<void, InitError> {
    switch (api) {
        case GRAPHICS_API::OPENGL:
#if JADEFRAME_ENABLE_OPENGL
            return ok<void, InitError>();
#else
            return err<void, InitError>(InitError{
                InitErrorCode::BACKEND_DISABLED,
                api,
                "the OpenGL renderer was disabled at build time "
                "(JADEFRAME_ENABLE_OPENGL=OFF)",
            });
#endif
        case GRAPHICS_API::VULKAN:
#if JADEFRAME_ENABLE_VULKAN
            return ok<void, InitError>();
#else
            return err<void, InitError>(InitError{
                InitErrorCode::BACKEND_DISABLED,
                api,
                "the Vulkan renderer was disabled at build time "
                "(JADEFRAME_ENABLE_VULKAN=OFF)",
            });
#endif
        case GRAPHICS_API::UNDEFINED:
            return err<void, InitError>(InitError{
                InitErrorCode::BACKEND_UNSUPPORTED,
                api,
                "no graphics API was selected",
            });
        case GRAPHICS_API::D3D11:
        case GRAPHICS_API::D3D12:
        case GRAPHICS_API::METAL:
        case GRAPHICS_API::SOFTWARE:
        case GRAPHICS_API::TERMINAL:
            return err<void, InitError>(InitError{
                InitErrorCode::BACKEND_UNSUPPORTED,
                api,
                std::string("JadeFrame has no renderer for the ") + to_string(api) +
                    " graphics API",
            });
    }
    // Not an enumerator. Do not hand the value to `to_string`, it is not total.
    return err<void, InitError>(InitError{
        InitErrorCode::BACKEND_UNSUPPORTED,
        GRAPHICS_API::UNDEFINED,
        "unknown graphics API value " + std::to_string(static_cast<u32>(api)),
    });
}

auto create_renderer(GRAPHICS_API api, RenderSystem& system, Window* window)
    -> Result<std::unique_ptr<IRenderer>, InitError> {
    using Created = Result<std::unique_ptr<IRenderer>, InitError>;

    auto support = check_renderer_support(api);
    if (support.is_err()) { return Created{Err, std::move(support).error()}; }

    if (window == nullptr) {
        return Created{
            Err,
            InitError{
                      InitErrorCode::INVALID_ARGUMENT,
                      api, "a renderer cannot be created without a window",
                      }
        };
    }

    switch (api) {
#if JADEFRAME_ENABLE_OPENGL
        case GRAPHICS_API::OPENGL:
            return Created{Ok, std::make_unique<OpenGL_Renderer>(system, window)};
#endif
#if JADEFRAME_ENABLE_VULKAN
        case GRAPHICS_API::VULKAN:
            return Created{Ok, std::make_unique<Vulkan_Renderer>(system, window)};
#endif
        default: break;
    }
    // `check_renderer_support` and the switch above disagree about what is compiled in.
    return Created{
        Err,
        InitError{
                  InitErrorCode::BACKEND_UNSUPPORTED,
                  api, "no renderer is registered for the requested graphics API",
                  }
    };
}

} // namespace JadeFrame
