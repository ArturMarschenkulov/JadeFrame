#include "Window.h"
#include <Windows.h>
#include "BaseApp.h"
#include <iostream>
#include <glad/glad.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013

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





#include "Input.h"

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	auto app = BaseApp::get_app_instance();
	auto input_manager = app->m_input_manager;

	switch (message) {
		//case WM_SIZE:
		//{
		//	//RECT wr;
		//	//wr.left = 100;
		//	//wr.right = width
		//	//std::cout << "WM_SIZE" << std::endl;
		//	//window_dimension dimension = getWindowDimension(hwnd);
		//	//resizeDIBSection(&globalBackBuffer, dimension.width, dimension.height);
		//	//OutputDebugStringA("WM_SIZE\n");
		//}break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP: {
		//InputManager::key_callback(lParam, wParam, message);
		input_manager.key_callback(lParam, wParam, message);

	}break;
		//case WM_CLOSE: {
		//	PostQuitMessage(0);
		//}break;
	default: {
		return DefWindowProc(hWnd, message, wParam, lParam);
	} break;
	}
	return 0;       // message handled
}
typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
	const int* piAttribIList,
	const FLOAT* pfAttribFList,
	UINT nMaxFormats,
	int* piFormats,
	UINT* nNumFormats);
static wgl_choose_pixel_format_arb* wglChoosePixelFormatARB;
typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
	const int* attribList);

static wgl_create_context_attribs_arb* wglCreateContextAttribsARB;


