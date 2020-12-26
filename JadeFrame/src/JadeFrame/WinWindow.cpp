#include "WinWindow.h"

#include <Windows.h>
#include "BaseApp.h"
#include <iostream>
#include <glad/glad.h>
#include "Input.h"
#include <Windows.h>
#include <Windowsx.h>
#include <sstream>
#include <iomanip>

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
#define REGISTER_MESSAGE(msg){msg,#msg}
// secret messages
#define WM_UAHDESTROYWINDOW 0x0090
#define WM_UAHDRAWMENU 0x0091
#define WM_UAHDRAWMENUITEM 0x0092
#define WM_UAHINITMENU 0x0093
#define WM_UAHMEASUREMENUITEM 0x0094
#define WM_UAHNCPAINTMENUPOPUP 0x0095
class WindowsMessageMap {
public:
	WindowsMessageMap() noexcept :
		map({
			REGISTER_MESSAGE(WM_CREATE),
			REGISTER_MESSAGE(WM_DESTROY),
			REGISTER_MESSAGE(WM_MOVE),
			REGISTER_MESSAGE(WM_SIZE),
			REGISTER_MESSAGE(WM_ACTIVATE),
			REGISTER_MESSAGE(WM_SETFOCUS),
			REGISTER_MESSAGE(WM_KILLFOCUS),
			REGISTER_MESSAGE(WM_ENABLE),
			REGISTER_MESSAGE(WM_SETREDRAW),
			REGISTER_MESSAGE(WM_SETTEXT),
			REGISTER_MESSAGE(WM_GETTEXT),
			REGISTER_MESSAGE(WM_GETTEXTLENGTH),
			REGISTER_MESSAGE(WM_PAINT),
			REGISTER_MESSAGE(WM_CLOSE),
			REGISTER_MESSAGE(WM_QUERYENDSESSION),
			REGISTER_MESSAGE(WM_QUIT),
			REGISTER_MESSAGE(WM_QUERYOPEN),
			REGISTER_MESSAGE(WM_ERASEBKGND),
			REGISTER_MESSAGE(WM_SYSCOLORCHANGE),
			REGISTER_MESSAGE(WM_ENDSESSION),
			REGISTER_MESSAGE(WM_SHOWWINDOW),
			REGISTER_MESSAGE(WM_CTLCOLORMSGBOX),
			REGISTER_MESSAGE(WM_CTLCOLOREDIT),
			REGISTER_MESSAGE(WM_CTLCOLORLISTBOX),
			REGISTER_MESSAGE(WM_CTLCOLORBTN),
			REGISTER_MESSAGE(WM_CTLCOLORDLG),
			REGISTER_MESSAGE(WM_CTLCOLORSCROLLBAR),
			REGISTER_MESSAGE(WM_CTLCOLORSTATIC),
			REGISTER_MESSAGE(WM_WININICHANGE),
			REGISTER_MESSAGE(WM_SETTINGCHANGE),
			REGISTER_MESSAGE(WM_DEVMODECHANGE),
			REGISTER_MESSAGE(WM_ACTIVATEAPP),
			REGISTER_MESSAGE(WM_FONTCHANGE),
			REGISTER_MESSAGE(WM_TIMECHANGE),
			REGISTER_MESSAGE(WM_CANCELMODE),
			REGISTER_MESSAGE(WM_SETCURSOR),
			REGISTER_MESSAGE(WM_MOUSEACTIVATE),
			REGISTER_MESSAGE(WM_CHILDACTIVATE),
			REGISTER_MESSAGE(WM_QUEUESYNC),
			REGISTER_MESSAGE(WM_GETMINMAXINFO),
			REGISTER_MESSAGE(WM_ICONERASEBKGND),
			REGISTER_MESSAGE(WM_NEXTDLGCTL),
			REGISTER_MESSAGE(WM_SPOOLERSTATUS),
			REGISTER_MESSAGE(WM_DRAWITEM),
			REGISTER_MESSAGE(WM_MEASUREITEM),
			REGISTER_MESSAGE(WM_DELETEITEM),
			REGISTER_MESSAGE(WM_VKEYTOITEM),
			REGISTER_MESSAGE(WM_CHARTOITEM),
			REGISTER_MESSAGE(WM_SETFONT),
			REGISTER_MESSAGE(WM_GETFONT),
			REGISTER_MESSAGE(WM_QUERYDRAGICON),
			REGISTER_MESSAGE(WM_COMPAREITEM),
			REGISTER_MESSAGE(WM_COMPACTING),
			REGISTER_MESSAGE(WM_NCCREATE),
			REGISTER_MESSAGE(WM_NCDESTROY),
			REGISTER_MESSAGE(WM_NCCALCSIZE),
			REGISTER_MESSAGE(WM_NCHITTEST),
			REGISTER_MESSAGE(WM_NCPAINT),
			REGISTER_MESSAGE(WM_NCACTIVATE),
			REGISTER_MESSAGE(WM_GETDLGCODE),
			REGISTER_MESSAGE(WM_NCMOUSEMOVE),
			REGISTER_MESSAGE(WM_NCLBUTTONDOWN),
			REGISTER_MESSAGE(WM_NCLBUTTONUP),
			REGISTER_MESSAGE(WM_NCLBUTTONDBLCLK),
			REGISTER_MESSAGE(WM_NCRBUTTONDOWN),
			REGISTER_MESSAGE(WM_NCRBUTTONUP),
			REGISTER_MESSAGE(WM_NCRBUTTONDBLCLK),
			REGISTER_MESSAGE(WM_NCMBUTTONDOWN),
			REGISTER_MESSAGE(WM_NCMBUTTONUP),
			REGISTER_MESSAGE(WM_NCMBUTTONDBLCLK),
			REGISTER_MESSAGE(WM_KEYDOWN),
			REGISTER_MESSAGE(WM_KEYUP),
			REGISTER_MESSAGE(WM_CHAR),
			REGISTER_MESSAGE(WM_DEADCHAR),
			REGISTER_MESSAGE(WM_SYSKEYDOWN),
			REGISTER_MESSAGE(WM_SYSKEYUP),
			REGISTER_MESSAGE(WM_SYSCHAR),
			REGISTER_MESSAGE(WM_SYSDEADCHAR),
			REGISTER_MESSAGE(WM_KEYLAST),
			REGISTER_MESSAGE(WM_INITDIALOG),
			REGISTER_MESSAGE(WM_COMMAND),
			REGISTER_MESSAGE(WM_SYSCOMMAND),
			REGISTER_MESSAGE(WM_TIMER),
			REGISTER_MESSAGE(WM_HSCROLL),
			REGISTER_MESSAGE(WM_VSCROLL),
			REGISTER_MESSAGE(WM_INITMENU),
			REGISTER_MESSAGE(WM_INITMENUPOPUP),
			REGISTER_MESSAGE(WM_MENUSELECT),
			REGISTER_MESSAGE(WM_MENUCHAR),
			REGISTER_MESSAGE(WM_ENTERIDLE),
			REGISTER_MESSAGE(WM_MOUSEWHEEL),
			REGISTER_MESSAGE(WM_MOUSEMOVE),
			REGISTER_MESSAGE(WM_LBUTTONDOWN),
			REGISTER_MESSAGE(WM_LBUTTONUP),
			REGISTER_MESSAGE(WM_LBUTTONDBLCLK),
			REGISTER_MESSAGE(WM_RBUTTONDOWN),
			REGISTER_MESSAGE(WM_RBUTTONUP),
			REGISTER_MESSAGE(WM_RBUTTONDBLCLK),
			REGISTER_MESSAGE(WM_MBUTTONDOWN),
			REGISTER_MESSAGE(WM_MBUTTONUP),
			REGISTER_MESSAGE(WM_MBUTTONDBLCLK),
			REGISTER_MESSAGE(WM_PARENTNOTIFY),
			REGISTER_MESSAGE(WM_MDICREATE),
			REGISTER_MESSAGE(WM_MDIDESTROY),
			REGISTER_MESSAGE(WM_MDIACTIVATE),
			REGISTER_MESSAGE(WM_MDIRESTORE),
			REGISTER_MESSAGE(WM_MDINEXT),
			REGISTER_MESSAGE(WM_MDIMAXIMIZE),
			REGISTER_MESSAGE(WM_MDITILE),
			REGISTER_MESSAGE(WM_MDICASCADE),
			REGISTER_MESSAGE(WM_MDIICONARRANGE),
			REGISTER_MESSAGE(WM_MDIGETACTIVE),
			REGISTER_MESSAGE(WM_MDISETMENU),
			REGISTER_MESSAGE(WM_CUT),
			REGISTER_MESSAGE(WM_COPYDATA),
			REGISTER_MESSAGE(WM_COPY),
			REGISTER_MESSAGE(WM_PASTE),
			REGISTER_MESSAGE(WM_CLEAR),
			REGISTER_MESSAGE(WM_UNDO),
			REGISTER_MESSAGE(WM_RENDERFORMAT),
			REGISTER_MESSAGE(WM_RENDERALLFORMATS),
			REGISTER_MESSAGE(WM_DESTROYCLIPBOARD),
			REGISTER_MESSAGE(WM_DRAWCLIPBOARD),
			REGISTER_MESSAGE(WM_PAINTCLIPBOARD),
			REGISTER_MESSAGE(WM_VSCROLLCLIPBOARD),
			REGISTER_MESSAGE(WM_SIZECLIPBOARD),
			REGISTER_MESSAGE(WM_ASKCBFORMATNAME),
			REGISTER_MESSAGE(WM_CHANGECBCHAIN),
			REGISTER_MESSAGE(WM_HSCROLLCLIPBOARD),
			REGISTER_MESSAGE(WM_QUERYNEWPALETTE),
			REGISTER_MESSAGE(WM_PALETTEISCHANGING),
			REGISTER_MESSAGE(WM_PALETTECHANGED),
			REGISTER_MESSAGE(WM_DROPFILES),
			REGISTER_MESSAGE(WM_POWER),
			REGISTER_MESSAGE(WM_WINDOWPOSCHANGED),
			REGISTER_MESSAGE(WM_WINDOWPOSCHANGING),
			REGISTER_MESSAGE(WM_HELP),
			REGISTER_MESSAGE(WM_NOTIFY),
			REGISTER_MESSAGE(WM_CONTEXTMENU),
			REGISTER_MESSAGE(WM_TCARD),
			REGISTER_MESSAGE(WM_MDIREFRESHMENU),
			REGISTER_MESSAGE(WM_MOVING),
			REGISTER_MESSAGE(WM_STYLECHANGED),
			REGISTER_MESSAGE(WM_STYLECHANGING),
			REGISTER_MESSAGE(WM_SIZING),
			REGISTER_MESSAGE(WM_SETHOTKEY),
			REGISTER_MESSAGE(WM_PRINT),
			REGISTER_MESSAGE(WM_PRINTCLIENT),
			REGISTER_MESSAGE(WM_POWERBROADCAST),
			REGISTER_MESSAGE(WM_HOTKEY),
			REGISTER_MESSAGE(WM_GETICON),
			REGISTER_MESSAGE(WM_EXITMENULOOP),
			REGISTER_MESSAGE(WM_ENTERMENULOOP),
			REGISTER_MESSAGE(WM_DISPLAYCHANGE),
			REGISTER_MESSAGE(WM_STYLECHANGED),
			REGISTER_MESSAGE(WM_STYLECHANGING),
			REGISTER_MESSAGE(WM_GETICON),
			REGISTER_MESSAGE(WM_SETICON),
			REGISTER_MESSAGE(WM_SIZING),
			REGISTER_MESSAGE(WM_MOVING),
			REGISTER_MESSAGE(WM_CAPTURECHANGED),
			REGISTER_MESSAGE(WM_DEVICECHANGE),
			REGISTER_MESSAGE(WM_PRINT),
			REGISTER_MESSAGE(WM_PRINTCLIENT),
			REGISTER_MESSAGE(WM_IME_SETCONTEXT),
			REGISTER_MESSAGE(WM_IME_NOTIFY),
			REGISTER_MESSAGE(WM_NCMOUSELEAVE),
			REGISTER_MESSAGE(WM_EXITSIZEMOVE),
			REGISTER_MESSAGE(WM_UAHDESTROYWINDOW),
			REGISTER_MESSAGE(WM_DWMNCRENDERINGCHANGED),
			REGISTER_MESSAGE(WM_ENTERSIZEMOVE),

				REGISTER_MESSAGE(WM_MOUSEHWHEEL),
				REGISTER_MESSAGE(SPI_SETACCESSTIMEOUT),
				REGISTER_MESSAGE(WM_IME_REQUEST),
				REGISTER_MESSAGE(WM_ENTERSIZEMOVE),
			})
	{}
	std::string operator()(DWORD msg, LPARAM lp, WPARAM wp) const noexcept {
		constexpr int firstColWidth = 25;
		const auto i = map.find(msg);

		std::ostringstream oss;
		if (i != map.end()) {
			oss << std::left << std::setw(firstColWidth) << i->second << std::right;
		} else {
			std::ostringstream padss;
			padss << "Unknown message: 0x" << std::hex << msg;
			oss << std::left << std::setw(firstColWidth) << padss.str() << std::right;
		}
		oss << "   LP: 0x" << std::hex << std::setfill('0') << std::setw(8) << lp;
		oss << "   WP: 0x" << std::hex << std::setfill('0') << std::setw(8) << wp << std::endl;

		return oss.str();
	}
private:
	std::unordered_map<DWORD, std::string> map;
};

