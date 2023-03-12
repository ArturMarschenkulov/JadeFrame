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

    HDC device_context = ::GetDC(win->m_window_handle);
    if (device_context == NULL) {
        Logger::err("GetDC(hWnd) failed! {}", ::GetLastError());
        assert(false);
    }

    return device_context;
}

auto load_opengl_funcs(/*HDC device_context, HGLRC render_context*/) -> bool {

    // const BOOL current_succes = wglMakeCurrent(device_context, render_context);
    // if (current_succes == false) {
    //     Logger::log("wglMakeCurrent() failed. {}", ::GetLastError());
    //     assert(false);
    // }

    // Load OpenGL functions with GLAD
    i32 result = gladLoadGL();
    if (result != 1) { Logger::err("gladLoadGL() failed.", ::GetLastError()); }
    return true;
}

auto init_render_context(HDC device_context) -> HGLRC {
    set_pixel_format(device_context);
    HGLRC render_context = create_render_context(device_context);
    return render_context;
}
static auto get_proc_address_wgl_funcs() -> void {
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC*)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC*)wglGetProcAddress("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC*)wglGetProcAddress("wglSwapIntervalEXT");
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC*)wglGetProcAddress("wglGetExtensionsStringEXT");
}

auto load_wgl_funcs(HMODULE instance) -> bool {
    // To load wgl functions we need to create a dummy window and context
    const LPCWSTR class_name = L"Dummy OpenGL Window";
    WNDCLASS      window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = ::DefWindowProcW;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = instance;
    window_class.hIcon = ::LoadIconW(NULL, IDI_WINLOGO);
    window_class.hCursor = ::LoadCursorW(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_MENUTEXT);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = class_name;
    if (!::RegisterClassW(&window_class)) { Logger::log("RegisterClassW Failed! {}", ::GetLastError()); }

    const HWND window_handle = ::CreateWindowExW(
        0,
        class_name,     // window class
        L"Fake Window", // title
        WS_POPUP,       // WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
        0, 0,           // position x, y
        1, 1,           // width, height
        NULL, NULL,     // parent window, menu
        instance, NULL);
    if (window_handle == NULL) assert(false);
    const HDC device_context = ::GetDC(window_handle);
    if (device_context == NULL) assert(false);

    PIXELFORMATDESCRIPTOR format;
    ZeroMemory(&format, sizeof(format));
    format.nSize = sizeof(format);
    format.nVersion = 1;
    format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    format.iPixelType = PFD_TYPE_RGBA;
    format.cColorBits = 32;
    format.cAlphaBits = 8;
    format.cDepthBits = 24;
    format.iLayerType = PFD_MAIN_PLANE;
    const i32 format_ID = ::ChoosePixelFormat(device_context, &format);
    if (format_ID == 0) { assert(false); }
    const BOOL result = ::SetPixelFormat(device_context, format_ID, &format);
    if (result == FALSE) { assert(false); }

    const HGLRC render_context = ::wglCreateContext(device_context);
    /*BOOL current_succes =*/::wglMakeCurrent(device_context, render_context);

    get_proc_address_wgl_funcs();



    // We are done loading wgl functions, now we destroy everything related to the dummy context
    ::wglMakeCurrent(NULL, NULL);
    ::wglDeleteContext(render_context);
    ::ReleaseDC(window_handle, device_context);
    ::DestroyWindow(window_handle);
    ::UnregisterClassW(class_name, instance);

    return true;
}

auto swap_interval(i32 i) -> void { wglSwapIntervalEXT(i); }

