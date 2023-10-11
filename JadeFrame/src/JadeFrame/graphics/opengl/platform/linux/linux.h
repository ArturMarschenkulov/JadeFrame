
#include "JadeFrame/platform/linux/linux_window.h"

namespace JadeFrame {
namespace opengl {

#undef linux
#if !defined(linux)
namespace linux {

auto load_glx_funcs(const Linux_Window* win) -> void;
auto load_opengl_funcs() -> bool;

} // namespace linux
#endif
} // namespace opengl
} // namespace JadeFrame