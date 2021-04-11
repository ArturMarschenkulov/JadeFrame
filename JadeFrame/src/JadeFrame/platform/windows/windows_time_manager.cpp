#include "windows_time_manager.h"
#include <Windows.h>
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

	m_offset = this->query_timer_value();
}

auto Windows_TimeManager::get_time() const -> double {
	return static_cast<double>(this->query_timer_value() - m_offset) / this->get_timer_frequency();
}

auto Windows_TimeManager::query_timer_value() const -> uint64_t {
	uint64_t value;
	QueryPerformanceCounter((LARGE_INTEGER*)&value);
	return value;
}

auto Windows_TimeManager::get_timer_frequency() const -> uint64_t {
	return m_frequency;
}

auto Windows_TimeManager::calc_elapsed() -> void {
	time.current = this->get_time();
	time.update = time.current - time.previous;
	time.previous = time.current;
}

auto Windows_TimeManager::frame_control() -> void {
	// Frame time control system
	time.current = this->get_time();
	time.draw = time.current - time.previous;
	time.previous = time.current;

	time.frame = time.update + time.draw;

	if (time.frame < time.target) {
		::Sleep((unsigned int)(float(time.target - time.frame) * 1000.0f));
		time.current = this->get_time();
		double time_wait = time.current - time.previous;
		time.previous = time.current;
		time.frame += time_wait;
	}
	//__debugbreak();
}

auto Windows_TimeManager::set_FPS(double FPS) -> void {
	max_FPS = FPS;
	time.target = 1 / (double)FPS;
}