auto set_pixel_format(const HDC& device_context) -> void {

    // NOTE: There are 2 ways to set the pixel format:
    // 1. Use `ChoosePixelFormat()`
    // 2. Use `wglChoosePixelFormatARB()`
    // `wglChoosePixelFormatARB()` supercedes `ChoosePixelFormat()`.
    // Pixel formats can be grouped into 4 categories:
    //     1. Accelerated pixel formats that are displayable
    //     2. Accelerated pixel formats that are displayable and which have extended attributes
    //     3. Generic pixel formats
    //     4. Accelerated pixel formats that are non displayable
    // ´ChoosePixelFormat()` only returns pixel formats from categories 1 and 3, while `wglChoosePixelFormatARB()`
    //     returns pixel formats from all 4 categories.
    // One needs `wglChoosePixelFormatARB()` if one wants sRGB framebuffers and multisampling.


#if 0

    PIXELFORMATDESCRIPTOR format = {};
    format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    format.nVersion = 1;
    format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    format.iPixelType = PFD_TYPE_RGBA;
    format.cColorBits = 32;
    format.cRedBits = 0;
    format.cRedShift = 0;
    format.cGreenBits = 0;
    format.cGreenShift = 0;
    format.cBlueBits = 0;
    format.cBlueShift = 0;
    format.cAlphaBits = 0; // 8
    format.cAlphaShift = 0;
    format.cAccumBits = 0;
    format.cAccumRedBits = 0;
    format.cAccumGreenBits = 0;
    format.cAccumBlueBits = 0;
    format.cAccumAlphaBits = 0;
    format.cDepthBits = 24;
    format.cStencilBits = 0;
    format.cAuxBuffers = 0;
    format.iLayerType = PFD_MAIN_PLANE;
    format.bReserved = 0;
    format.dwLayerMask = 0;
    format.dwVisibleMask = 0;
    format.dwDamageMask = 0;

    i32 format_ID = ::ChoosePixelFormat(device_context, &format);

#else

    std::vector<i32> pixel_attributes;
    pixel_attributes.push_back(WGL_DRAW_TO_WINDOW_ARB); // draw to window
    pixel_attributes.push_back(GL_TRUE);
    pixel_attributes.push_back(WGL_SUPPORT_OPENGL_ARB); // support OpenGL
    pixel_attributes.push_back(GL_TRUE);
    pixel_attributes.push_back(WGL_DOUBLE_BUFFER_ARB); // double buffer
    pixel_attributes.push_back(GL_TRUE);
    pixel_attributes.push_back(WGL_PIXEL_TYPE_ARB); // RGBA type
    pixel_attributes.push_back(WGL_TYPE_RGBA_ARB);
    pixel_attributes.push_back(WGL_COLOR_BITS_ARB); // 32-bit color
    pixel_attributes.push_back(32);
    pixel_attributes.push_back(WGL_DEPTH_BITS_ARB); // 24-bit depth
    pixel_attributes.push_back(24);
    pixel_attributes.push_back(WGL_STENCIL_BITS_ARB); // 8-bit stencil
    pixel_attributes.push_back(8);
    pixel_attributes.push_back(WGL_ACCELERATION_ARB); // full acceleration
    pixel_attributes.push_back(WGL_FULL_ACCELERATION_ARB);
    pixel_attributes.push_back(WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB); // sRGB capable
    pixel_attributes.push_back(GL_TRUE);
    pixel_attributes.push_back(WGL_SWAP_METHOD_ARB); // swap exchange
    pixel_attributes.push_back(WGL_SWAP_EXCHANGE_ARB);
    pixel_attributes.push_back(0);




    i32  format_ID;
    UINT num_formats;
    bool status = ::wglChoosePixelFormatARB(device_context, pixel_attributes.data(), NULL, 1, &format_ID, &num_formats);
    if (status == false || num_formats == 0) {
        Logger::log("wglChoosePixelFormatARB() failed. {}", ::GetLastError());
        return;
    }

    // Note: Technically, this is not necessary, but it is good practice to set the pixel format
    PIXELFORMATDESCRIPTOR format;
    i32                   max_format_index = ::DescribePixelFormat(device_context, format_ID, sizeof(format), &format);
    if (DescribePixelFormat == 0) { Logger::log("DescribePixelFormat() failed. {}", ::GetLastError()); }

#endif

    BOOL result = ::SetPixelFormat(device_context, format_ID, &format);
    if (result == FALSE) { Logger::log("SetPixelFormat() failed. {}", ::GetLastError()); }
}

auto create_render_context(HDC device_context) -> HGLRC {
    constexpr bool extended_context = false;

    if (extended_context == true) {
        const i32 major_min = 4;
        const i32 minor_min = 6;

        std::vector<i32> context_attributes;
        context_attributes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB); // major version
        context_attributes.push_back(major_min);
        context_attributes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB); // minor version
        context_attributes.push_back(minor_min);
        context_attributes.push_back(WGL_CONTEXT_FLAGS_ARB); // context flags
        context_attributes.push_back(WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB);
        context_attributes.push_back(WGL_CONTEXT_PROFILE_MASK_ARB); // profile type
        context_attributes.push_back(WGL_CONTEXT_CORE_PROFILE_BIT_ARB);
        context_attributes.push_back(0);

        const HGLRC render_context = wglCreateContextAttribsARB(device_context, 0, context_attributes.data());
        if (render_context == NULL) {
            Logger::log("wglCreateContextAttribsARB() failed. {}", ::GetLastError());
            return NULL;
        }
        return render_context;
    } else {
        const HGLRC render_context = ::wglCreateContext(device_context);
        if (render_context == NULL) {
            Logger::log("wglCreateContext() failed. {}", ::GetLastError());
            return NULL;
        }
        return render_context;
    }
}
} // namespace win32
} // namespace opengl
} // namespace JadeFrame
#endif