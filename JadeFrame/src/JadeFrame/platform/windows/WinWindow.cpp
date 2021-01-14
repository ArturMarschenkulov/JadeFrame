#include "WinWindow.h"
#include <windows.h>

#include "../../BaseApp.h" // for the singleton

#include <glad/glad.h>



#include <iostream>
#include <tuple>
#include <unordered_map>
#include "WindowsMessageMap.h"



/*
	OPENGL stuff
*/
#if 1 
/*
	WGL macros
*/
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
#define WGL_SWAP_EXCHANGE_ARB					0x2028
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



/*
	OpenGL function pointers
*/
typedef BOOL WINAPI	PFNWGLCHOOSEPIXELFORMATARBPROC(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
typedef HGLRC WINAPI PFNWGLCREATECONTEXTATTRIBSARBPROC(HDC hDC, HGLRC hShareContext, const int* attribList);
typedef BOOL WINAPI PFNWGLSWAPINTERVALEXTPROC(int interval);

static PFNWGLCHOOSEPIXELFORMATARBPROC* wglChoosePixelFormatARB;
static PFNWGLCREATECONTEXTATTRIBSARBPROC* wglCreateContextAttribsARB;
static PFNWGLSWAPINTERVALEXTPROC* wglSwapIntervalEXT;

static auto load_wgl_functions() -> void {
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC*)wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC*)wglGetProcAddress("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC*)wglGetProcAddress("wglSwapIntervalEXT");
}

auto opengl_message_callback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam) -> void {
	std::string _source;
	switch (source) {
	case GL_DEBUG_SOURCE_API:				_source = "API "; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		_source = "Window "; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:	_source = "Shader "; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:		_source = "Third Party "; break;
	case GL_DEBUG_SOURCE_APPLICATION:		_source = "Application "; break;
	case GL_DEBUG_SOURCE_OTHER:				_source = "Other "; break;
	default:								_source = "UNKNOWN "; break;
	}

	std::string _type;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:				_type = "Type Error "; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	_type = "Deprecated Behavior "; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	_type = "Undefined Behavior "; break;
	case GL_DEBUG_TYPE_PORTABILITY:			_type = "Protability "; break;
	case GL_DEBUG_TYPE_PERFORMANCE:			_type = "Performance "; break;
	case GL_DEBUG_TYPE_OTHER:				_type = "Other "; break;
	case GL_DEBUG_TYPE_MARKER:				_type = "Marker "; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:			_type = "Push Group "; break;
	case GL_DEBUG_TYPE_POP_GROUP:			_type = "Push Group "; break;
	default:								_type = "UKNOWN "; break;
	}

	auto _id = id;

	std::string _severity;
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:			_severity = "High "; break;
	case GL_DEBUG_SEVERITY_MEDIUM:			_severity = "Medium "; break;
	case GL_DEBUG_SEVERITY_LOW:				_severity = "Low "; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	_severity = "Notification "; break;
	default:								_severity = "UKNOWN "; break;
	}


	std::cout << "GL_ERR: ";
	std::cout << "Source: " << _source << "| ";
	std::cout << "Type: " << _type << "| ";
	std::cout << "ID: " << _id << " " << "| ";
	std::cout << "Severity: " << _severity << "| ";
	std::cout << "Message " << message << std::endl;
}
#endif
/*
	~OPENGL stuff
*/
/*
	WindowMessageMap
*/

static WindowsMessageMap windows_message_map;

//static auto window_resize_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
static auto window_resize_callback(WinWindow& current_window, const WindowsMessage& window_message) -> void {
	//window_message.hWnd;
	//window_message.message;
	auto wParam = window_message.wParam;
	auto lParam = window_message.lParam;

	switch (wParam) {
	case SIZE_MAXIMIZED:
	{
		current_window.m_window_state = WinWindow::WINDOW_STATE::MAXIMIZED;
	}break;
	case SIZE_MINIMIZED:
	{
		current_window.m_window_state = WinWindow::WINDOW_STATE::MINIMIZED;
	}break;
	case SIZE_RESTORED:
	{
		current_window.m_window_state = WinWindow::WINDOW_STATE::WINDOWED;
	}break;
	case SIZE_MAXHIDE: break;
	case SIZE_MAXSHOW: break;
	}
	current_window.m_size.width = LOWORD(lParam);
	current_window.m_size.height = HIWORD(lParam);
	glViewport(0, 0, current_window.m_size.width, current_window.m_size.height);

}
static auto window_move_callback(WinWindow& current_window, const WindowsMessage& window_message) -> void {
	//window_message.hWnd;
	//window_message.message;
	//window_message.wParam;
	auto lParam = window_message.lParam;

	current_window.m_position.x = LOWORD(lParam);
	current_window.m_position.y = HIWORD(lParam);
}




