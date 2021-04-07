#pragma once
#include <cstdint>

class Windows_TimeManager {
public:
	auto initialize() -> void;
	auto get_time() const -> double;

private:
	auto query_timer_value() const -> uint64_t;
	auto get_timer_frequency() const -> uint64_t;
private:
	bool m_has_performance_counter;
	uint64_t m_frequency;
	uint64_t m_offset;

	//double current;
	//double previous;
	//double update;
	//double draw;
	//double frame;
	//double target;
};