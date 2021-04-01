#include "opengl_context.h"

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
		case GL_DEBUG_TYPE_POP_GROUP:			_type = "Pop Group "; break;
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

	if (0) {
		std::cout << "GL_ERR: ";
		std::cout << "Source: " << _source << "| ";
		std::cout << "Type: " << _type << "| ";
		std::cout << "ID: " << _id << "| ";
		std::cout << "Severity: " << _severity << "| ";
		std::cout << "Message " << message << std::endl;
	} else {
		std::cout << "GL_ERR: ";
		std::cout << "Source: " << _source << std::endl << "| ";
		std::cout << "Type: " << _type << std::endl << "| ";
		std::cout << "ID: " << _id << std::endl << "| ";
		std::cout << "Severity: " << _severity << std::endl << "| ";
		std::cout << "Message " << message << std::endl;
		std::cout << "|-----------------------------------------|" << std::endl;
	}
}

static auto set_debug_mode(bool b) -> void {
	if (b) { // enable debug output
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(opengl_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	}
}

/*
	Windows OpenGL Stuff, like WGL. This should be abstracted somehow into a plattform specific section
*/
#if _WIN32
#include "windows.h"
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

typedef BOOL	WINAPI	PFNWGLCHOOSEPIXELFORMATARBPROC(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
typedef HGLRC	WINAPI	PFNWGLCREATECONTEXTATTRIBSARBPROC(HDC hDC, HGLRC hShareContext, const int* attribList);

static PFNWGLCHOOSEPIXELFORMATARBPROC* wglChoosePixelFormatARB;
static PFNWGLCREATECONTEXTATTRIBSARBPROC* wglCreateContextAttribsARB;

static auto load_wgl_functions() -> void {
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC*)wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC*)wglGetProcAddress("wglCreateContextAttribsARB");
}

static auto wgl_load_0() -> bool {
	//DummyWindow dummy_window;
	const HINSTANCE instance = GetModuleHandleW(NULL);
	const LPCWSTR window_class_name = L"OpenGL";

	//dummy_window.registerr();
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
	::RegisterClass(&window_class);

	//dummy_window.create();
	const HWND window_handle = CreateWindowExW(
		0,
		window_class_name,		// window class
		L"Fake Window",      //title
		WS_POPUP, //WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // style
		0, 0,                       // position x, y
		1, 1,                       // width, height
		NULL, NULL,                 // parent window, menu
		instance,
		NULL
	);
	if (window_handle == NULL) {
		__debugbreak();
	}
	const HDC device_context = GetDC(window_handle);
	if (device_context == NULL) {
		__debugbreak();
	}

	//dummy_window.set_pixel_format();
	PIXELFORMATDESCRIPTOR pixel_format_descriptor;
	ZeroMemory(&pixel_format_descriptor, sizeof(pixel_format_descriptor));
	pixel_format_descriptor.nSize = sizeof(pixel_format_descriptor);
	pixel_format_descriptor.nVersion = 1;
	pixel_format_descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixel_format_descriptor.iPixelType = PFD_TYPE_RGBA;
	pixel_format_descriptor.cColorBits = 32;
	pixel_format_descriptor.cAlphaBits = 8;
	pixel_format_descriptor.cDepthBits = 24;
	pixel_format_descriptor.iLayerType = PFD_MAIN_PLANE;

	const int pixel_format_descriptor_ID = ChoosePixelFormat(device_context, &pixel_format_descriptor);
	if (pixel_format_descriptor_ID == 0) {
		__debugbreak();
	}
	const BOOL pixel_format_success = SetPixelFormat(device_context, pixel_format_descriptor_ID, &pixel_format_descriptor);
	if (pixel_format_success == FALSE) {
		__debugbreak();
	}

	//dummy_window.create_render_context();
	const HGLRC render_context = wglCreateContext(device_context);
	/*BOOL current_succes =*/ wglMakeCurrent(device_context, render_context);

	load_wgl_functions();

	//dummy_window.destroy();
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(render_context);
	::ReleaseDC(window_handle, device_context);
	::DestroyWindow(window_handle);

	//dummy_window.unregister_();
	UnregisterClassW(window_class_name, instance);

	return true;
}