static auto win32_convert_char_array_to_LPCWSTR(const char* charArray) -> wchar_t* {
	wchar_t* wString = new wchar_t[4096];
	//wchar_t wString[4096];
	::MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}


static auto CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)->LRESULT;
static auto win32_register_window_class(HINSTANCE instance) -> void {
	WNDCLASSEX window_class;
	ZeroMemory(&window_class, sizeof(window_class));
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = WindowProcedure;
	//window_class.cbClsExtra;
	//window_class.cbWndExtra;
	window_class.hInstance = instance;
	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	//window_class.hbrBackground;
	//window_class.lpszMenuName;
	window_class.lpszClassName = L"JadeFrame";//"L"JadeFrame Window";
	if (!::RegisterClassExW(&window_class)) {
		std::cout << "Window Registration Failed!" << std::endl;
	}
}
#if 1 // dummy window logic
static auto win32_register_dummy_window_class(HINSTANCE instance) -> void {
	WNDCLASSEX window_class;
	ZeroMemory(&window_class, sizeof(window_class));
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = DefWindowProc;
	//window_class.cbClsExtra;
	//window_class.cbWndExtra;
	window_class.hInstance = instance;
	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	//window_class.hbrBackground;
	//window_class.lpszMenuName;
	window_class.lpszClassName = L"OpenGL";//"L"JadeFrame Window";
	::RegisterClassExW(&window_class);
}
static auto win32_create_dummy_window(HINSTANCE instance) -> HWND {
	HWND dummy_window_handle = CreateWindowExW(
		0,
		L"OpenGL",
		L"Fake Window",      // window class, title
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
		0, 0,                       // position x, y
		1, 1,                       // width, height
		NULL, NULL,                 // parent window, menu
		instance, NULL
	);
	return dummy_window_handle;
}
static auto win32_set_dummy_pixel_format(HDC dummy_device_context) -> void {
	PIXELFORMATDESCRIPTOR dummy_pixel_format_descriptor;
	ZeroMemory(&dummy_pixel_format_descriptor, sizeof(dummy_pixel_format_descriptor));
	dummy_pixel_format_descriptor.nSize = sizeof(dummy_pixel_format_descriptor);
	dummy_pixel_format_descriptor.nVersion = 1;
	dummy_pixel_format_descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	dummy_pixel_format_descriptor.iPixelType = PFD_TYPE_RGBA;
	dummy_pixel_format_descriptor.cColorBits = 32;
	dummy_pixel_format_descriptor.cAlphaBits = 8;
	dummy_pixel_format_descriptor.cDepthBits = 24;
	dummy_pixel_format_descriptor.iLayerType = PFD_MAIN_PLANE;

	int dummy_pixel_format_descriptor_ID = ChoosePixelFormat(dummy_device_context, &dummy_pixel_format_descriptor);
	/*BOOL pixel_format_success =*/ SetPixelFormat(dummy_device_context, dummy_pixel_format_descriptor_ID, &dummy_pixel_format_descriptor);
}
static auto win32_wgl_create_dummy_render_context(HDC dummy_device_context) -> HGLRC {
	HGLRC dummy_render_context = wglCreateContext(dummy_device_context);
	/*BOOL current_succes =*/ wglMakeCurrent(dummy_device_context, dummy_render_context);
	return dummy_render_context;
}
static auto win32_wgl_destroy_dummy_window(HWND dummy_window_handle, HDC dummy_device_context, HGLRC dummy_render_context) -> void {
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(dummy_render_context);
	::ReleaseDC(dummy_window_handle, dummy_device_context);
	::DestroyWindow(dummy_window_handle);
}
static auto win32_wgl_create_dummy_context(HINSTANCE instance) -> std::tuple<HWND, HDC, HGLRC> {
	HWND dummy_window_handle = win32_create_dummy_window(instance);
	HDC dummy_device_context = GetDC(dummy_window_handle);
	win32_set_dummy_pixel_format(dummy_device_context);
	HGLRC dummy_render_context = win32_wgl_create_dummy_render_context(dummy_device_context);
	return std::make_tuple(dummy_window_handle, dummy_device_context, dummy_render_context);
}
static auto win32_wgl_load() -> bool {
	HINSTANCE instance = GetModuleHandleW(NULL);
	win32_register_dummy_window_class(instance);
	auto [dummy_window_handle, dummy_device_context, dummy_render_context] = win32_wgl_create_dummy_context(instance);
	load_wgl_functions();
	win32_wgl_destroy_dummy_window(dummy_window_handle, dummy_device_context, dummy_render_context);
	UnregisterClassW(L"OpenGL", instance);
	return true;
}
#endif // ~ dummy window logic