void opengl_message_callback(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam) {
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "GL_DEBUG_SEVERITY_HIGH        : " << message << std::endl; return;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "GL_DEBUG_SEVERITY_MEDIUM      : " << message << std::endl; return;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "GL_DEBUG_SEVERITY_LOW         : " << message << std::endl; return;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "GL_DEBUG_SEVERITY_NOTIFICATION: " << message << std::endl; return;
	}
	std::cout << "GL_DEBUG_UNKNOWN              : " << std::endl; return;
}

struct WindowsMessage {
	HWND hWnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
};
static WindowsMessageMap window_message_map;
auto window_resize_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	BaseApp* app = BaseApp::get_instance();

	switch (wParam) {
	case SIZE_MAXIMIZED:
	{
		app->m_window.is_minimized = false;
		app->m_window.is_maximized = true;
	}break;
	case SIZE_MINIMIZED:
	{
		app->m_window.is_minimized = true;
		app->m_window.is_maximized = false;
	}break;
	case SIZE_RESTORED:
	{
		app->m_window.is_minimized = false;
		app->m_window.is_maximized = false;
	}break;
	case SIZE_MAXHIDE: break;
	case SIZE_MAXSHOW: break;
	}
	app->m_window.m_size.width = LOWORD(lParam);
	app->m_window.m_size.height = HIWORD(lParam);
	glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
}
auto window_move_callback(int64_t lParam, uint64_t wParam, uint32_t message) -> void {
	BaseApp* app = BaseApp::get_instance();
	app->m_window.m_position.x = LOWORD(lParam);
	app->m_window.m_position.y = HIWORD(lParam);
}
static auto CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
	BaseApp* app = BaseApp::get_instance();
	if (app == nullptr) {
		std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	WinInputManager& input_manager = app->m_input_manager;
	WindowsMessage win_message = { hWnd, message, wParam, lParam };

	switch (message) {
	case WM_SETFOCUS:
	{
		app->m_window.has_focus = true;
	}break;
	case WM_KILLFOCUS:
	{
		app->m_window.has_focus = false;
	}break;
	case WM_SIZE:
	{
		window_resize_callback(lParam, wParam, message);
	} break;
	case WM_MOVE:
	{
		window_move_callback(lParam, wParam, message);
	}break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		//std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
		input_manager.key_callback(lParam, wParam, message);
	}break;
	case WM_CHAR:
	{
		//std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
		input_manager.char_callback(lParam, wParam, message);
	} break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	{
		input_manager.mouse_button_callback(lParam, wParam, message);
	}break;
	case WM_CLOSE:
	{
		std::cout << "WM_CLOSE" << std::endl;
		::PostQuitMessage(0);
	}break;
	case WM_DESTROY:
	{
		std::cout << "WM_DESTROY" << std::endl;
		//::PostQuitMessage(0);
	}break;
	case WM_QUIT:
	{
		std::cout << "WM_QUIT" << std::endl;
		//::PostQuitMessage(0);
	}break;

	default:
	{
		//std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
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
	window_class.cbClsExtra;
	window_class.cbWndExtra;
	window_class.hInstance = instance;
	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground;
	window_class.lpszMenuName;
	window_class.lpszClassName = L"Core";//"L"JadeFrame Window";
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
}



#include <Wingdi.h>
static auto win32_wgl_create_fake_render_context(HDC fake_device_context) -> HGLRC {
	HGLRC fake_render_context = wglCreateContext(fake_device_context);
	BOOL current_succes = wglMakeCurrent(fake_device_context, fake_render_context);
	return fake_render_context;
}
static auto win32_wgl_destroy_fake_window(HWND fake_window_handle, HDC fake_device_context, HGLRC fake_render_context) -> void {
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fake_render_context);
	::ReleaseDC(fake_window_handle, fake_device_context);
	::DestroyWindow(fake_window_handle);
}

