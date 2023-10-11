#include "JadeFrame/prelude.h"
#include "linux.h"
#include <glad/glad.h>
#include <GL/glx.h>

namespace JadeFrame {
namespace opengl {
namespace linux {

auto load_glx_funcs(const Linux_Window* win) -> void {
    // glXGetProcAddress();
    GLXContext render_context = glXCreateContext(win->m_display, win->m_visual_info, nullptr, GL_TRUE);
    glXMakeCurrent(win->m_display, win->m_window, render_context);
}
auto load_opengl_funcs() -> bool {
    i32 result = gladLoadGL();
    if (result != 1) { Logger::err("gladLoadGL() failed."); }
    return true;
}


} // namespace linux
} // namespace opengl
} // namespace JadeFrame