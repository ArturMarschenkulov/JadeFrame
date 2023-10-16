#include "linux_window.h"
#include <GL/gl.h>
#include <GL/glx.h>

namespace JadeFrame {

Linux_Window::Linux_Window(const Linux_Window::Desc& desc) {
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        printf("\n\tcannot connect to X server\n\n");
        exit(0);
    }

    XID root = DefaultRootWindow(m_display);

    GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

    m_visual_info = glXChooseVisual(m_display, 0, att);
    int     screen = DefaultScreen(m_display);
    Visual* visual = DefaultVisual(m_display, screen);
    int     depth = DefaultDepth(m_display, screen);

    Colormap colormap =
        XCreateColormap(m_display, RootWindow(m_display, screen), visual, AllocNone);

    XSetWindowAttributes window_attributes = {};
    window_attributes.colormap = colormap;
    window_attributes.background_pixel = 0xFFFFFFFF;
    window_attributes.border_pixel = 0;
    window_attributes.event_mask =
        KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask;

    m_window = XCreateWindow(
        m_display,
        RootWindow(m_display, screen),
        0,
        0,
        desc.size.x,
        desc.size.y,
        0,
        depth,
        InputOutput,
        visual,
        CWBackPixel | CWBorderPixel | CWEventMask | CWColormap,
        &window_attributes
    );
    XSelectInput(m_display, m_window, ExposureMask | KeyPressMask);
    XMapWindow(m_display, m_window);
    XFlush(m_display);

    // Client area dimensions
    XWindowAttributes client_attributes;
    XGetWindowAttributes(m_display, m_window, &client_attributes);
    int client_width = client_attributes.width;
    int client_height = client_attributes.height;



    // Window dimensions
    ::Window     root_window;
    int          x, y;
    unsigned int border_width;
    unsigned int depth_;
    unsigned int window_width;
    unsigned int window_height;
    XGetGeometry(
        m_display,
        m_window,
        &root_window,
        &x,
        &y,
        &window_width,
        &window_height,
        &border_width,
        &depth_
    );

    // m_title = desc.title;
    m_size = v2u32(window_width, window_height);
    // m_position = v2u32(window_rect.left, window_rect.top);

}

Linux_Window::~Linux_Window() { XDestroyWindow(m_display, m_window); }
} // namespace JadeFrame