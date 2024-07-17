#include "linux_window.h"
#include "JadeFrame/platform/linux/linux_input_manager.h"
#include "JadeFrame/platform/window_event.h"
#include <GL/gl.h>
#include <GL/glx.h>

#include <X11/Xlib.h>
#include <string>
#include <iostream>
#include <cstdlib>

namespace JadeFrame {

X11_NativeWindow::X11_NativeWindow(const Window::Desc& desc) {

    const char* x11_display_env = std::getenv("DISPLAY");
    if (x11_display_env == nullptr) {
        printf("The DISPLAY environment variable is not set. This is required for X11.\n"
        );
        std::exit(1);
    }

    m_display = XOpenDisplay(nullptr);
    if (m_display == nullptr) {
        printf("\n\tcannot connect to X server\n\n");
        std::exit(0);
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

    m_window = ::XCreateWindow(
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
    int          x;
    int          y;
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

    ::XIM xim = XOpenIM(m_display, nullptr, nullptr, nullptr);
    if (xim == nullptr) {
        // fallback to internal input method
        XSetLocaleModifiers("@im=none");
        xim = XOpenIM(m_display, nullptr, nullptr, nullptr);
    }
    ::XIC xic = ::XCreateIC(
        xim,
        XNInputStyle,
        XIMPreeditNothing | XIMStatusNothing,
        XNClientWindow,
        m_window,
        XNFocusWindow,
        m_window,
        nullptr
    );

    XSetICFocus(xic);

    XSelectInput(
        m_display,
        m_window,
        KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask
    );
}

X11_NativeWindow::~X11_NativeWindow() { XDestroyWindow(m_display, m_window); }

static auto XCreateWindow(XEvent* event) -> bool {
    return event->type == KeyPress || event->type == KeyRelease;
}

static auto to_key_event_type(const int type) -> KeyEvent::TYPE {
    switch (type) {
        case KeyPress: return KeyEvent::TYPE::PRESSED;
        case KeyRelease: return KeyEvent::TYPE::RELEASED;
        default: assert(false && "Invalid key event type");
    }
}

static auto process_event(XEvent* event, X11_NativeWindow* win, WindowEventQueue* event_queu)
    -> void {

    switch (event->type) {
        case Expose: {

        } break;
        case KeyPress:
        case KeyRelease: {
            unsigned int keycode = event->xkey.keycode;

            KeySym keysym = {};
            XLookupString(&event->xkey, nullptr, 0, &keysym, nullptr);

            // make sure it is the latin-1 alphabet
            bool is_latin_1 = false;
            if ((keysym >= 0x0020 && keysym <= 0x007e) ||
                (keysym >= 0x00a0 && keysym <= 0x00ff)) {
                is_latin_1 = true;
            }
            assert(is_latin_1 && "Only latin-1 alphabet is supported");

            KEY            key = translate_key(keysym);
            KeyEvent::TYPE keyevent_type = to_key_event_type(event->type);

            KeyEvent key_event = {
                .type = keyevent_type,
                .key = key,
            };
            WindowEvent we = {
                .type = WindowEvent::TYPE::KEY,
                .key_event = key_event,
            };

            event_queu->push(we);

        } break;
        case ConfigureNotify: {

        } break;
        default: break;
    }
}

auto X11_NativeWindow::handle_events(bool&) -> void {

    XPending(m_display);
    WindowEventQueue event_queue;
    while (XQLength(m_display) != 0) {
        XEvent event;
        XNextEvent(m_display, &event);
        process_event(&event, this, &event_queue);
    }
}
} // namespace JadeFrame