static auto win32_create_window(HINSTANCE instance, const std::string& title, Vec2 size, Vec2 position) -> HWND {
	DWORD window_ex_style = 0;
	LPCWSTR app_window_class = L"JadeFrame";
	LPCWSTR app_window_title = win32_convert_char_array_to_LPCWSTR(static_cast<const char*>(title.c_str()));
	DWORD window_style = WS_OVERLAPPEDWINDOW;
	int32_t window_x = (position.x == -1) ? CW_USEDEFAULT : position.x;
	int32_t window_y = (position.y == -1) ? CW_USEDEFAULT : position.y;
	int32_t window_width = static_cast<int32_t>(size.x); //CW_USEDEFAULT;
	int32_t window_height = static_cast<int32_t>(size.y);  //CW_USEDEFAULT;
	HWND parent_window = NULL;
	HMENU menu = NULL;

	LPVOID lpParam = NULL;

	HWND window_handle = CreateWindowExW(
		window_ex_style,
		app_window_class,
		app_window_title,
		window_style,
		window_x, window_y,
		window_width, window_height,
		parent_window, menu,                     // parent window, menu
		instance, lpParam
	);
	if (window_handle == nullptr) {
		std::cout << GetLastError() << std::endl;
	}
	return window_handle;
}
static auto win32_wgl_set_pixel_format(HDC real_device_context) -> void {
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

	int pixel_format_descriptor_ID;
	UINT num_formats;
	bool status = wglChoosePixelFormatARB(real_device_context, pixel_attributes, NULL, 1, &pixel_format_descriptor_ID, &num_formats);

	if (status == false || num_formats == 0) {
		std::cout << "wglChoosePixelFormatARB() failed." << std::endl;
		return;
	}

	PIXELFORMATDESCRIPTOR pixel_format_descriptor;
	//DescribePixelFormat(real_device_context, pixel_format_descriptor_ID, sizeof(pixel_format_descriptor), &pixel_format_descriptor);
	SetPixelFormat(real_device_context, pixel_format_descriptor_ID, &pixel_format_descriptor);
}
static auto win32_wgl_create_render_context(HDC real_device_context) -> HGLRC {
	const int major_min = 4, minor_min = 5;
	int  context_attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB, // TODO check whether this UE4 part is relevant to us 
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	HGLRC render_context = wglCreateContextAttribsARB(real_device_context, 0, context_attributes);
	BOOL current_succes = wglMakeCurrent(real_device_context, render_context);

	{
		if (render_context == NULL) {
			std::cout << "wglCreateContextAttribsARB() failed." << std::endl;
			return NULL;
		} //else std::cout << "wglCreateContextAttribsARB() succeeded." << std::endl;

		if (current_succes == false) {
			std::cout << "wglMakeCurrent() failed." << std::endl;
			return NULL;
		} //else std::cout << "wglMakeCurrent() succeeded." << std::endl;
	}
	return render_context;
}




