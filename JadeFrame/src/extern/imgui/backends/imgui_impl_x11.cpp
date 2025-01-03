
#include "imgui.h"

#include "imgui_impl_x11.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <ctime>
#include <cstdlib>
#include <climits>
#include <cstdint>

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear
// ImGui contexts

struct ImGui_ImplX11_Data {
    Display*         hDisplay;
    Window           hWindow;
    bool             MouseTracked;
    int              MouseButtonsDown;
    uint64_t         Time;
    uint64_t         TicksPerSecond;
    ImGuiMouseCursor LastMouseCursor;
    bool             HasGamepad;
    bool             WantUpdateHasGamepad;

    Atom BufId;
    Atom PropId;
    Atom FmtIdUtf8String;
    Atom IncrId;

    char*  ClipboardBuffer;
    size_t ClipboardBufferLength;
    size_t ClipboardBufferSize;
    bool   ClipboardOwned;

    Bool (*XQueryPointer)(
        Display*      display,
        Window        w,
        Window*       root_return,
        Window*       child_return,
        int*          root_x_return,
        int*          root_y_return,
        int*          win_x_return,
        int*          win_y_return,
        unsigned int* mask_return
    );

    ImGui_ImplX11_Data() { memset((void*)this, 0, sizeof(*this)); }
};

// It is STRONGLY preferred that you use docking branch with multi-viewports (== single
// Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this
// backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using
// multi-context.
static ImGui_ImplX11_Data* ImGui_ImplX11_GetBackendData() {
    return ImGui::GetCurrentContext()
             ? (ImGui_ImplX11_Data*)ImGui::GetIO().BackendPlatformUserData
             : NULL;
}

static void ImGui_ImplX11_SendClipboard(XSelectionRequestEvent* sender) {
    ImGui_ImplX11_Data* bd = ImGui_ImplX11_GetBackendData();

    XSelectionEvent event;

    XChangeProperty(
        bd->hDisplay,
        sender->requestor,
        sender->property,
        bd->FmtIdUtf8String,
        8,
        PropModeReplace,
        (const unsigned char*)bd->ClipboardBuffer,
        bd->ClipboardBufferLength
    );

    event.type = SelectionNotify;
    event.requestor = sender->requestor;
    event.selection = sender->selection;
    event.target = sender->target;
    event.property = sender->property;
    event.time = sender->time;

    XSendEvent(bd->hDisplay, sender->requestor, True, NoEventMask, (XEvent*)&event);
}

static void ImGui_ImplX11_SetClipboardText(void* user_data, const char* text) {
    ImGui_ImplX11_Data* bd = ImGui_ImplX11_GetBackendData();

    bd->ClipboardBufferLength = strlen(text);
    if (bd->ClipboardBufferLength > 0) {
        if (bd->ClipboardBufferLength >= bd->ClipboardBufferSize) {
            free(bd->ClipboardBuffer);
            bd->ClipboardBuffer = (char*)malloc(sizeof(char) * bd->ClipboardBufferLength);
            bd->ClipboardBufferSize = bd->ClipboardBufferLength;
        }
        memcpy(bd->ClipboardBuffer, text, bd->ClipboardBufferLength);

        if (!bd->ClipboardOwned) {
            bd->ClipboardOwned = true;
            XSetSelectionOwner(bd->hDisplay, bd->BufId, bd->hWindow, CurrentTime);
        }
    }
}

static const char* ImGui_ImplX11_GetClipboardText(void* user_data) {
    ImGui_ImplX11_Data* bd = ImGui_ImplX11_GetBackendData();

    XEvent        event;
    char*         result;
    unsigned long ressize, restail;
    int           resbits;
    bool          timed_out;
    time_t        now;

    now = time(NULL);
    timed_out = false;
    XConvertSelection(
        bd->hDisplay, bd->BufId, bd->FmtIdUtf8String, bd->PropId, bd->hWindow, CurrentTime
    );
    do {
        XNextEvent(bd->hDisplay, &event);
        if (event.type ==
            SelectionRequest) { // This happens when we are requesting our own buffer.
            ImGui_ImplX11_SendClipboard(&event.xselectionrequest);
            continue;
        }
        if (time(NULL) - now > 2) {
            timed_out = true;
            break;
        }
    } while (event.type != SelectionNotify || event.xselection.selection != bd->BufId);

    bd->ClipboardBuffer[0] = '\0';
    if (!timed_out && event.xselection.property) {
        XGetWindowProperty(
            bd->hDisplay,
            bd->hWindow,
            bd->PropId,
            0,
            LONG_MAX / 4,
            False,
            AnyPropertyType,
            &bd->FmtIdUtf8String,
            &resbits,
            &ressize,
            &restail,
            (unsigned char**)&result
        );

        if (bd->FmtIdUtf8String == bd->IncrId) {
            IM_ASSERT(
                0 && "Buffer is too large and INCR reading is not implemented yet.\n"
            );
        } else {
            if (ressize > bd->ClipboardBufferSize) {
                free(bd->ClipboardBuffer);
                bd->ClipboardBufferSize = ressize + 1;
                bd->ClipboardBuffer =
                    (char*)malloc(sizeof(char) * bd->ClipboardBufferSize);
            }
            memcpy(bd->ClipboardBuffer, result, ressize);
            bd->ClipboardBuffer[ressize] = '\0';
        }

        XFree(result);
    }

    return bd->ClipboardBuffer;
}

