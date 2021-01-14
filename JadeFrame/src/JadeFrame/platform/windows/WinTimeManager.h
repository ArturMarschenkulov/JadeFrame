#pragma once
#include <cstdint>

class WinTimeManager {
public:
	WinTimeManager();
	auto get_time();

private:
	auto get_timer_value() -> uint64_t;
	auto get_timer_frequency() -> uint64_t;
private:
	bool m_has_performance_counter;
	uint64_t m_frequency;
	uint64_t m_offset;
};