static auto win32_convert_char_array_to_LPCWSTR(const char* charArray) -> wchar_t* {
	//wchar_t* wString = new wchar_t[4096];
	wchar_t wString[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
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
	RegisterClassEx(&window_class);
}

static auto win32_create_fake_window(HINSTANCE instance) -> HWND {
	HWND fake_window_handle = CreateWindow(
		L"Core", L"Fake Window",      // window class, title
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


	int fake_pixel_format_descriptor_ID = ChoosePixelFormat(fake_device_context, &fake_pixel_format_descriptor);
	if (fake_pixel_format_descriptor_ID == 0) {
		std::cout << "ChoosePixelFormat() failed." << std::endl;
		return;
	} else std::cout << "ChoosePixelFormat() succeeded." << std::endl;
	if (SetPixelFormat(fake_device_context, fake_pixel_format_descriptor_ID, &fake_pixel_format_descriptor) == false) {
		std::cout << "SetPixelFormat() failed." << std::endl;
		return;
	} else std::cout << "SetPixelFormat() succeeded." << std::endl;
}
static auto win32_create_fake_render_context(HDC fake_device_context) -> HGLRC {
	HGLRC fake_render_context = wglCreateContext(fake_device_context);
	if (fake_render_context == 0) {
		std::cout << "wglCreateContext() failed." << std::endl;
		return NULL;
	} else std::cout << "wglCreateContext() succeeded." << std::endl;

	if (wglMakeCurrent(fake_device_context, fake_render_context) == false) {
		std::cout << "wglMakeCurrent() failed." << std::endl;
		return NULL;
	} else std::cout << "wglMakeCurrent() succeeded." << std::endl;
	return fake_render_context;
}
static auto win32_destroy_fake_window(HWND fake_window_handle, HDC fake_device_context, HGLRC fake_render_context) -> void {
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fake_render_context);
	ReleaseDC(fake_window_handle, fake_device_context);
	DestroyWindow(fake_window_handle);
}

static auto win32_create_real_window(HINSTANCE instance) -> HWND {
	HWND real_window_handle = CreateWindow(
		L"Core", L"OpenGL Window",        // class name, window name
		WS_OVERLAPPEDWINDOW /*| WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/, // style
		CW_USEDEFAULT, CW_USEDEFAULT,       // posx, posy
		CW_USEDEFAULT, CW_USEDEFAULT,    // width, height
		NULL, NULL,                     // parent window, menu
		instance, NULL
	);
	return real_window_handle;
}
static auto win32_set_real_pixel_format(HDC real_device_context) -> void {
	PIXELFORMATDESCRIPTOR real_pixel_format_descriptor;
	const int pixel_attributes[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE, // 0
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // 1
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE, // 2
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE, // 3
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // 4
		WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE, // 5
		0,
	};

	int real_pixel_format_descriptor_ID; UINT num_formats;
	bool status = wglChoosePixelFormatARB(real_device_context, pixel_attributes, NULL, 1, &real_pixel_format_descriptor_ID, &num_formats);

	if (status == false || num_formats == 0) {
		std::cout << "wglChoosePixelFormatARB() failed." << std::endl;
		return;
	} else std::cout << "wglChoosePixelFormatARB() succeeded." << std::endl;


	DescribePixelFormat(real_device_context, real_pixel_format_descriptor_ID, sizeof(real_pixel_format_descriptor), &real_pixel_format_descriptor);
	SetPixelFormat(real_device_context, real_pixel_format_descriptor_ID, &real_pixel_format_descriptor);
}
static auto win32_create_real_render_context(HDC real_device_context) -> HGLRC {
	const int major_min = 4, minor_min = 5;
	int  context_attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	HGLRC real_render_context = wglCreateContextAttribsARB(real_device_context, 0, context_attributes);
	if (real_render_context == NULL) {
		std::cout << "wglCreateContextAttribsARB() failed." << std::endl;
		return NULL;
	} else std::cout << "wglCreateContextAttribsARB() succeeded." << std::endl;

	if (!wglMakeCurrent(real_device_context, real_render_context)) {
		std::cout << "wglMakeCurrent() failed." << std::endl;
		return NULL;
	} else std::cout << "wglMakeCurrent() succeeded." << std::endl;
	return real_render_context;
}

static auto convertCharArrayToLPCWSTR(const char* charArray) -> wchar_t* {
	wchar_t* wString = new wchar_t[4096];
	//wchar_t wString[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}


static auto win32_make_window(Vec2 size, const std::string& title) -> HWND {
	 
}
Window::Window() : m_window_handle(nullptr) /*m_handle(nullptr)*/ {}


auto Window::init(const std::string& title, Vec2 size) -> void {
	m_size = size;
	m_title = title;
	HINSTANCE instance = GetModuleHandleW(NULL);
	win32_register_window_class(instance);

	HWND fake_window_handle = win32_create_fake_window(instance);
	HDC fake_device_context = GetDC(fake_window_handle);
	win32_set_fake_pixel_format(fake_device_context);
	HGLRC fake_render_context = win32_create_fake_render_context(fake_device_context);

	if (gladLoadGL() != 1) {
		std::cout << "gladLoadGL() failed." << std::endl;
	}

	wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb*)wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (wgl_create_context_attribs_arb*)wglGetProcAddress("wglCreateContextAttribsARB");

	HWND real_window_handle = win32_create_real_window(instance);
	HDC real_device_context = GetDC(real_window_handle);
	win32_set_real_pixel_format(real_device_context);
	win32_destroy_fake_window(fake_window_handle, fake_device_context, fake_render_context);
	HGLRC real_render_context = win32_create_real_render_context(real_device_context);


	auto version = win32_convert_char_array_to_LPCWSTR((const char*)glGetString(GL_VERSION));
	SetWindowTextW(real_window_handle, version);
	ShowWindow(real_window_handle, SW_SHOW);

	m_window_handle = real_window_handle;
	m_device_context = real_device_context;

	//glfwInit();

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//m_handle = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), NULL, NULL);
	//if (m_handle == nullptr) {
	//	std::cout << "Failed to create GLFW window" << std::endl;
	//	glfwTerminate();
	//}
	//glfwMakeContextCurrent(m_handle);
	//glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);



	//if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	//	std::cout << "Failed to initialize GLAD" << std::endl;
	//}

	//GLint gl_major_version;
	//GLint gl_minor_version;
	//glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
	//glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);
	//std::cout << gl_major_version << ", " << gl_minor_version << std::endl;

	//glfwSwapInterval(1);
}

//auto Window::get_handle() const -> GLFWwindow* {
//	return m_handle;
//}

auto Window::get_size() const -> Vec2 {
	return m_size;
}

auto Window::get_width() const  -> float {
	return m_size.x;
}
auto Window::get_height() const -> float {
	return m_size.y;
}

auto Window::display_FPS() const -> void {
	static int FPS = 0;       // This will store our fps
	static float lastTime = 0.0f;       // This will hold the time from the last frame
	float currentTime = GetTickCount64() * 0.001f;
	++FPS;
	if (currentTime - lastTime > 1.0f) {
		lastTime = currentTime;
		if (1) {
			std::string title = m_title + " FPS: " + std::to_string(FPS);

			SetWindowTextW(m_window_handle, win32_convert_char_array_to_LPCWSTR(title.c_str()));
			//glfwSetWindowTitle(m_handle, title.c_str());
		}
		FPS = 0;
	}
}
