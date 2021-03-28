#include "windows_time_manager.h"
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib")


auto Windows_TimeManager::initialize() -> void {
	timeBeginPeriod(1);
	uint64_t frequency;
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
		m_has_performance_counter = true;
		m_frequency = frequency;
	} else {
		m_has_performance_counter = true;
		m_frequency = 1000;
	}

	m_offset = query_timer_value();
}

auto Windows_TimeManager::get_time() const -> double {
	return static_cast<double>(query_timer_value() - m_offset) / get_timer_frequency();
}

auto Windows_TimeManager::query_timer_value() const -> uint64_t {
	uint64_t value;
	QueryPerformanceCounter((LARGE_INTEGER*)&value);
	return value;
}

auto Windows_TimeManager::get_timer_frequency() const -> uint64_t {
	return m_frequency;
}