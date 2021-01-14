#pragma once

#include "graphics/Camera.h"

#include "platform/windows/WinTimeManager.h"
#include "platform/windows/WinInputManager.h"

#include "graphics/GLRenderer.h"

#include <set>
#include <unordered_map>
#include <unordered_set>
#include <map>



#if 1 // TODO: move to somewhere else in the future
// Convert a wide Unicode string to an UTF8 string
static auto from_wstring_to_string(const std::wstring& wstr) -> std::string {
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
static auto from_string_to_wstring(const std::string& str) -> std::wstring {
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
#endif


class WinSystemManager {
public:
	WinSystemManager() {
		{
			TCHAR computer_name[256] = L"";
			uint32_t sizee = sizeof(computer_name);
			GetComputerName(computer_name, (::DWORD*)&sizee);
			m_computer_name = from_wstring_to_string(computer_name);
		}

		{
			TCHAR user_name[256] = L"";
			uint32_t sizee = sizeof(user_name);
			GetUserName(user_name, (::DWORD*)&sizee);
			m_user_name = from_wstring_to_string(user_name);
		}

		{
			MEMORYSTATUSEX status;
			status.dwLength = sizeof(MEMORYSTATUSEX);
			GlobalMemoryStatusEx(&status);

			m_available_physical_memory = (int64_t)status.ullAvailPhys;
			m_total_physical_memory = (int64_t)status.ullTotalPhys;

			m_available_virtual_memory = (int64_t)status.ullAvailVirtual;
			m_total_virtual_memory = (int64_t)status.ullTotalVirtual;

		}
	}
	auto bytes_to_string(int64_t bytes) -> std::string {
		static const double TB = 1024ull * 1024ull * 1024ull * 1024ull;
		static const double GB = 1024ull * 1024ull * 1024ull;
		static const double MB = 1024ull * 1024ull;
		static const double KB = 1024ull;

		std::string result;
		if (bytes > TB)
			result = std::to_string(bytes / TB) + " TB";
		else if (bytes > GB)
			result = std::to_string(bytes / GB) + " GB";
		else if (bytes > MB)
			result = std::to_string(bytes / MB) + " MB";
		else if (bytes > KB)
			result = std::to_string(bytes / KB) + " KB";
		else
			result = std::to_string((float)bytes) + " bytes";

		return result;
	}
	auto log() -> void {
		std::cout << "Computer Name: " << m_computer_name << std::endl;
		std::cout << "User Name    : " << m_user_name << std::endl;

		std::cout << "Available Physical Memory: " << bytes_to_string(m_available_physical_memory) << std::endl;
		std::cout << "Total Physical Memory    : " << bytes_to_string(m_total_physical_memory) << std::endl;
		std::cout << "Available Virtual Memory : " << bytes_to_string(m_available_virtual_memory) << std::endl;
		std::cout << "Total Virtual Memory     : " << bytes_to_string(m_total_virtual_memory) << std::endl;

	}
	std::string m_computer_name;
	std::string m_user_name;
	int64_t m_available_physical_memory;
	int64_t m_total_physical_memory;
	int64_t m_available_virtual_memory;
	int64_t m_total_virtual_memory;
};


struct CachedData {
	std::deque<Object> m_objects;
	std::deque<GLTexture> m_textures;
	std::deque<GLShader> m_shaders;
	std::deque<Mesh> m_meshes;
};
class WinWindow;
class BaseApp {
public:
	BaseApp(const std::string& title, Vec2 size, Vec2 position = { -1, -1 });
	virtual ~BaseApp();

	virtual auto init() -> void {
	}
	virtual auto update() -> void {
	}
	virtual auto draw() -> void {
	}
	auto start() -> void;
	auto poll_events() -> void;
	auto clear(GLbitfield bitfield) -> void;

	static auto get_instance()->BaseApp*;
private:
	static BaseApp* instance;
public:
	WinSystemManager m_system_manager;
	WinInputManager m_input_manager;
	WinTimeManager m_time_manager;

	bool m_is_running = true;

	int m_window_counter = 0;
	std::map<int, WinWindow> m_windows;
	WinWindow* m_current_window_p;
	GLRenderer m_renderer;

	Camera1 m_camera;
};

struct TestApp : public BaseApp {
	//Camera1 m_camera;

	//GLShader m_shader[2]; // 0 = flat, 1 = textures
	std::unordered_map<std::string, GLShader> m_shaders;
	//GLTexture m_textures[2];
	std::unordered_map<std::string, GLTexture> m_textures;
	Mesh m_meshes[9] = {};
	std::vector<Object> m_objs;

	TestApp(const std::string& title, Vec2 size, Vec2 position = { -1, -1 });

	virtual auto init() -> void override;
	virtual auto update() -> void override;
	virtual auto draw() -> void override;
};