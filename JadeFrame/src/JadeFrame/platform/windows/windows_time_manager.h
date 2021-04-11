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

public:
	auto calc_elapsed() -> void;
	auto frame_control() -> void;
	auto set_FPS(double FPS) -> void;
	struct Time {
		double current = 0;
		double previous = 0;
		double update = 0;
		double draw = 0;
		double frame = 0;
		double target = 0;
	};
	Time time;
	float max_FPS;
};