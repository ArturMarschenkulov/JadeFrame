#include <glad/glad.h>
// #include <GL/glx.h>

#include "JadeFrame/prelude.h"
#include "linux.h"
#include <iostream>
#include <vector>
#include <map>

using PFN_glXCreateContextAttribsARBProc =
    GLXContext (*)(Display*, GLXFBConfig, GLXContext, int, const int*);
using PFN_glXChooseFBConfigProc = GLXFBConfig* (*)(Display*, int, const int*, int*);
using PFN_glXGetVisualFromFBConfigProc = XVisualInfo* (*)(Display*, GLXFBConfig);

struct OGL_Funcs {
    PFN_glXCreateContextAttribsARBProc glXCreateContextAttribsARB;
};

static OGL_Funcs gl;

namespace JadeFrame {
namespace opengl {
namespace linux {
static auto get_proc(const char* name) -> void* {
    void* proc = (void*)glXGetProcAddressARB((const GLubyte*)name);
    if (proc == nullptr) { Logger::err("Failed to load function: {}", name); }
    return proc;
}

static auto create_render_context(Display* display) -> GLXContext {
    constexpr bool extended_context = true;
    GLXFBConfig*   framebuffer_config = nullptr;
    if (extended_context) {
        const int major_min = 4;
        const int minor_min = 6;

        std::vector<int> context_attributes;
        context_attributes.push_back(GLX_CONTEXT_MAJOR_VERSION_ARB); // major version
        context_attributes.push_back(major_min);
        context_attributes.push_back(GLX_CONTEXT_MINOR_VERSION_ARB); // minor version
        context_attributes.push_back(minor_min);
        context_attributes.push_back(GLX_CONTEXT_FLAGS_ARB); // context flags
        context_attributes.push_back(
            GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | GLX_CONTEXT_DEBUG_BIT_ARB
        );
        context_attributes.push_back(GLX_CONTEXT_PROFILE_MASK_ARB); // profile type
        context_attributes.push_back(GLX_CONTEXT_CORE_PROFILE_BIT_ARB);
        context_attributes.push_back(0); // End

        int          fb_count = 0;
        GLXFBConfig* fb_config =
            glXChooseFBConfig(display, DefaultScreen(display), nullptr, &fb_count);
        if (fb_config == nullptr) {
            std::cout << "glXChooseFBConfig() failed." << std::endl;
            return nullptr;
        }
        GLXContext render_context = gl.glXCreateContextAttribsARB(
            display, *fb_config, nullptr, True, context_attributes.data()
        );
        if (render_context == nullptr) {
            std::cout << "glXCreateContextAttribsARB() failed." << std::endl;
            return nullptr;
        }
        return render_context;
    } else {
        // int          fb_count = 0;
        // GLXFBConfig* fb_config =
        //     glXChooseFBConfig(display, DefaultScreen(display), nullptr, &fb_count);
        // GLXContext render_context = glXCreateContext(
        //     display, glXGetVisualFromFBConfig(display, fb_config), NULL, GL_TRUE
        // );
        // if (render_context == nullptr) {
        //     std::cerr << "glXCreateContext() failed." << std::endl;
        //     return nullptr;
        // }
        GLXContext render_context;
        return render_context;
    }
}

static auto init_render_context(Display* display) -> GLXContext {
    return create_render_context(display);
}

static auto get_proc_address_glx_funcs() -> void {
    gl.glXCreateContextAttribsARB =
        (PFN_glXCreateContextAttribsARBProc)get_proc("glXCreateContextAttribsARB");
}

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static auto is_extension_supported(const char* extList, const char* extension) -> bool {
    const char* start = nullptr;
    const char* where = nullptr;
    const char* terminator = nullptr;

    /* Extension names should not have spaces. */
    where = strchr(extension, ' ');
    if ((where != nullptr) || *extension == '\0') { return false; }

    /* It takes a bit of care to be fool-proof about parsing the
       OpenGL extensions string. Don't be fooled by sub-strings,
       etc. */
    for (start = extList;;) {
        where = strstr(start, extension);

        if (where == nullptr) { break; }

        terminator = where + strlen(extension);

        if (where == start || *(where - 1) == ' ') {
            if (*terminator == ' ' || *terminator == '\0') { return true; }
        }

        start = terminator;
    }

    return false;
}

static bool g_ctx_error_occurred = false;

static auto get_best_fb_config(Display* display, const std::vector<int>& visual_attribs)
    -> GLXFBConfig {
#if 0
    int          num_fbc = 0;
    GLXFBConfig* fb_configs = glXChooseFBConfig(
        display, DefaultScreen(display), visual_attribs.data(), &num_fbc
    );
    if (fb_configs == nullptr) {
        Logger::err("Failed to retrieve framebuffer configurations");
    }

#else
    int          fb_count = 0;
    GLXFBConfig* fb_configs = glXChooseFBConfig(
        display, DefaultScreen(display), visual_attribs.data(), &fb_count
    );
    if (fb_configs == nullptr) {
        Logger::err("Failed to retrieve framebuffer configurations");
    }

    int best_index = -1;
    int worst_fbc = -1;
    int best_num_samp = -1;
    int worst_num_samp = 999;

    for (int i = 0; i < fb_count; ++i) {
        XVisualInfo* vi = glXGetVisualFromFBConfig(display, fb_configs[i]);
        if (vi != nullptr) {
            int samp_buf = 0;
            int samples = 0;
            glXGetFBConfigAttrib(display, fb_configs[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(display, fb_configs[i], GLX_SAMPLES, &samples);

            Logger::debug(
                "  Matching fbconfig {}, visual ID {}: SAMPLE_BUFFERS = {}, SAMPLES = {}",
                i,
                vi->visualid,
                samp_buf,
                samples
            );

            if (best_index < 0 || (samp_buf != 0) && samples > best_num_samp) {
                best_index = i;
                best_num_samp = samples;
            }
            if (worst_fbc < 0 || (samp_buf == 0) || samples < worst_num_samp) {
                worst_fbc = i;
                worst_num_samp = samples;
            }
        }
        XFree(vi);
    }
    if (best_index < 0) {
        Logger::warn("Failed to find a suitable framebuffer configuration, simply "
                     "using first one.");
        best_index = 0;
    }
    Logger::debug("Best config is {}: {}", best_index, best_num_samp);
    GLXFBConfig best_fbc = fb_configs[best_index];
    XFree(fb_configs);
    return best_fbc;
#endif
}

static auto ctx_error_handler(Display* dpy, XErrorEvent* ev) -> int {
    // char error_text[1024];
    // XGetErrorText(dpy, ev->error_code, error_text, 1024);
    // Logger::err("X error: {}", error_text);
    // Logger::err("Request code: {}, Resource ID: {}, Serial Number: {}",
    // ev->request_code, ev->resourceid, ev->serial);

    g_ctx_error_occurred = true;
    return 0;
}

static auto map_to_array(const std::map<int, int>& map) -> std::vector<int> {
    std::vector<int> array;
    for (const auto& [key, value] : map) {
        array.push_back(key);
        array.push_back(value);
    }
    array.push_back(None);
    return array;
}

auto load_glx_funcs(const X11_NativeWindow* window) -> void {

    int glx_major = 0;
    int glx_minor = 0;
    if (glXQueryVersion(window->m_display, &glx_major, &glx_minor) == 0) {
        Logger::err("Failed to query GLX version.");
    }
    Logger::info("GLX version: {}.{}", glx_major, glx_minor);

    ::Display* display = window->m_display;

    std::map<int, int> visual_attrib_map = {
        { GLX_X_RENDERABLE,           True},

        {GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT},
        {  GLX_RENDER_TYPE,   GLX_RGBA_BIT},
        {GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR},
        {     GLX_RED_SIZE,              8},
        {   GLX_GREEN_SIZE,              8},
        {    GLX_BLUE_SIZE,              8},
        {   GLX_ALPHA_SIZE,              8},
        {   GLX_DEPTH_SIZE,             24},
        { GLX_STENCIL_SIZE,              8},
        { GLX_DOUBLEBUFFER,           True}
    };
    // convert this to a vector
    std::vector<int> visual_attribs = map_to_array(visual_attrib_map);

    GLXFBConfig best_fbc = get_best_fb_config(display, visual_attribs);

    // XVisualInfo* vi = glXGetVisualFromFBConfig(display, best_fbc);
    // Logger::debug("Chosen visual ID = {}", vi->visualid);
    // XSetWindowAttributes swa;
    // swa.colormap =
    //     XCreateColormap(display, RootWindow(display, vi->screen), vi->visual,
    //     AllocNone);
    // swa.background_pixmap = None;
    // swa.border_pixel = 0;
    // swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    // ::Window win = XCreateWindow(
    //     display,
    //     RootWindow(display, vi->screen),
    //     0,
    //     0,
    //     1,
    //     1,
    //     0,
    //     vi->depth,
    //     InputOutput,
    //     vi->visual,
    //     CWBorderPixel | CWColormap | CWEventMask,
    //     &swa
    // );

    // if (win == 0U) {
    //     Logger::err("Failed to create window.");
    //     std::exit(1);
    // }
    // XFree(vi);

    const char* glxExts = glXQueryExtensionsString(display, DefaultScreen(display));
    bool        is_glx_arb_create_context_supported =
        is_extension_supported(glxExts, "GLX_ARB_create_context");
    if (!is_glx_arb_create_context_supported) {
        Logger::err("GLX_ARB_create_context not supported.");
        std::exit(1);
    }
    get_proc_address_glx_funcs();
    g_ctx_error_occurred = false;
    int (*old_handler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctx_error_handler);

    auto create_context = [](Display* display, GLXFBConfig best_fbc) -> GLXContext {
        const int          major_min = 4;
        const int          minor_min = 6;
        std::map<int, int> context_attribs_map = {
            {GLX_CONTEXT_MAJOR_VERSION_ARB,major_min                                           },
            {GLX_CONTEXT_MINOR_VERSION_ARB,                           minor_min},
            {        GLX_CONTEXT_FLAGS_ARB,
             GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | GLX_CONTEXT_DEBUG_BIT_ARB},
            { GLX_CONTEXT_PROFILE_MASK_ARB,    GLX_CONTEXT_CORE_PROFILE_BIT_ARB},
        };
        std::vector<int> context_attribs = map_to_array(context_attribs_map);

        GLXContext ctx = gl.glXCreateContextAttribsARB(
            display, best_fbc, nullptr, True, context_attribs.data()
        );
        XSync(display, False);
        if (ctx == nullptr) { Logger::err("Failed to create GL context."); }
        return ctx;
    };

    GLXContext ctx = create_context(display, best_fbc);
    glXMakeCurrent(display, window->m_window, ctx);

    // if (win != 0) { XDestroyWindow(display, win); }
}

auto load_opengl_funcs() -> bool {
    i32 result = gladLoadGL();
    if (result != 1) { Logger::err("gladLoadGL() failed."); }
    return true;
}

} // namespace linux
} // namespace opengl
} // namespace JadeFrame