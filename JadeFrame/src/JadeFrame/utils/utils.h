#pragma once
#include <sysinfoapi.h>
#include <stdio.h>
#include <glad\glad.h>
#include <iostream>
#include <vector>
#include <profileapi.h>



struct CPUInfo {

	DWORD OEM_ID;
	DWORD page_size;
	DWORD processor_type;
	DWORD number_of_processors;
	LPVOID minimum_application_address;
	LPVOID maximum_application_address;
	DWORD_PTR active_processor_mask;

	CPUInfo() {
		SYSTEM_INFO siSysInfo;
		GetSystemInfo(&siSysInfo);

		OEM_ID = siSysInfo.dwOemId;
		number_of_processors = siSysInfo.dwNumberOfProcessors;
		page_size = siSysInfo.dwPageSize;
		processor_type = siSysInfo.dwProcessorType;
		minimum_application_address = siSysInfo.lpMinimumApplicationAddress;
		maximum_application_address = siSysInfo.lpMaximumApplicationAddress;
		active_processor_mask = siSysInfo.dwActiveProcessorMask;
	}
	auto print() -> void {
		printf("Hardware information: \n");
		printf("  OEM ID: %lu\n", OEM_ID);
		printf("  Number of processors: %lu\n", number_of_processors);
		printf("  Page size: %lu\n", page_size);
		printf("  Processor type: %lu\n", processor_type);
		printf("  Minimum application address: %p\n", minimum_application_address);
		printf("  Maximum application address: %p\n", maximum_application_address);
		printf("  Active processor mask: %llu\n", active_processor_mask);
	}

};
struct GPUInfo {
	std::string vendor;
	std::string renderer;
	std::string version;
	std::string shading_language_version;
	std::vector<std::string> extentenions;
	int major_version;
	int minor_version;
	int max_clip_distances;
	int max_draw_buffers;
	int num_extensions;

	GPUInfo() {
		vendor = reinterpret_cast<char const*>(glGetString(GL_VENDOR));
		renderer = reinterpret_cast<char const*>(glGetString(GL_RENDERER));
		version = reinterpret_cast<char const*>(glGetString(GL_VERSION));
		shading_language_version = reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
		glGetIntegerv(GL_MAJOR_VERSION, &major_version);
		glGetIntegerv(GL_MINOR_VERSION, &minor_version);
		glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
		glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
		glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
		glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
		for (int i = 0; i < num_extensions; i++) {
			extentenions.push_back(reinterpret_cast<char const*>(glGetStringi(GL_EXTENSIONS, i)));
		}
	}
	auto print() -> void {
		std::cout << "GL_VENDOR: " << vendor << std::endl;
		std::cout << "GL_RENDERER: " << renderer << std::endl;
		std::cout << "GL_VERSION: " << version << std::endl;
		std::cout << "GL_SHADING_LANGUAGE_VERSION: " << shading_language_version << std::endl;

		std::cout << "GL_MAJOR_VERSION: " << major_version << std::endl;
		std::cout << "GL_MINOR_VERSION: " << minor_version << std::endl;
		std::cout << "GL_MAX_DRAW_BUFFERS: " << max_draw_buffers << std::endl;
		std::cout << "GL_MAX_CLIP_DISTANCES: " << max_clip_distances << std::endl;
		std::cout << "GL_NUM_EXTENSIONS: " << num_extensions << std::endl;
		for (int i = 0; i < num_extensions; i++) {
			std::cout << "GL_EXTENSIONS " << i << ": " << extentenions[i] << std::endl;
		}
	}

};

class Timer {
private:
	struct Members {
		LARGE_INTEGER m_start;
		double m_frequency;
	};
	uint8_t m_reserved[32];
	Members* m_members;
public:
	Timer()
		: m_members(new (m_reserved) Members()) {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		m_members->m_frequency = 1.0 / frequency.QuadPart;

		this->reset();
	}

	void reset() {
		QueryPerformanceCounter(&m_members->m_start);
	}

	float elapsed() {
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);
		uint64_t cycles = current.QuadPart - m_members->m_start.QuadPart;
		return static_cast<float>(cycles * m_members->m_frequency);
	}

	float elapsed_millis() {
		return elapsed() * 1000.0f;
	}
};

class Timestep {
public:
	float m_time_step;
	float m_last_time;
public:
	Timestep(const float initial_time)
		: m_time_step(0.0f)
		, m_last_time(initial_time) {
	}
	auto update(const float current_time) -> void {
		m_time_step = current_time - m_last_time;
		m_last_time = current_time;
	}
	auto get_millis() const -> float {
		return m_time_step;
	}
	auto get_seconds() const -> float {
		return m_time_step * 0.001f;
	}
};

#if 0
#include <string>
#include <codecvt>
#include <locale>
using convert_t = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_t, wchar_t> strconverter;

static auto from_wstring_to_string(std::wstring wstr) -> std::string {
	return strconverter.to_bytes(wstr);
}

static auto from_string_to_wstring(std::string str) -> std::wstring {
	return strconverter.from_bytes(str);
}
#else
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