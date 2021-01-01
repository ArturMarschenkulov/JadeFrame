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
			m_has_performance_counter = true;
			m_frequency = frequency;
		} else {
			m_has_performance_counter = true;
			m_frequency = 1000;
		}

		m_offset = get_timer_value();
	}

	auto get_time() {
		return static_cast<double>(get_timer_value() - m_offset) / get_timer_frequency();
	}

private:
	auto get_timer_value() -> uint64_t {
		uint64_t value;
		QueryPerformanceCounter((LARGE_INTEGER*)&value);
		return value;
	}
	auto get_timer_frequency() -> uint64_t {
		return m_frequency;
	}
private:
	bool m_has_performance_counter;
	uint64_t m_frequency;
	uint64_t m_offset;
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
	Camera1 m_camera;

	//CachedData m_cached_data;
	//std::vector<Object> m_objects;

	bool m_is_running = true;
	float m_last_frame_time = 0.0f;

};

struct TestApp : public BaseApp {
	//Camera1 m_camera;

	GLShader m_shader[2]; // 0 = flat, 1 = textures
	GLTexture m_textures[2];
	Mesh m_meshes[6] = {};
	std::vector<Object> m_objs;

	TestApp(const std::string& title, Vec2 size, Vec2 position = { -1, -1 });

	virtual auto init() -> void override;
	virtual auto update() -> void override;
	virtual auto draw() -> void override;
};