#include "linux_window.h"

#include <string>
#include <iostream>
#include <cstdlib>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "JadeFrame/platform/linux/linux_input_manager.h"
#include "JadeFrame/platform/window.h"
#include "JadeFrame/platform/window_event.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {

auto X11_NativeWindow::create(const Window::Desc& desc) -> X11_NativeWindow {

    // X11_NativeWindow win;

    // // TODO: This should be done by the caller before calling this
    // function/constructor.
    // // There it should be decided whether to call this at all.
    // const char* x11_display_env = std::getenv("DISPLAY");
    // if (x11_display_env == nullptr) {
    //     printf("The DISPLAY environment variable is not set. This is required for
    //     X11.\n"
    //     );
    //     std::exit(1);
    // }

    // win.m_display = XOpenDisplay(nullptr);
    // if (win.m_display == nullptr) {
    //     printf("\n\tcannot connect to X server\n\n");
    //     std::exit(0);
    // }

    // XID root = DefaultRootWindow(win.m_display);
    // (void)root;

    // GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

    // win.m_visual_info = glXChooseVisual(win.m_display, 0, att);
    // int     screen = DefaultScreen(win.m_display);
    // Visual* visual = DefaultVisual(win.m_display, screen);
    // int     depth = DefaultDepth(win.m_display, screen);

    // Colormap colormap = XCreateColormap(
    //     win.m_display, RootWindow(win.m_display, screen), visual, AllocNone
    // );

    // XSetWindowAttributes window_attributes = {};
    // window_attributes.colormap = colormap;
    // window_attributes.background_pixel = 0xFFFFFFFF;
    // window_attributes.border_pixel = 0;
    // window_attributes.event_mask =
    //     KeyPressMask | KeyReleaseMask | StructureNotifyMask | ExposureMask;

    // win.m_window = ::XCreateWindow(
    //     win.m_display,
    //     RootWindow(win.m_display, screen),
    //     0,
    //     0,
    //     desc.size.x,
    //     desc.size.y,
    //     0,
    //     depth,
    //     InputOutput,
    //     visual,
    //     CWBackPixel | CWBorderPixel | CWEventMask | CWColormap,
    //     &window_attributes
    // );
    // XSelectInput(win.m_display, win.m_window, ExposureMask | KeyPressMask);
    // XMapWindow(win.m_display, win.m_window);
    // XFlush(win.m_display);

    // // Client area dimensions
    // XWindowAttributes client_attributes;
    // XGetWindowAttributes(win.m_display, win.m_window, &client_attributes);
    // // int client_width = client_attributes.width;
    // // int client_height = client_attributes.height;

    // // Window dimensions
    // ::Window     root_window;
    // int          x;
    // int          y;
    // unsigned int border_width;
    // unsigned int depth_;
    // unsigned int window_width;
    // unsigned int window_height;
    // XGetGeometry(
    //     win.m_display,
    //     win.m_window,
    //     &root_window,
    //     &x,
    //     &y,
    //     &window_width,
    //     &window_height,
    //     &border_width,
    //     &depth_
    // );

    // // m_title = desc.title;
    // win.m_size = v2u32::create(window_width, window_height);
    // // m_position = v2u32(window_rect.left, window_rect.top);

    // ::XIM xim = XOpenIM(win.m_display, nullptr, nullptr, nullptr);
    // if (xim == nullptr) {
    //     // fallback to internal input method
    //     XSetLocaleModifiers("@im=none");
    //     xim = XOpenIM(win.m_display, nullptr, nullptr, nullptr);
    // }
    // ::XIC xic = ::XCreateIC(
    //     xim,
    //     XNInputStyle,
    //     XIMPreeditNothing | XIMStatusNothing,
    //     XNClientWindow,
    //     win.m_window,
    //     XNFocusWindow,
    //     win.m_window,
    //     nullptr
    // );

    // XSetICFocus(xic);

    // XSelectInput(
    //     win.m_display,
    //     win.m_window,
    //     KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
    //         PointerMotionMask | StructureNotifyMask | ExposureMask
    // );
    // return win;

    return X11_NativeWindow{desc};
}

X11_NativeWindow::X11_NativeWindow(const Window::Desc& desc) {

    // TODO: This should be done by the caller before calling this function/constructor.
    // There it should be decided whether to call this at all.
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

    // m_visual_info = glXChooseVisual(m_display, 0, att);
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
    m_size = v2u32::create(window_width, window_height);
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
        KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask | StructureNotifyMask | ExposureMask
    );
}

X11_NativeWindow::X11_NativeWindow(X11_NativeWindow&& other) noexcept {
    m_display = std::exchange(other.m_display, nullptr);
    m_visual_info = std::exchange(other.m_visual_info, nullptr);
    m_window = std::exchange(other.m_window, 0);
    m_size = std::exchange(other.m_size, {});
}

auto X11_NativeWindow::operator=(X11_NativeWindow&& other) noexcept -> X11_NativeWindow& {
    if (this == &other) { return *this; }
    m_display = std::exchange(other.m_display, nullptr);
    m_visual_info = std::exchange(other.m_visual_info, nullptr);
    m_window = std::exchange(other.m_window, 0);
    m_size = std::exchange(other.m_size, {});
    return *this;
}

