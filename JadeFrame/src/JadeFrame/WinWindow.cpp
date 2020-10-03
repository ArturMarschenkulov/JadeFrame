#include "WinWindow.h"

#include <Windows.h>
#include "BaseApp.h"
#include <iostream>
#include <glad/glad.h>
#include "Input.h"
#include <Windows.h>
#include <Windowsx.h>

#pragma comment(lib,"opengl32.lib")

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_SWAP_METHOD_ARB						0x2007
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB						0x2014

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9

#define WGL_RED_BITS_ARB                        0x2015
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB					0x2023
#define WGL_SAMPLES_ARB							0x2042

typedef BOOL WINAPI	PFNWGLCHOOSEPIXELFORMATARBPROC(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
typedef HGLRC WINAPI PFNWGLCREATECONTEXTATTRIBSARBPROC(HDC hDC, HGLRC hShareContext, const int* attribList);
typedef BOOL WINAPI PFNWGLSWAPINTERVALEXTPROC(int interval);

static PFNWGLCHOOSEPIXELFORMATARBPROC* wglChoosePixelFormatARB;
static PFNWGLCREATECONTEXTATTRIBSARBPROC* wglCreateContextAttribsARB;
static PFNWGLSWAPINTERVALEXTPROC* wglSwapIntervalEXT;


auto load_opengl_functions() -> void {
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC*)wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC*)wglGetProcAddress("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC*)wglGetProcAddress("wglSwapIntervalEXT");
}

auto CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {

	auto app = BaseApp::get_app_instance();
	auto& input_manager = app->m_input_manager;



	switch (message) {

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP: {
		//InputManager::key_callback(lParam, wParam, message);
		input_manager.key_callback(lParam, wParam, message);
	}break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP: {
		input_manager.mouse_button_callback(lParam, wParam, message);
	}break;
	case WM_CLOSE: {
		std::cout << "WM_CLOSE" << std::endl;
		::PostQuitMessage(0);
	}break;
	case WM_DESTROY: {
		std::cout << "WM_DESTROY" << std::endl;
		//::PostQuitMessage(0);
	}break;
	case WM_QUIT: {
		std::cout << "WM_QUIT" << std::endl;
		//::PostQuitMessage(0);
	}break;

	default: {
		return DefWindowProc(hWnd, message, wParam, lParam);
	} break;
	}
	return 0;       // message handled
}



static auto win32_convert_char_array_to_LPCWSTR(const char* charArray) -> wchar_t* {
	wchar_t* wString = new wchar_t[4096];
	//wchar_t wString[4096];
	::MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

static auto win32_register_window_class(HINSTANCE instance) -> void {
	WNDCLASSEX window_class;
	ZeroMemory(&window_class, sizeof(window_class));
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = WindowProcedure;
	window_class.hInstance = instance;
	window_class.hCursor = 0;// LoadCursor(NULL, IDC_ARROW);
	window_class.lpszClassName = L"Core";
	::RegisterClassExW(&window_class);
}

static auto win32_create_fake_window(HINSTANCE instance) -> HWND {
	HWND fake_window_handle = CreateWindowExW(
		0, L"Core", L"Fake Window",      // window class, title
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
		0, 0,                       // position x, y
		1, 1,                       // width, height
		NULL, NULL,                 // parent window, menu
		instance, NULL
		);
	return fake_window_handle;
}
static auto win32_set_fake_pixel_format(HDC fake_device_context) -> void {
	PIXELFORMATDESCRIPTOR fake_pixel_format_descriptor;
	ZeroMemory(&fake_pixel_format_descriptor, sizeof(fake_pixel_format_descriptor));
	fake_pixel_format_descriptor.nSize = sizeof(fake_pixel_format_descriptor);
	fake_pixel_format_descriptor.nVersion = 1;
	fake_pixel_format_descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	fake_pixel_format_descriptor.iPixelType = PFD_TYPE_RGBA;
	fake_pixel_format_descriptor.cColorBits = 32;
	fake_pixel_format_descriptor.cAlphaBits = 8;
	fake_pixel_format_descriptor.cDepthBits = 24;
	fake_pixel_format_descriptor.iLayerType = PFD_MAIN_PLANE;

	int fake_pixel_format_descriptor_ID = ChoosePixelFormat(fake_device_context, &fake_pixel_format_descriptor);
	BOOL pixel_format_success = SetPixelFormat(fake_device_context, fake_pixel_format_descriptor_ID, &fake_pixel_format_descriptor);

	{
		if (fake_pixel_format_descriptor_ID == 0) {
			std::cout << "ChoosePixelFormat() failed." << std::endl;
			return;
		} //else std::cout << "ChoosePixelFormat() succeeded." << std::endl;
		if (pixel_format_success == false) {
			std::cout << "SetPixelFormat() failed." << std::endl;
			return;
		} //else std::cout << "SetPixelFormat() succeeded." << std::endl;
	}
}



#include <Wingdi.h>
static auto win32_wgl_create_fake_render_context(HDC fake_device_context) -> HGLRC {
	HGLRC fake_render_context = wglCreateContext(fake_device_context);
	BOOL current_succes = wglMakeCurrent(fake_device_context, fake_render_context);

	{
		if (fake_render_context == 0) {
			std::cout << "wglCreateContext() failed." << std::endl;
			return NULL;
		} //else std::cout << "wglCreateContext() succeeded." << std::endl;

		if (current_succes == false) {
			std::cout << "wglMakeCurrent() failed." << std::endl;
			return NULL;
		} //else std::cout << "wglMakeCurrent() succeeded." << std::endl;
	}
	return fake_render_context;
}
static auto win32_wgl_destroy_fake_window(HWND fake_window_handle, HDC fake_device_context, HGLRC fake_render_context) -> void {
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fake_render_context);
	::ReleaseDC(fake_window_handle, fake_device_context);
	::DestroyWindow(fake_window_handle);
}

