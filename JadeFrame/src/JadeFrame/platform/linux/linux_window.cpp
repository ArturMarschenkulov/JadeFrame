#include "linux_window.h"


namespace JadeFrame {

Linux_Window::Linux_Window() {
    Status status = XInitThreads();

    m_display = XOpenDisplay(nullptr);
    int screen = DefaultScreen(display);
    Visual* visual = DefaultVisual(display, screen);
    int depth = DefaultDepth(display, screen);

    Colormap colormap = XCreateColormap(
        display, RootWindow(display, screen),
        visual, AllocNone
    );

    XSetWindowAttributes window_attributes = {};
    window_attributes.colormap = colormap;
    window_attributes.background_pixel = 0xFFFFFFFF;
    window_attributes.border_pixel = 0;
    window_attributes.event_mask =
        KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask;


    m_window = XCreateWindow(
        display, 
        RootWindow(display, screen), 
        0, 0, 
        desc.width, desc.height, 
        0, depth, InputOutput, visual, 
        CWBackPixel | CWBorderPixel | CWEventMask | CWColormap,
        &window_attributes
    );

    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    XFlush(display);
}
Linux_Window::~Linux_Window() {
    XDestroyWindow(m_display, m_window);
}
}