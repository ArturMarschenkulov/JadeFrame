#pragma once
#include "JadeFrame/defines.h"
#include <cstdint>

class Windows_TimeManager {
public:
	auto initialize() -> void;
	auto get_time() const -> f64;

private:
	auto query_timer_value() const -> u64;
	auto get_timer_frequency() const ->u64;
private:
	bool m_has_performance_counter;
	u64 m_frequency;
	u64 m_offset;

public:
	auto calc_elapsed() -> void;
	auto frame_control() -> void;
	auto set_FPS(f64 FPS) -> void;
	struct Time {
		f64 current = 0;
		f64 previous = 0;
		f64 update = 0;
		f64 draw = 0;
		f64 frame = 0;
		f64 target = 0;
	};
	Time time;
	f32 max_FPS;
};