static auto win32_create_real_window(HINSTANCE instance, Vec2 size, const std::string& title) -> HWND {
	uint32_t window_ex_style = 0;
	TCHAR app_window_class[] = L"Core";
	TCHAR* app_window_title = win32_convert_char_array_to_LPCWSTR(static_cast<const char*>(title.c_str()));
	uint32_t window_style = WS_OVERLAPPEDWINDOW;
	int32_t window_x = CW_USEDEFAULT;
	int32_t window_y = CW_USEDEFAULT;
	int32_t window_width = static_cast<int32_t>(size.x); //CW_USEDEFAULT;
	int32_t window_height = static_cast<int32_t>(size.y);  //CW_USEDEFAULT;

	HWND real_window_handle = CreateWindowExW(
		window_ex_style,
		app_window_class,
		app_window_title,
		window_style,
		window_x, window_y,
		window_width, window_height,
		NULL, NULL,                     // parent window, menu
		instance, NULL
		);
	return real_window_handle;
}
static auto win32_wgl_set_real_pixel_format(HDC real_device_context) -> void {
	PIXELFORMATDESCRIPTOR real_pixel_format_descriptor;
	const int pixel_attributes[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,

		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
		WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
		0,
	};

	int real_pixel_format_descriptor_ID;
	UINT num_formats;
	bool status = wglChoosePixelFormatARB(real_device_context, pixel_attributes, NULL, 1, &real_pixel_format_descriptor_ID, &num_formats);

	if (status == false || num_formats == 0) {
		std::cout << "wglChoosePixelFormatARB() failed." << std::endl;
		return;
	} //else std::cout << "wglChoosePixelFormatARB() succeeded." << std::endl;


	DescribePixelFormat(real_device_context, real_pixel_format_descriptor_ID, sizeof(real_pixel_format_descriptor), &real_pixel_format_descriptor);
	SetPixelFormat(real_device_context, real_pixel_format_descriptor_ID, &real_pixel_format_descriptor);
}
static auto win32_wgl_create_real_render_context(HDC real_device_context) -> HGLRC {
	const int major_min = 4, minor_min = 5;
	int  context_attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	HGLRC real_render_context = wglCreateContextAttribsARB(real_device_context, 0, context_attributes);
	BOOL current_succes = wglMakeCurrent(real_device_context, real_render_context);

	{
		if (real_render_context == NULL) {
			std::cout << "wglCreateContextAttribsARB() failed." << std::endl;
			return NULL;
		} //else std::cout << "wglCreateContextAttribsARB() succeeded." << std::endl;

		if (current_succes == false) {
			std::cout << "wglMakeCurrent() failed." << std::endl;
			return NULL;
		} //else std::cout << "wglMakeCurrent() succeeded." << std::endl;
	}
	return real_render_context;
}

#include <tuple>
auto  win32_wgl_create_fake_context(HINSTANCE instance) -> std::tuple<HWND, HDC, HGLRC> {
	HWND fake_window_handle = win32_create_fake_window(instance);
	HDC fake_device_context = GetDC(fake_window_handle);
	win32_set_fake_pixel_format(fake_device_context);
	HGLRC fake_render_context = win32_wgl_create_fake_render_context(fake_device_context);
	return std::make_tuple(fake_window_handle, fake_device_context, fake_render_context);
}

WinWindow::WinWindow() : m_window_handle(nullptr) {}

auto WinWindow::init(const std::string& title, Vec2 size) -> void {
	m_size = size;
	m_title = title;

	HINSTANCE instance = GetModuleHandleW(NULL);
	win32_register_window_class(instance);

	auto [fake_window_handle, fake_device_context, fake_render_context] = win32_wgl_create_fake_context(instance);

	load_opengl_functions();

	// equivalent to glfwCreateWindow()
	HWND real_window_handle = win32_create_real_window(instance, size, title);
	HDC real_device_context = GetDC(real_window_handle);
	win32_wgl_set_real_pixel_format(real_device_context);
	win32_wgl_destroy_fake_window(fake_window_handle, fake_device_context, fake_render_context);
	/*HGLRC real_render_context = */win32_wgl_create_real_render_context(real_device_context);


	if (gladLoadGL() != 1) {
		std::cout << "gladLoadGL() failed." << std::endl;
	}


	//auto version = win32_convert_char_array_to_LPCWSTR((const char*)glGetString(GL_VERSION));
	//SetWindowTextW(real_window_handle, version);
	ShowWindow(real_window_handle, SW_SHOW);

	m_window_handle = real_window_handle;

	std::cout << glGetString(GL_VERSION) << std::endl;
	wglSwapIntervalEXT(1);


}

//auto Window::get_handle() const -> GLFWwindow* {
//	return m_handle;
//}

//auto WinWindow::get_size() const -> Vec2 {
//	return m_size;
//}
//
//auto WinWindow::get_width() const  -> float {
//	return get_size().x;
//}
//auto WinWindow::get_height() const -> float {
//	return get_size().y;
//}