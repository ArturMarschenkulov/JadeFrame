#include "Window.h"
#include <Windows.h>
#include <Windowsx.h>

LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = NULL;
	//Window* window = Window::GetWindowClass(hWnd);
	//if (window == nullptr) {
	//	return DefWindowProc(hWnd, message, wParam, lParam);
	//}


	//InputManager* inputManager = window->GetInputManager();
	switch (message) {
	case WM_ACTIVATE:
	{
		if (!HIWORD(wParam)) // Is minimized
		{
			// active
		}
		else {
			// inactive
		}

		return 0;
	}
	case WM_SYSCOMMAND:
	{
		switch (wParam) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
		result = DefWindowProc(hWnd, message, wParam, lParam);
	} break;
	case WM_SETFOCUS:
		//FocusCallback(window, true);
		break;
	case WM_KILLFOCUS:
		//FocusCallback(window, false);
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		//KeyCallback(inputManager, lParam, wParam, message);
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		//MouseButtonCallback(inputManager, message, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_SIZE:
		//(window, LOWORD(lParam), HIWORD(lParam));
		break;
	default:
		result = DefWindowProc(hWnd, message, wParam, lParam);
	}
	return result;
}

HINSTANCE hInstance;
HDC hDc;
HWND hWnd = {};

static auto get_pixel_format() -> PIXELFORMATDESCRIPTOR {
	PIXELFORMATDESCRIPTOR result = {};
	result.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	result.nVersion = 1;
	result.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	result.iPixelType = PFD_TYPE_RGBA;
	result.cColorBits = 32;
	result.cDepthBits = 24;
	result.cStencilBits = 8;
	result.cAuxBuffers = 0;
	result.iLayerType = PFD_MAIN_PLANE;
	return result;
}

auto convertCharArrayToLPCWSTR(const char* charArray) -> wchar_t* {
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

auto framebuffer_size_callback(GLFWwindow* window, int width, int height) -> void {
	glViewport(0, 0, width, height);
}

Window::Window() : m_handle(nullptr) {}


struct GLContex {
	GLint gl_major_version;
	GLint gl_minor_version;
};

auto Window::init(const std::string& title, Vec2 size) -> void {
	m_size = size;
	m_title = title;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	m_handle = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), NULL, NULL);
	if (m_handle == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_handle);
	glfwSetFramebufferSizeCallback(m_handle, framebuffer_size_callback);



	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	GLint gl_major_version;
	GLint gl_minor_version;
	glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
	glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);
	std::cout << gl_major_version << ", " << gl_minor_version << std::endl;

	glfwSwapInterval(1);
}

auto Window::get_handle() const -> GLFWwindow* {
	return m_handle;
}

auto Window::get_size() const -> Vec2 {
	return m_size;
}

auto Window::get_width() const  -> float {
	return m_size.x;
}
auto Window::get_height() const -> float {
	return m_size.y;
}
