
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/platform/linux/linux_window.h"
#include <GL/glx.h>

namespace JadeFrame {
namespace opengl {

#undef linux
#if !defined(linux)
namespace linux {

/// Creates an OpenGL 4.6 core context for `win` and makes it current.
///
/// On failure nothing is left behind: a context that was created but could not be made
/// current is destroyed again. On success the caller owns the context.
[[nodiscard]] auto create_glx_context(const X11_NativeWindow* win)
    -> Result<GLXContext, InitError>;
/// Loads the OpenGL function pointers. Needs a current context. Returns false if the
/// functions could not be loaded; no OpenGL function may be called in that case.
[[nodiscard]] auto load_opengl_funcs() -> bool;

} // namespace linux
#endif
} // namespace opengl
} // namespace JadeFrame