X11_NativeWindow::~X11_NativeWindow() {
    if (m_display != nullptr && m_window != 0) { XDestroyWindow(m_display, m_window); }
}

static auto is_key_event(XEvent* event) -> bool {
    return event->type == KeyPress || event->type == KeyRelease;
}

static auto from_x11_key_to_input_state(const int type) -> INPUT_STATE {
    switch (type) {
        case KeyPress: return INPUT_STATE::PRESSED;
        case KeyRelease: return INPUT_STATE::RELEASED;
        default: assert(false && "Invalid key event type");
    }
}

static auto from_x11_button_to_input_state(const int type) -> INPUT_STATE {
    switch (type) {
        case ButtonPress: return INPUT_STATE::PRESSED;
        case ButtonRelease: return INPUT_STATE::RELEASED;
        default: assert(false && "Invalid button event type");
    }
}

static auto x11_event_defines_to_string(const int type) -> std::string {
    switch (type) {
        case KeyPress: return "KeyPress";
        case KeyRelease: return "KeyRelease";
        case ButtonPress: return "ButtonPress";
        case ButtonRelease: return "ButtonRelease";
        case MotionNotify: return "MotionNotify";
        case EnterNotify: return "EnterNotify";
        case LeaveNotify: return "LeaveNotify";
        case FocusIn: return "FocusIn";
        case FocusOut: return "FocusOut";
        case KeymapNotify: return "KeymapNotify";
        case Expose: return "Expose";
        case GraphicsExpose: return "GraphicsExpose";
        case NoExpose: return "NoExpose";
        case VisibilityNotify: return "VisibilityNotify";
        case CreateNotify: return "CreateNotify";
        case DestroyNotify: return "DestroyNotify";
        case UnmapNotify: return "UnmapNotify";
        case MapNotify: return "MapNotify";
        case MapRequest: return "MapRequest";
        case ReparentNotify: return "ReparentNotify";
        case ConfigureNotify: return "ConfigureNotify";
        case ConfigureRequest: return "ConfigureRequest";
        case GravityNotify: return "GravityNotify";
        case ResizeRequest: return "ResizeRequest";
        case CirculateNotify: return "CirculateNotify";
        case CirculateRequest: return "CirculateRequest";
        case PropertyNotify: return "PropertyNotify";
        case SelectionClear: return "SelectionClear";
        case SelectionRequest: return "SelectionRequest";
        case SelectionNotify: return "SelectionNotify";
        case ColormapNotify: return "ColormapNotify";
        case ClientMessage: return "ClientMessage";
        case MappingNotify: return "MappingNotify";
        case GenericEvent: return "GenericEvent";
        case LASTEvent: return "LASTEvent";
        default: return "Unknown";
    }
}

static auto
process_event(XEvent* event, X11_NativeWindow* win, WindowEventQueue* event_queue)
    -> void {

    switch (event->type) {
        case Expose: {

        } break;
        case ButtonRelease:
        case ButtonPress: {
            unsigned int x11_button = event->xbutton.button;
            if (x11_button >= 4 && x11_button <= 7) {
                // X11 buttons 4 and 5 are scroll up and down
                // X11 buttons 6 and 7 are scroll left and right
                // We don't support these buttons
                // Logger::warn(
                //     "X11 button {} is not supported. This is a scroll button.",
                //     x11_button
                // );
                break;
            }
            BUTTON      button = translate_button(x11_button);
            INPUT_STATE button_event_type = from_x11_button_to_input_state(event->type);

            ButtonEvent button_event = {
                .type = button_event_type,
                .button = button,
            };
            WindowEvent we = {
                .type = WindowEvent::TYPE::BUTTON,
                .button_event = button_event,
            };
            event_queue->push(we);

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

            if (!is_latin_1) {
                Logger::warn("Only latin-1 alphabet is supported. This is {}", keysym);
                // assert(is_latin_1 && "Only latin-1 alphabet is supported");
            }

            KEY         key = translate_key(keysym);
            INPUT_STATE keyevent_type = from_x11_key_to_input_state(event->type);

            KeyEvent key_event = {
                .type = keyevent_type,
                .key = key,
            };
            WindowEvent we = {
                .type = WindowEvent::TYPE::KEY,
                .key_event = key_event,
            };

            event_queue->push(we);

        } break;

        case MotionNotify: {
            auto x = event->xbutton.x;
            auto y = event->xbutton.y;

            MouseEvent mouse_event = {
                .m_x = x,
                .m_y = y,
            };
            WindowEvent we = {
                .type = WindowEvent::TYPE::MOUSE,
                .mouse_event = mouse_event,
            };
            event_queue->push(we);

        } break;
        case ConfigureNotify: {

        } break;
        default: break;
    }
}

auto X11_NativeWindow::handle_events(bool&) -> void {

    // XPending == XEventsQueued(display, QueuedAfterFlush)
    // XQLength == XEventsQueued(display, QueuedAlready)
    // ???      == XEventsQueued(display, QueuedAfterReading)

    auto events_left = XPending(m_display);
    // m_platform_window->m_queue.clear();
    while (XQLength(m_display) != 0) {
        XEvent event;
        XNextEvent(m_display, &event);
        process_event(&event, this, &m_platform_window->m_queue);
    }
}

} // namespace JadeFrame