#include "WinWindow.h"

#include <windows.h>

#include "WindowsMessageMap.h"

#include "../../BaseApp.h" // for the singleton

#include <iostream>
#include <tuple>
#include <unordered_map>


static WindowsMessageMap windows_message_map;

#if 1 // TODO: Utility function, move it to another place
static auto win32_convert_char_array_to_LPCWSTR(const char* charArray) -> wchar_t* {
	wchar_t* wString = new wchar_t[4096];
	//wchar_t wString[4096];
	::MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}
#endif

static auto window_resize_callback(Windows_Window& current_window, const WindowsMessage& window_message) -> void {
	//window_message.hWnd;
	//window_message.message;
	auto wParam = window_message.wParam;
	auto lParam = window_message.lParam;

	switch (wParam) {
		case SIZE_MAXIMIZED: current_window.m_window_state = Windows_Window::WINDOW_STATE::MAXIMIZED; break;
		case SIZE_MINIMIZED: current_window.m_window_state = Windows_Window::WINDOW_STATE::MINIMIZED; break;
		case SIZE_RESTORED: current_window.m_window_state = Windows_Window::WINDOW_STATE::WINDOWED; break;
		case SIZE_MAXHIDE: break;
		case SIZE_MAXSHOW: break;
	}
	current_window.m_size.width = LOWORD(lParam);
	current_window.m_size.height = HIWORD(lParam);

	auto& renderer = JadeFrame::get_singleton()->m_current_app->m_renderer;
	renderer.set_viewport(0, 0, current_window.m_size.width, current_window.m_size.height);

}
static auto window_move_callback(Windows_Window& current_window, const WindowsMessage& window_message) -> void {
	//window_message.hWnd;
	//window_message.message;
	//window_message.wParam;
	auto lParam = window_message.lParam;

	current_window.m_position.x = LOWORD(lParam);
	current_window.m_position.y = HIWORD(lParam);
}
static auto window_focux_callback(Windows_Window& current_window, bool should_focus) {
	current_window.has_focus = should_focus;
}

static auto CALLBACK window_procedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
	WindowsMessage win_message = { hWnd, message, wParam, lParam };

	BaseApp* app = JadeFrame::get_singleton()->m_current_app;
	if (app == nullptr) {
		//std::cout << "WindowProced___:" << windows_message_map(win_message);
		return DefWindowProc(hWnd, message, wParam, lParam);
	} else {
		//std::cout << "WindowProcedure:" << windows_message_map(win_message);
	}


	Windows_InputManager& input_manager = JadeFrame::get_singleton()->m_input_manager;
	int current_window_id = -1;
	for (auto const& [window_id, window] : app->m_windows) {
		if (window.m_window_handle == hWnd) {
			current_window_id = window_id;
		}
	}
	Windows_Window& current_window = app->m_windows[current_window_id];

	switch (message) {
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		{
			//std::cout << "WindowProcedure:" << windows_message_map(win_message);
			bool should_focus = message == WM_SETFOCUS ? true : false;
			window_focux_callback(current_window, should_focus);
		}break;
		case WM_SIZE:
		{
			window_resize_callback(current_window, win_message);

		} break;
		case WM_MOVE:
		{
			window_move_callback(current_window, win_message);
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

			current_window.deinitialize();
			app->m_windows.erase(current_window_id);

			std::cout << "\tLOG: Window Nr " << app->m_windows.size() << " closing" << std::endl;
			if (app->m_windows.empty() == true) {
				std::cout << "\tLOG: All Windows were closed" << std::endl;
				::PostQuitMessage(0);
			} else if (app->m_windows.contains(0) == false) {
				std::cout << "\tLOG: Main Window was closed thus every other as well" << std::endl;
				::PostQuitMessage(0);
				app->m_is_running = false;
			}

			//TODO: One has to add code which deals with ImGui

		}break;
		case WM_DESTROY:
		{
			std::cout << "WindowProcedure:" << windows_message_map(win_message);
			//::PostQuitMessage(0);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}break;
		case WM_QUIT:
		{
			std::cout << "WindowProcedure:" << windows_message_map(win_message);
			return DefWindowProc(hWnd, message, wParam, lParam);
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
static auto win32_register_window_class(HINSTANCE instance) -> void {
	WNDCLASSEX window_class;
	ZeroMemory(&window_class, sizeof(window_class));
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc = window_procedure;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = instance;
	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground = nullptr;
	window_class.lpszMenuName = nullptr;
	window_class.lpszClassName = L"JadeFrame";//"L"JadeFrame Window";
	if (!::RegisterClassExW(&window_class)) {
		auto er = ::GetLastError();
		LPVOID lpMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			er,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		std::cout << "Window Registration Failed! " << er << std::endl;
	}
}

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
		std::cout << "win32_create_window error: " << GetLastError() << std::endl;
	}
	return window_handle;
}


Windows_Window::~Windows_Window() {

	////wglMakeCurrent(NULL, NULL);
	//wglDeleteContext(m_render_context);
	//::ReleaseDC(m_window_handle, m_device_context);
	//::DestroyWindow(m_window_handle);
}

auto Windows_Window::initialize(const std::string& title, const Vec2& size, const Vec2& position) -> void {
	HINSTANCE instance = GetModuleHandleW(NULL);
	static bool is_window_class_registered = false;
	if (is_window_class_registered == false) {
		win32_register_window_class(instance);
		is_window_class_registered = true;
	}
	HWND window_handle = win32_create_window(instance, title, size, position);

	m_title = title;
	m_size = size;
	m_position = position;
	m_window_handle = window_handle;

	::ShowWindow(window_handle, SW_SHOW);
}

auto Windows_Window::deinitialize() const -> void {
	//wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_render_context);
	::ReleaseDC(m_window_handle, m_device_context);
	::DestroyWindow(m_window_handle);
}

auto Windows_Window::set_title(const std::string& title) {
	m_title = title;
	SetWindowTextA(m_window_handle, m_title.c_str());
}

//auto Windows_Window::set_v_sync(bool b) -> void {
//	//wglSwapIntervalEXT(static_cast<int>(b));
//}
#include <cassert>
auto Windows_Window::make_current() const -> void {
	assert(m_device_context != 0);
	assert(m_render_context != 0);
	BOOL result = wglMakeCurrent(m_device_context, m_render_context);
	if (result == FALSE) {
		__debugbreak();
		std::cout << GetLastError() << std::endl;
	}
}
