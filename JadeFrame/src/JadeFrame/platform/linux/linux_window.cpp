#include "linux_window.h"
#include <GL/gl.h>
#include <GL/glx.h>

#include <X11/Xlib.h>
#include <string>
#include <iostream>
#include <cstdlib>

namespace JadeFrame {

Linux_Window::Linux_Window(const Window::Desc& desc) {

    const char* x11_display_env = std::getenv("DISPLAY");
    if (x11_display_env == nullptr) {
        printf("The DISPLAY environment variable is not set. This is required for X11.\n"
        );
        exit(1);
    }

    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        printf("\n\tcannot connect to X server\n\n");
        exit(0);
    }

    XID root = DefaultRootWindow(m_display);
    (void)root;

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
    // int client_width = client_attributes.width;
    // int client_height = client_attributes.height;

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

static auto process_event(XEvent* event) -> void {
    switch (event->type) {
        case Expose: {

        } break;
        case KeyPress: {

        } break;
        case KeyRelease: {
        } break;
        case ConfigureNotify: {

        } break;
        default: break;
    }
}

auto Linux_Window::handle_events(bool&) -> void {

    XPending(m_display);

    while (XQLength(m_display)) {
        XEvent event;
        XNextEvent(m_display, &event);
        process_event(&event);
    }
}
} // namespace JadeFrame