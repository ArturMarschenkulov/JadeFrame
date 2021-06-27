#include "windows_time_manager.h"
#include <Windows.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib")

namespace JadeFrame {

auto Windows_TimeManager::initialize() -> void {
	timeBeginPeriod(1);
	u64 frequency;
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
		m_has_performance_counter = true;
		m_frequency = frequency;
	} else {
		m_has_performance_counter = true;
		m_frequency = 1000;
	}

	m_offset = this->query_timer_value();
}

auto Windows_TimeManager::get_time() const -> f64 {
	return static_cast<f64>(this->query_timer_value() - m_offset) / this->get_timer_frequency();
}

auto Windows_TimeManager::query_timer_value() const -> u64 {
	u64 value;
	QueryPerformanceCounter((LARGE_INTEGER*)&value);
	return value;
}

auto Windows_TimeManager::get_timer_frequency() const -> u64 {
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
		::Sleep((u32)(f32(time.target - time.frame) * 1000.0f));
		time.current = this->get_time();
		f64 time_wait = time.current - time.previous;
		time.previous = time.current;
		time.frame += time_wait;
	}
	//__debugbreak();
}

auto Windows_TimeManager::set_FPS(f64 FPS) -> void {
	max_FPS = FPS;
	time.target = 1 / (f64)FPS;
}
}