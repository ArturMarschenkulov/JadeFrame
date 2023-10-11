#include "linux_window.h"
#include <GL/gl.h>
#include <GL/glx.h>


namespace JadeFrame {

Linux_Window::Linux_Window(const Linux_Window::Desc& desc) {
    Status status = XInitThreads();

    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        printf("\n\tcannot connect to X server\n\n");
        exit(0);
    }

    XID root = DefaultRootWindow(m_display);

    GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

    m_visual_info = glXChooseVisual(m_display, 0, att);
    int          screen = DefaultScreen(m_display);
    Visual*      visual = DefaultVisual(m_display, screen);
    int          depth = DefaultDepth(m_display, screen);

    Colormap colormap = XCreateColormap(m_display, RootWindow(m_display, screen), visual, AllocNone);

    XSetWindowAttributes window_attributes = {};
    window_attributes.colormap = colormap;
    window_attributes.background_pixel = 0xFFFFFFFF;
    window_attributes.border_pixel = 0;
    window_attributes.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask;
    // glXCreateContext(m_display, visual, nullptr, GL_TRUE);

    m_window = XCreateWindow(
        m_display, RootWindow(m_display, screen), 0, 0, 500, 500, 0, depth, InputOutput, visual,
        CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &window_attributes);

    XSelectInput(m_display, m_window, ExposureMask | KeyPressMask);
    XMapWindow(m_display, m_window);
    XFlush(m_display);
}
Linux_Window::~Linux_Window() { XDestroyWindow(m_display, m_window); }
} // namespace JadeFrame