static auto CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
	WindowsMessage win_message = { hWnd, message, wParam, lParam };
	BaseApp* app = BaseApp::get_instance();
	if (app == nullptr) {
		//std::cout << "WindowProced___:" << window_message_map(win_message);
		return DefWindowProc(hWnd, message, wParam, lParam);
	} else {
		//std::cout << "WindowProcedure:" << window_message_map(win_message);
	}
	WinInputManager& input_manager = app->m_input_manager;

	int current_window_id = -1;
	for (auto const& [window_id, window] : app->m_windows) {
		if (window.m_window_handle == hWnd) {
			current_window_id = window_id;
		}
	}

	auto& current_window = app->m_windows[0];

	switch (message) {
	case WM_SETFOCUS:
	{
		std::cout << "WindowProcedure:" << windows_message_map(win_message);
		current_window.has_focus = true;
		current_window.make_current();
		//__debugbreak();
	}break;
	case WM_KILLFOCUS:
	{
		std::cout << "WindowProcedure:" << windows_message_map(win_message);
		current_window.has_focus = false;
	}break;
	case WM_SIZE:
	{
		window_resize_callback(app->m_windows[current_window_id], win_message);
	} break;
	case WM_MOVE:
	{
		window_move_callback(app->m_windows[current_window_id], win_message);
	}break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		//std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
		input_manager.key_callback(win_message);
	}break;
	case WM_CHAR:
	{
		//std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
		input_manager.char_callback(win_message);
	} break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	{
		input_manager.mouse_button_callback(win_message);
	}break;
	case WM_CLOSE:
	{
		std::cout << "WindowProcedure:" << windows_message_map(win_message);
		app->m_windows[current_window_id].deinit();
		if (current_window_id == 0) {
			PostQuitMessage(0);
		}
		//::PostQuitMessage(0);
	}break;
	case WM_DESTROY:
	{
		std::cout << "WindowProcedure:" << windows_message_map(win_message);
		//::PostQuitMessage(0);
	}break;
	case WM_QUIT:
	{
		std::cout << "WindowProcedure:" << windows_message_map(win_message);
		//::PostQuitMessage(0);
	}break;

	default:
	{
		//std::cout << "WindowProcedure:" << window_message_map(win_message);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	}
	return 0;       // message handled
}
static bool is_wgl_loaded = false;
WinWindow::WinWindow(const std::string& title, Vec2 size, Vec2 position)
	: m_window_handle(nullptr)
	, m_title(title)
	, m_size(size)
	, m_position(position) {

	_init(m_title, m_size, m_position);
}
auto WinWindow::_init(const std::string& title, Vec2 size, Vec2 position) -> void {
	m_title = title;
	m_size = size;
	m_position = position;
	if (is_wgl_loaded == false) {
		is_wgl_loaded = win32_wgl_load();
	}


	HINSTANCE instance = GetModuleHandleW(NULL);
	win32_register_window_class(instance);
	HWND real_window_handle = win32_create_window(instance, m_title, m_size, m_position);
	m_window_handle = real_window_handle;

	//auto version = win32_convert_char_array_to_LPCWSTR((const char*)glGetString(GL_VERSION));
	//SetWindowTextW(real_window_handle, version);
	ShowWindow(real_window_handle, SW_SHOW);


	/*
		OPENGL STUFF TODO: Move it to somewhere else
	*/
	HDC real_device_context = GetDC(m_window_handle);
	m_device_context = real_device_context;
	win32_wgl_set_pixel_format(real_device_context);
	HGLRC real_render_context = win32_wgl_create_render_context(real_device_context);
	m_render_context = real_render_context;

	if (gladLoadGL() != 1) {
		std::cout << "gladLoadGL() failed." << std::endl;
	}

	{	//OpenGL init
		if (true) { // enable debug output
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(opengl_message_callback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
		}


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);

		std::cout << glGetString(GL_VERSION) << std::endl;

		wglSwapIntervalEXT(1);
	}
}


WinWindow::~WinWindow() {

	////wglMakeCurrent(NULL, NULL);
	//wglDeleteContext(m_render_context);
	//::ReleaseDC(m_window_handle, m_device_context);
	//::DestroyWindow(m_window_handle);
}

auto WinWindow::deinit() -> void {
	//wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_render_context);
	::ReleaseDC(m_window_handle, m_device_context);
	::DestroyWindow(m_window_handle);
}



auto WinWindow::set_title(const std::string& title) {
	m_title = title;
	SetWindowTextA(m_window_handle, m_title.c_str());
}

auto WinWindow::set_v_sync(bool b) -> void {
	wglSwapIntervalEXT(static_cast<int>(b));
}

auto WinWindow::make_current() -> void {
	wglMakeCurrent(m_device_context, m_render_context);
}
