#include "linux_window.h"

#include <X11/X.h>
#include <string>
#include <cstdlib>
#include <utility>
#include <cassert>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "JadeFrame/platform/linux/linux_input_manager.h"
#include "JadeFrame/platform/window.h"
#include "JadeFrame/platform/window_event.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {

auto X11_NativeWindow::create(const Window::Desc& desc) -> X11_NativeWindow {
    return X11_NativeWindow{desc};
}

X11_NativeWindow::X11_NativeWindow(const Window::Desc& desc) {

    // TODO: This should be done by the caller before calling this function/constructor.
    // There it should be decided whether to call this at all.
    const char* x11_display_env = std::getenv("DISPLAY");
    if (x11_display_env == nullptr) {
        printf(
            "The DISPLAY environment variable is not set. This is required for X11.\n"
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
    ::Window     root_window = 0;
    int          x = 0;
    int          y = 0;
    unsigned int border_width = 0;
    unsigned int depth_ = 0;
    unsigned int window_width = 0;
    unsigned int window_height = 0;
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

    m_title = desc.title;
    XStoreName(m_display, m_window, m_title.c_str());
    m_size = v2u32::create(window_width, window_height);
    // m_position = v2u32(window_rect.left, window_rect.top);

    m_xim = XOpenIM(m_display, nullptr, nullptr, nullptr);
    if (m_xim == nullptr) {
        // fallback to internal input method
        XSetLocaleModifiers("@im=none");
        m_xim = XOpenIM(m_display, nullptr, nullptr, nullptr);
    }
    m_xic = ::XCreateIC(
        m_xim,
        XNInputStyle,
        XIMPreeditNothing | XIMStatusNothing,
        XNClientWindow,
        m_window,
        XNFocusWindow,
        m_window,
        nullptr
    );

    XSetICFocus(m_xic);

    XSelectInput(
        m_display,
        m_window,
        KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask | StructureNotifyMask | ExposureMask
    );

    m_wm_delete = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_display, m_window, &m_wm_delete, 1);
}

X11_NativeWindow::X11_NativeWindow(X11_NativeWindow&& other) noexcept {
    m_wm_delete = std::exchange(other.m_wm_delete, 0);
    m_display = std::exchange(other.m_display, nullptr);
    m_visual_info = std::exchange(other.m_visual_info, nullptr);
    m_window = std::exchange(other.m_window, 0);
    m_size = std::exchange(other.m_size, {});
    m_title = std::exchange(other.m_title, {});
    m_xic = std::exchange(other.m_xic, nullptr);
    m_xim = std::exchange(other.m_xim, nullptr);
}

auto X11_NativeWindow::operator=(X11_NativeWindow&& other) noexcept -> X11_NativeWindow& {
    if (this == &other) { return *this; }
    if (m_display != nullptr && m_window != 0) { XDestroyWindow(m_display, m_window); }
    

    m_display = std::exchange(other.m_display, nullptr);
    m_visual_info = std::exchange(other.m_visual_info, nullptr);
    m_window = std::exchange(other.m_window, 0);
    m_size = std::exchange(other.m_size, {});
    m_wm_delete = std::exchange(other.m_wm_delete, 0);
    m_title = std::exchange(other.m_title, {});
    m_xic = std::exchange(other.m_xic, nullptr);
    m_xim = std::exchange(other.m_xim, nullptr);
    return *this;
}

X11_NativeWindow::~X11_NativeWindow() {
    if (m_xic != nullptr) {
        XDestroyIC(m_xic);
        m_xic = nullptr;
    }
    if (m_xim != nullptr) {
        XCloseIM(m_xim);
        m_xim = nullptr;
    }
    if (m_display != nullptr && m_window != 0) {
        XDestroyWindow(m_display, m_window);
        m_window = 0;
    }

    if (m_display != nullptr) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
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

static auto process_event(
    XEvent*           event,
    X11_NativeWindow* win,
    WindowEventQueue* event_queue,
    bool&             should_close
) -> void {

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

            auto is_keysym_latin = [](KeySym keysym) -> bool {
                const u32 range_0_start = 0x0020;
                const u32 range_0_end = 0x007e;
                const u32 range_1_start = 0x00a0;
                const u32 range_1_end = 0x00ff;
                return (keysym >= range_0_start && keysym <= range_0_end) ||
                       (keysym >= range_1_start && keysym <= range_1_end);
            };

            // make sure it is the latin-1 alphabet
            bool is_latin_1 = is_keysym_latin(keysym);
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
            auto x = event->xmotion.x;
            auto y = event->xmotion.y;

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
            win->m_size = v2u32::create(
                static_cast<uint32_t>(event->xconfigure.width),
                static_cast<uint32_t>(event->xconfigure.height)
            );
        } break;
        case ClientMessage: {
            if (static_cast<Atom>(event->xclient.data.l[0]) == win->m_wm_delete) {
                should_close = true;
            }
        } break;
        default: break;
    }
}

auto X11_NativeWindow::handle_events(bool& should_close) -> void {

    // XPending == XEventsQueued(display, QueuedAfterFlush)
    // XQLength == XEventsQueued(display, QueuedAlready)
    // ???      == XEventsQueued(display, QueuedAfterReading)

    auto events_left = XPending(m_display);
    // m_platform_window->m_queue.clear();
    while (XQLength(m_display) != 0) {
        XEvent event;
        XNextEvent(m_display, &event);
        process_event(&event, this, &m_platform_window->m_queue, should_close);
    }
}

} // namespace JadeFrame