static auto wgl_set_pixel_format(const HDC& device_context) -> void {
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

	int format_descriptor_ID;
	UINT num_formats;
	const bool status = wglChoosePixelFormatARB(device_context, pixel_attributes, NULL, 1, &format_descriptor_ID, &num_formats);

	if (status == false || num_formats == 0) {
		std::cout << "wglChoosePixelFormatARB() failed." << std::endl;
		return;
	}

	PIXELFORMATDESCRIPTOR format_descriptor;
	DescribePixelFormat(device_context, format_descriptor_ID, sizeof(format_descriptor), &format_descriptor);
	SetPixelFormat(device_context, format_descriptor_ID, &format_descriptor);
}
static auto wgl_create_render_context(HDC device_context) -> HGLRC {
	const int major_min = 4, minor_min = 5;
	int  context_attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB, // TODO check whether this UE4 part is relevant to us 
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	const HGLRC render_context = wglCreateContextAttribsARB(device_context, 0, context_attributes);
	const BOOL current_succes = wglMakeCurrent(device_context, render_context);

	{
		if (render_context == NULL) {
			std::cout << "wglCreateContextAttribsARB() failed." << std::endl;
			return NULL;
		}

		if (current_succes == false) {
			std::cout << "wglMakeCurrent() failed." << std::endl;
			return NULL;
		}
	}
	return render_context;
}
#endif


GLContext::GLContext(HWND hWnd, HDC& o_device_context, HGLRC& o_render_context) {
	static bool is_wgl_loaded = false;
	if (is_wgl_loaded == false) {
		is_wgl_loaded = wgl_load_0();
	}

	HDC device_context = GetDC(hWnd);
	wgl_set_pixel_format(device_context);
	HGLRC render_context = wgl_create_render_context(device_context);

	o_device_context = device_context;
	o_render_context = render_context;

	m_window_handle = hWnd;
	m_device_context = o_device_context;
	m_render_context = o_render_context;

	if (gladLoadGL() != 1) {
		std::cout << "gladLoadGL() failed." << std::endl;
	}

	set_debug_mode(true);
	gl_cache.set_default();
}

GLContext::~GLContext() {
}

auto GLCache::set_default() -> void {
	set_clear_color({ 0.2f, 0.2f, 0.2f, 1.0f });
	set_depth_test(true);
	set_clear_bitfield(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	set_blending(true);
	set_polygon_mode(POLYGON_FACE::FRONT_AND_BACK, POLYGON_MODE::FILL);
}

auto GLCache::set_blending(bool enable, BLENDING_FACTOR sfactor, BLENDING_FACTOR dfactor) -> void {
	if (blending != enable) {
		blending = enable;
		if (enable) {
			glEnable(GL_BLEND);
			glBlendFunc(sfactor, dfactor);
		} else {
			glDisable(GL_BLEND);
		}
	}
}

auto GLCache::set_clear_color(const Color& color) -> void {
	if (clear_color != color) {
		clear_color = color;
		glClearColor(color.r, color.g, color.b, color.a);
	}
}

auto GLCache::set_polygon_mode(POLYGON_FACE face, POLYGON_MODE mode) -> void {
	if ((polygon_mode.first != face) || (polygon_mode.second != mode)) {
		polygon_mode = { face, mode };

		glPolygonMode((GLenum)face, (GLenum)mode);
	}
}

auto GLCache::set_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield = bitfield;
}

auto GLCache::add_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield |= (1 << bitfield);
}

auto GLCache::remove_clear_bitfield(const GLbitfield& bitfield) -> void {
	clear_bitfield &= ~(1 << bitfield);
}

auto GLCache::set_depth_test(bool enable) -> void {
	if (depth_test != enable) {
		depth_test = enable;
		if (enable) {
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
	}
}