
#include "JadeFrame/platform/linux/linux_window.h"
#include <GL/glx.h>

namespace JadeFrame {
namespace opengl {

#undef linux
#if !defined(linux)
namespace linux {

auto load_glx_funcs(const X11_NativeWindow* win) -> void;
auto load_opengl_funcs() -> bool;

} // namespace linux
#endif
} // namespace opengl
} // namespace JadeFrame