#pragma once
#include "WinWindow.h"
#include "graphics/GLRenderer.h"
#include "graphics/BaseRenderer.h"
#include "Input.h"
#include <set>
#include <unordered_map>
#include <unordered_set>



class WinTimeManager {
public:
	WinTimeManager() {
		uint64_t frequency;
		if (QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
			timer.timer.has_performance_counter = true;
			timer.timer.frequency = frequency;
		} else {
			timer.timer.has_performance_counter = true;
			timer.timer.frequency = 1000;
		}

		timer.offset = get_timer_value();
	}

	auto get_time() {
		return static_cast<double>(get_timer_value() - timer.offset) / get_timer_frequency();
	}

private:
	auto get_timer_value() -> uint64_t {
		uint64_t value;
		QueryPerformanceCounter((LARGE_INTEGER*)&value);
		return value;
	}
	auto get_timer_frequency() -> uint64_t {
		return timer.timer.frequency;
	}
private:
	struct {
		struct Timer {
			bool has_performance_counter;
			uint64_t frequency;
		} timer;
		uint64_t offset;
	} timer;
};

struct CachedData {
	std::deque<Object> m_objects;
	std::deque<GLTexture> m_textures;
	std::deque<GLShader> m_shaders;
	std::deque<Mesh> m_meshes;
};

class GLContext {

	GLContext() {
	
	}

};

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
	//private:
	WinWindow m_window;
	GLRenderer m_renderer;

	WinInputManager m_input_manager;
	WinTimeManager m_time_manager;

	CachedData m_cached_data;
	std::vector<Object> m_objects;

	bool m_is_running = true;
	float m_last_frame_time = 0.0f;

};

struct TestApp : public BaseApp {
	Camera m_camera;

	GLShader m_shader;
	GLTexture m_textures[2];
	Mesh m_meshes[3] = {};

	std::vector<Object> m_objs;

	TestApp(const std::string& title, Vec2 size, Vec2 position = { -1, -1 });

	virtual auto init() -> void override;
	virtual auto update() -> void override;
	virtual auto draw() -> void override;

	auto draw_all_objects() {
		for (size_t i = 0; i < m_objects.size(); i++) {
			m_objects[i].draw();
		}
	}
};