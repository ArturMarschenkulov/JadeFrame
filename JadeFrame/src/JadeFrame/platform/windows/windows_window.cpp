#include "windows_window.h"

#include <Windows.h>

#include "windows_message_map.h"

#include "JadeFrame/base_app.h" // for the singleton

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <cassert>

namespace JadeFrame {


static WindowsMessageMap windows_message_map;

#if 1 // TODO: Utility function, move it to another place
static auto win32_convert_char_array_to_LPCWSTR(const char* charArray) -> wchar_t* {
	wchar_t* wString = new wchar_t[4096];
	//wchar_t wString[4096];
	::MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}
#endif

static auto window_resize_callback(Windows_Window& window, const WindowsMessage& wm) -> void {
	//window_message.hWnd;
	//window_message.message;
	auto wParam = wm.wParam;
	auto lParam = wm.lParam;

	switch (wParam) {
		case SIZE_MAXIMIZED: window.m_window_state = Windows_Window::WINDOW_STATE::MAXIMIZED; break;
		case SIZE_MINIMIZED: window.m_window_state = Windows_Window::WINDOW_STATE::MINIMIZED; break;
		case SIZE_RESTORED: window.m_window_state = Windows_Window::WINDOW_STATE::WINDOWED; break;
		case SIZE_MAXHIDE: break;
		case SIZE_MAXSHOW: break;
	}
	window.m_size.width = LOWORD(lParam);
	window.m_size.height = HIWORD(lParam);

	//auto& renderer = JadeFrame::get_singleton()->m_current_app_p->m_renderer;
	//renderer.set_viewport(0, 0, window.m_size.width, window.m_size.height);

}
static auto window_move_callback(Windows_Window& window, const WindowsMessage& wm) -> void {
	//window_message.hWnd;
	//window_message.message;
	//window_message.wParam;
	auto lParam = wm.lParam;

	window.m_position.x = LOWORD(lParam);
	window.m_position.y = HIWORD(lParam);
}
static auto window_focus_callback(Windows_Window& window, bool should_focus) {
	window.has_focus = should_focus;
}

static auto CALLBACK window_procedure(::HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT {
	const WindowsMessage& wm = { hWnd, message, wParam, lParam };

	BaseApp* app = JadeFrameInstance::get_singleton()->m_current_app_p;
	if (app == nullptr) {
		//std::cout << "WindowProced___:" << windows_message_map(win_message);
		return DefWindowProc(hWnd, message, wParam, lParam);
	} else {
		//std::cout << "WindowProcedure:" << windows_message_map(win_message);
	}


	Windows_InputManager& input_manager = JadeFrameInstance::get_singleton()->m_input_manager;
	i32 current_window_id = -1;
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
			window_focus_callback(current_window, should_focus);
		}break;
		case WM_SIZE:
		{
			window_resize_callback(current_window, wm);

		} break;
		case WM_MOVE:
		{
			window_move_callback(current_window, wm);
		}break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			//std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
			input_manager.key_callback(wm);
		}break;
		case WM_CHAR:
		{

			//std::cout << "WindowProcedure:" << window_message_map(message, lParam, wParam);
			input_manager.char_callback(wm);
		} break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
		{
			input_manager.mouse_button_callback(wm);
		}break;

