#ifdef _WIN32
#include "pch.h"

#include "JadeFrame/prelude.h"
#include "JadeFrame/platform/windows/windows_window.h"
#include "opengl_windows.h"
#include <glad/glad.h>

#pragma comment(lib, "opengl32.lib")
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB   0x2093
#define WGL_CONTEXT_FLAGS_ARB         0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_SWAP_METHOD_ARB                       0x2007
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_SWAP_EXCHANGE_ARB                     0x2028
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014

#define WGL_TYPE_RGBA_ARB         0x202B
#define WGL_FULL_ACCELERATION_ARB 0x2027

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9

#define WGL_RED_BITS_ARB     0x2015
#define WGL_GREEN_BITS_ARB   0x2017
#define WGL_BLUE_BITS_ARB    0x2019
#define WGL_ALPHA_BITS_ARB   0x201B
#define WGL_DEPTH_BITS_ARB   0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_SAMPLES_ARB      0x2042


typedef BOOL WINAPI PFNWGLCHOOSEPIXELFORMATARBPROC(
    HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
typedef HGLRC WINAPI PFNWGLCREATECONTEXTATTRIBSARBPROC(HDC hDC, HGLRC hShareContext, const int* attribList);
// typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);
typedef const char* WINAPI PFNWGLGETEXTENSIONSSTRINGEXTPROC(void);
typedef BOOL WINAPI        PFNWGLSWAPINTERVALEXTPROC(int);
// typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);

static PFNWGLCHOOSEPIXELFORMATARBPROC*    wglChoosePixelFormatARB = nullptr;
static PFNWGLCREATECONTEXTATTRIBSARBPROC* wglCreateContextAttribsARB = nullptr;
static PFNWGLSWAPINTERVALEXTPROC*         wglSwapIntervalEXT = nullptr;
static PFNWGLGETEXTENSIONSSTRINGEXTPROC*  wglGetExtensionsStringEXT = nullptr;

// static auto wgl_extension_supported(const char* extension_name) -> bool {
//	if (strstr(wglGetExtensionsStringEXT(), extension_name) == NULL)     {
//		// string was not found
//		return false;
//	}
//	// extension is supported
//	return true;
// }
namespace JadeFrame {
namespace opengl {
namespace win32 {

auto init_device_context(const IWindow* window) -> HDC {
    auto win = static_cast<const JadeFrame::win32::Window*>(window);

    static bool is_wgl_loaded = false;
    if (is_wgl_loaded == false) { is_wgl_loaded = load(win->m_instance_handle); }

    HDC device_context = ::GetDC(win->m_window_handle);
    if (device_context == NULL) {
        Logger::err("GetDC(hWnd) failed! {}", ::GetLastError());
        assert(false);
    }
    return device_context;
}

auto init_render_context(HDC device_context) -> HGLRC {
    win32::set_pixel_format(device_context);
    HGLRC render_context = win32::create_render_context(device_context);
    i32   result = gladLoadGL();
    if (result != 1) { Logger::err("gladLoadGL() failed.", ::GetLastError()); }
    return render_context;
}
static auto load_wgl_functions() -> void {
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC*)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC*)wglGetProcAddress("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC*)wglGetProcAddress("wglSwapIntervalEXT");
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC*)wglGetProcAddress("wglGetExtensionsStringEXT");
}

auto load(HMODULE instance) -> bool {
    const LPCWSTR window_class_name = L"OpenGL";

    // dummy_window.registerr();
    WNDCLASS window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = DefWindowProc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_MENUTEXT);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = window_class_name;
    if (!::RegisterClassW(&window_class)) { Logger::log("RegisterClassW Failed! {}", ::GetLastError()); }

    // dummy_window.create();
    const HWND window_handle = CreateWindowExW(
        0,
        window_class_name, // window class
        L"Fake Window",    // title
        WS_POPUP,          // WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
        0, 0,              // position x, y
        1, 1,              // width, height
        NULL, NULL,        // parent window, menu
        instance, NULL);
    if (window_handle == NULL) assert(false);
    const HDC device_context = GetDC(window_handle);
    if (device_context == NULL) assert(false);

    // dummy_window.set_pixel_format();
    PIXELFORMATDESCRIPTOR desired_pixel_format;
    ZeroMemory(&desired_pixel_format, sizeof(desired_pixel_format));
    desired_pixel_format.nSize = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion = 1;
    desired_pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;
    desired_pixel_format.cDepthBits = 24;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;

    const i32 suggested_pixel_format_ID = ChoosePixelFormat(device_context, &desired_pixel_format);
    if (suggested_pixel_format_ID == 0) { assert(false); }
    const BOOL pixel_format_success = SetPixelFormat(device_context, suggested_pixel_format_ID, &desired_pixel_format);
    if (pixel_format_success == FALSE) { assert(false); }

    // dummy_window.create_render_context();
    const HGLRC render_context = wglCreateContext(device_context);
    /*BOOL current_succes =*/wglMakeCurrent(device_context, render_context);

    load_wgl_functions();

    // dummy_window.destroy();
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(render_context);
    ::ReleaseDC(window_handle, device_context);
    ::DestroyWindow(window_handle);

    // dummy_window.unregister_();
    ::UnregisterClassW(window_class_name, instance);

    return true;
}

auto swap_interval(i32 i) -> void { wglSwapIntervalEXT(i); }

auto set_pixel_format(const HDC& device_context) -> void {
    const i32 pixel_attributes[] = {
        WGL_DRAW_TO_WINDOW_ARB,
        GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,
        GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,
        GL_TRUE,
        WGL_PIXEL_TYPE_ARB,
        WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,
        32,
        WGL_DEPTH_BITS_ARB,
        24,
        WGL_STENCIL_BITS_ARB,
        8,

        WGL_ACCELERATION_ARB,
        WGL_FULL_ACCELERATION_ARB,
        WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB,
        GL_TRUE,
        WGL_SWAP_METHOD_ARB,
        WGL_SWAP_EXCHANGE_ARB,
        0,
    };

    i32        format_descriptor_ID;
    UINT       num_formats;
    const bool status =
        wglChoosePixelFormatARB(device_context, pixel_attributes, NULL, 1, &format_descriptor_ID, &num_formats);
    if (status == false || num_formats == 0) {
        Logger::log("wglChoosePixelFormatARB() failed. {}", ::GetLastError());
        return;
    }

    PIXELFORMATDESCRIPTOR format_descriptor;
    i32                   maximum_pixel_format_index =
        DescribePixelFormat(device_context, format_descriptor_ID, sizeof(format_descriptor), &format_descriptor);
    if (maximum_pixel_format_index == 0) { Logger::log("DescribePixelFormat() failed. {}", ::GetLastError()); }
    BOOL result = SetPixelFormat(device_context, format_descriptor_ID, &format_descriptor);
    if (result == FALSE) { Logger::log("SetPixelFormat() failed. {}", ::GetLastError()); }
}

auto create_render_context(HDC device_context) -> HGLRC {
    const i32 major_min = 4, minor_min = 6;
    i32       context_attributes[] = {
              WGL_CONTEXT_MAJOR_VERSION_ARB,
              major_min,
              WGL_CONTEXT_MINOR_VERSION_ARB,
              minor_min,
              WGL_CONTEXT_FLAGS_ARB,
              WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB |
                  WGL_CONTEXT_DEBUG_BIT_ARB, // TODO check whether this UE4 part is relevant to us
              WGL_CONTEXT_PROFILE_MASK_ARB,
              WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
              0};
    const HGLRC render_context = wglCreateContextAttribsARB(device_context, 0, context_attributes);
    if (render_context == NULL) {
        Logger::log("wglCreateContextAttribsARB() failed. {}", ::GetLastError());
        return NULL;
    }
    const BOOL current_succes = wglMakeCurrent(device_context, render_context);
    if (current_succes == false) {
        Logger::log("wglMakeCurrent() failed. {}", ::GetLastError());
        return NULL;
    }
    return render_context;
}
} // namespace win32
} // namespace opengl
} // namespace JadeFrame
#endif