bool ImGui_ImplX11_Init(void* display, void* window) {
    void*    XQueryPointerFunction = nullptr;
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(
        io.BackendPlatformUserData == NULL && "Already initialized a platform backend!"
    );

    // Setup backend capabilities flags
    ImGui_ImplX11_Data* bd = IM_NEW(ImGui_ImplX11_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_X11";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor()
                                                          // values (optional)
    io.BackendFlags |=
        ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests
                                          // (optional, rarely used)
    io.GetClipboardTextFn = ImGui_ImplX11_GetClipboardText;
    io.SetClipboardTextFn = ImGui_ImplX11_SetClipboardText;

    timespec ts, tsres;
    clock_getres(CLOCK_MONOTONIC_RAW, &tsres);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    bd->hDisplay = reinterpret_cast<Display*>(display);
    bd->hWindow = reinterpret_cast<Window>(window);
    bd->WantUpdateHasGamepad = true;
    bd->TicksPerSecond =
        1000000000.0f / (static_cast<uint64_t>(tsres.tv_nsec) +
                         static_cast<uint64_t>(tsres.tv_sec) * 1000000000);
    bd->Time =
        static_cast<uint64_t>(ts.tv_nsec) + static_cast<uint64_t>(ts.tv_sec) * 1000000000;
    bd->LastMouseCursor = ImGuiMouseCursor_COUNT;
    bd->ClipboardBuffer = (char*)malloc(sizeof(char) * 256);
    bd->ClipboardBufferSize = 256;

    bd->XQueryPointer = XQueryPointerFunction == nullptr
                          ? &XQueryPointer
                          : (decltype(XQueryPointer)*)XQueryPointerFunction;

    bd->BufId = XInternAtom(bd->hDisplay, "CLIPBOARD", False);
    bd->PropId = XInternAtom(bd->hDisplay, "XSEL_DATA", False);
    bd->FmtIdUtf8String = XInternAtom(bd->hDisplay, "UTF8_STRING", False);
    bd->IncrId = XInternAtom(bd->hDisplay, "INCR", False);

    return true;
}

void ImGui_ImplX11_Shutdown() {
    ImGui_ImplX11_Data* bd = ImGui_ImplX11_GetBackendData();
    IM_ASSERT(bd != NULL && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    io.GetClipboardTextFn = NULL;
    io.SetClipboardTextFn = NULL;
    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;

    free(bd->ClipboardBuffer);
    IM_DELETE(bd);
}

static void ImGui_ImplX11_UpdateMouseData() {
    ImGui_ImplX11_Data* bd = ImGui_ImplX11_GetBackendData();
    ImGuiIO&            io = ImGui::GetIO();
    IM_ASSERT(bd->hWindow != 0);

    const bool is_app_focused = true; //(::GetForegroundWindow() == bd->hWnd);
    if (is_app_focused) {
        // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used,
        // only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        if (io.WantSetMousePos) {
            // POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
            // if (::ClientToScreen(bd->hWnd, &pos))
            //     ::SetCursorPos(pos.x, pos.y);
        }

        // (Optional) Fallback to provide mouse position when focused (WM_MOUSEMOVE
        // already provides this when hovered or captured)
        if (!io.WantSetMousePos && !bd->MouseTracked) {
            Window       unused_window;
            int          rx, ry, x, y;
            unsigned int mask;

            bd->XQueryPointer(
                bd->hDisplay,
                bd->hWindow,
                &unused_window,
                &unused_window,
                &rx,
                &ry,
                &x,
                &y,
                &mask
            );

            io.AddMousePosEvent((float)x, (float)y);
        }
    }
}

// Gamepad navigation mapping
static void ImGui_ImplX11_UpdateGamepads() {
    // TODO: support linux gamepad ?
#ifndef IMGUI_IMPL_X11_DISABLE_GAMEPAD
#endif
}

void ImGui_ImplX11_NewFrame() {
    ImGui_ImplX11_Data* bd = ImGui_ImplX11_GetBackendData();
    ImGuiIO&            io = ImGui::GetIO();
    IM_ASSERT(
        io.Fonts->IsBuilt() &&
        "Font atlas not built! It is generally built by the renderer back-end. Missing "
        "call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame()."
    );

    unsigned int width, height;
    Window       unused_window;
    int          unused_int;
    unsigned int unused_unsigned_int;

    XGetGeometry(
        bd->hDisplay,
        bd->hWindow,
        &unused_window,
        &unused_int,
        &unused_int,
        &width,
        &height,
        &unused_unsigned_int,
        &unused_unsigned_int
    );

    io.DisplaySize.x = width;
    io.DisplaySize.y = height;

    timespec ts, tsres;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    uint64_t current_time =
        static_cast<uint64_t>(ts.tv_nsec) + static_cast<uint64_t>(ts.tv_sec) * 1000000000;

    io.DeltaTime = (float)(current_time - bd->Time) / bd->TicksPerSecond;
    bd->Time = current_time;

    // Update OS mouse position
    ImGui_ImplX11_UpdateMouseData();

    // Update game controllers (if enabled and available)
    ImGui_ImplX11_UpdateGamepads();

    // return true;
}