		case WM_CLOSE:
		{
			std::cout << "WindowProcedure:" << windows_message_map(wm);

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

			//TODO: Add code which deals with ImGui!

		}break;
		case WM_DESTROY:
		{
			std::cout << "WindowProcedure:" << windows_message_map(wm);
			//::PostQuitMessage(0);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}break;
		case WM_QUIT:
		{
			std::cout << "WindowProcedure:" << windows_message_map(wm);
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
//static auto win32_register_window_class(HINSTANCE instance) -> void {
//	WNDCLASSEX window_class;
//	ZeroMemory(&window_class, sizeof(window_class));
//	window_class.cbSize = sizeof(window_class);
//	window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
//	window_class.lpfnWndProc = window_procedure;
//	window_class.cbClsExtra = 0;
//	window_class.cbWndExtra = 0;
//	window_class.hInstance = instance;
//	window_class.hIcon = LoadIcon(NULL, IDI_WINLOGO);
//	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
//	window_class.hbrBackground = nullptr;
//	window_class.lpszMenuName = nullptr;
//	window_class.lpszClassName = L"JadeFrame";//"L"JadeFrame Window";
//	if (!::RegisterClassExW(&window_class)) {
//		std::cout << "Window Registration Failed! " << ::GetLastError() << std::endl;
//		__debugbreak();
//		//LPVOID lpMsgBuf;
//		//::FormatMessage(
//		//	FORMAT_MESSAGE_ALLOCATE_BUFFER |
//		//	FORMAT_MESSAGE_FROM_SYSTEM |
//		//	FORMAT_MESSAGE_IGNORE_INSERTS,
//		//	NULL,
//		//	er,
//		//	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		//	(LPTSTR)&lpMsgBuf,
//		//	0, NULL);
//	}
//}

//static auto win32_create_window(HINSTANCE instance, const std::string& title, Vec2 size, Vec2 position) -> HWND {
//	DWORD window_ex_style = 0;
//	LPCWSTR app_window_class = L"JadeFrame";
//	LPCWSTR app_window_title = win32_convert_char_array_to_LPCWSTR(static_cast<const char*>(title.c_str()));
//	DWORD window_style = WS_OVERLAPPEDWINDOW;
//	i32 window_x = (position.x == -1) ? CW_USEDEFAULT : position.x;
//	i32 window_y = (position.y == -1) ? CW_USEDEFAULT : position.y;
//	i32 window_width = static_cast<int32_t>(size.x); //CW_USEDEFAULT;
//	i32 window_height = static_cast<int32_t>(size.y);  //CW_USEDEFAULT;
//	HWND parent_window = NULL;
//	HMENU menu = NULL;
//	LPVOID lpParam = NULL;
//
//	HWND window_handle = CreateWindowExW(
//		window_ex_style,
//		app_window_class,
//		app_window_title,
//		window_style,
//		window_x, window_y,
//		window_width, window_height,
//		parent_window, menu,                     // parent window, menu
//		instance, lpParam
//	);
//	if (window_handle == NULL) {
//		std::cout << "win32_create_window error: " << GetLastError() << std::endl;
//		__debugbreak();
//	}
//	return window_handle;
//}


Windows_Window::Windows_Window() {
}

Windows_Window::Windows_Window(const std::string& title, const Vec2& size, const Vec2& position) {
	this->init(title, size, position);
}
Windows_Window::~Windows_Window() {
	this->deinit();
}

auto Windows_Window::set_title(const std::string& title) -> void {
	m_title = title;
	SetWindowTextA(m_window_handle, m_title.c_str());
}

auto Windows_Window::recreate() const -> void {
	::DestroyWindow(m_window_handle);
	::PostQuitMessage(0);

	this->init(m_title, m_size, m_position);
}

auto Windows_Window::init(const std::string& title, const Vec2& size, const Vec2& position) const -> void {
	HINSTANCE instance = GetModuleHandleW(NULL);
	if (instance == NULL) {
		std::cout << "GetModuleHandleW(NULL) failed! " << ::GetLastError() << std::endl;
	}
	static bool is_window_class_registered = false;
	if (is_window_class_registered == false) {

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
			std::cout << "Window Registration Failed! " << ::GetLastError() << std::endl;
			__debugbreak();
		}


		is_window_class_registered = true;
	}


	DWORD window_ex_style = 0;
	LPCWSTR app_window_class = L"JadeFrame";
	LPCWSTR app_window_title = win32_convert_char_array_to_LPCWSTR(static_cast<const char*>(title.c_str()));
	DWORD window_style = WS_OVERLAPPEDWINDOW;
	i32 window_x = (position.x == -1) ? CW_USEDEFAULT : position.x;
	i32 window_y = (position.y == -1) ? CW_USEDEFAULT : position.y;
	i32 window_width = static_cast<int32_t>(size.x); //CW_USEDEFAULT;
	i32 window_height = static_cast<int32_t>(size.y);  //CW_USEDEFAULT;
	HWND parent_window = NULL;
	HMENU menu = NULL;
	LPVOID lpParam = NULL;
	HWND window_handle = CreateWindowExW(
		window_ex_style,
		app_window_class,
		app_window_title,
		window_style,
		window_x,
		window_y,
		window_width,
		window_height,
		parent_window,
		menu,                     // parent window, menu
		instance,
		lpParam
	);
	if (window_handle == NULL) {
		std::cout << "win32_create_window error: " << GetLastError() << std::endl;
		__debugbreak();
	}


	//HWND window_handle = win32_create_window(instance, title, size, position);

	m_title = title;
	m_size = size;
	m_position = position;
	m_window_handle = window_handle;

	::ShowWindow(window_handle, SW_SHOW);
}

auto Windows_Window::deinit() const -> void {
	::DestroyWindow(m_window_handle);

}

}