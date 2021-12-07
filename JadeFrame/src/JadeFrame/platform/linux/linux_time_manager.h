#pragma once
#include "JadeFrame/defines.h"
#include "../platform_shared.h"


namespace JadeFrame {

class Linux_TimeManager : ITimeManager {
public:
	virtual auto initialize() -> void override {

	}
	virtual auto get_time() const -> f64 override {
		return {};
	}
	virtual auto calc_elapsed() -> f64 override {
		return {};
	}

	virtual auto frame_control(f64 delta_time) -> void override {
	}
	virtual auto set_FPS(f64 FPS) -> void override {
	}

};

}