static auto win32_create_real_window(HINSTANCE instance, const std::string& title, Vec2 size, Vec2 position) -> HWND {
	DWORD window_ex_style = 0;
	LPCWSTR app_window_class = L"Core";
	LPCWSTR app_window_title = win32_convert_char_array_to_LPCWSTR(static_cast<const char*>(title.c_str()));
	DWORD window_style = WS_OVERLAPPEDWINDOW;
	int32_t window_x = (position.x == -1) ? CW_USEDEFAULT : position.x;
	int32_t window_y = (position.y == -1) ? CW_USEDEFAULT : position.y;
	int32_t window_width = static_cast<int32_t>(size.x); //CW_USEDEFAULT;
	int32_t window_height = static_cast<int32_t>(size.y);  //CW_USEDEFAULT;
	HWND parent_window = NULL;
	HMENU menu = NULL;

	LPVOID lpParam = NULL;

	HWND real_window_handle = CreateWindowExW(
		window_ex_style,
		app_window_class,
		app_window_title,
		window_style,
		window_x, window_y,
		window_width, window_height,
		parent_window, menu,                     // parent window, menu
		instance, lpParam
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
	}


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
static auto  win32_wgl_create_fake_context(HINSTANCE instance) -> std::tuple<HWND, HDC, HGLRC> {
	HWND fake_window_handle = win32_create_fake_window(instance);
	HDC fake_device_context = GetDC(fake_window_handle);
	win32_set_fake_pixel_format(fake_device_context);
	HGLRC fake_render_context = win32_wgl_create_fake_render_context(fake_device_context);
	return std::make_tuple(fake_window_handle, fake_device_context, fake_render_context);
}

static auto win32_wgl_load(HINSTANCE instance) -> void {
	auto [fake_window_handle, fake_device_context, fake_render_context] = win32_wgl_create_fake_context(instance);
	load_wgl_functions();
	win32_wgl_destroy_fake_window(fake_window_handle, fake_device_context, fake_render_context);
}
static bool is_wgl_loaded = false;
WinWindow::WinWindow(const std::string& title, Vec2 size, Vec2 position)
	: m_window_handle(nullptr)
	, m_title(title)
	, m_size(size)
	, m_position(position) {

	HINSTANCE instance = GetModuleHandleW(NULL);
	win32_register_window_class(instance);

	if (is_wgl_loaded == false) {
		win32_wgl_load(instance);
		is_wgl_loaded = true;
	}

	HWND real_window_handle = win32_create_real_window(instance, title, size, position);
	HDC real_device_context = GetDC(real_window_handle);
	win32_wgl_set_real_pixel_format(real_device_context);
	/*HGLRC real_render_context = */win32_wgl_create_real_render_context(real_device_context);

	if (gladLoadGL() != 1) {
		std::cout << "gladLoadGL() failed." << std::endl;
	}

	//auto version = win32_convert_char_array_to_LPCWSTR((const char*)glGetString(GL_VERSION));
	//SetWindowTextW(real_window_handle, version);
	ShowWindow(real_window_handle, SW_SHOW);

	m_window_handle = real_window_handle;

	{	//OpenGL init
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(opengl_message_callback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);

		std::cout << glGetString(GL_VERSION) << std::endl;

		wglSwapIntervalEXT(1);
	}
}

auto WinWindow::init(const std::string& title, Vec2 size, Vec2 position) -> WinWindow {
	return WinWindow(title, size, position);
}

auto WinWindow::set_title(const std::string& title) {
	m_title = title;
	SetWindowTextA(m_window_handle, m_title.c_str());
}