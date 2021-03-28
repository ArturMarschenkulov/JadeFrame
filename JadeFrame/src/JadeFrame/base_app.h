#pragma once
#if _WIN32 || _WIN64 
#include "platform/windows/windows_time_manager.h"
#include "platform/windows/windows_input_manager.h"
#include "platform/windows/windows_window.h"
#include "platform/windows/windows_system_manager.h"

using SystemManager = Windows_SystemManager;
using TimeManager = Windows_TimeManager;
using InputManager = Windows_InputManager;

#endif 
#include "graphics/camera.h"
#include "graphics/opengl/opengl_renderer.h"

#include <set>
#include <unordered_map>
#include <map>

using GCamera = Camera1; // remove it in a later date. This is only used to experiment with various cameras

class BaseApp;
class JadeFrame {
public:
	JadeFrame();
	auto run() -> void;
	auto add(BaseApp* app) -> void;
	static auto get_singleton()->JadeFrame*;

public:
	SystemManager m_system_manager;
	TimeManager m_time_manager;
	InputManager m_input_manager;

	std::deque<BaseApp*> m_apps;
	BaseApp* m_current_app = nullptr;

	static JadeFrame* m_singleton;
};

class BaseApp {
public:
	BaseApp() = default;
	BaseApp(const std::string& title, const Vec2& size, const Vec2& position = { -1, -1 });
	virtual ~BaseApp() = default;

	virtual auto on_init() -> void = 0;
	virtual auto on_update() -> void = 0;
	virtual auto on_draw() -> void = 0;

	auto start() -> void;
protected:
	auto poll_events() -> void;

public:
	int m_window_counter = 0;
	using WindowID = int;
	std::map<WindowID, Windows_Window> m_windows;
	Windows_Window* m_current_window_p;

	OpenGL_Renderer m_renderer;
	GCamera m_camera;

	bool m_is_running = true;
};


struct ResourceStorage {
public:
	//auto set_shader(const std::string& name, const OpenGL_Shader& shader) -> void {
	//	m_shaders.insert({ name, shader });
	//}
	auto set_shader(const std::string& name, OpenGL_Shader&& shader) -> void {
		//m_shaders.insert({ name, shader });
		m_shaders.emplace(name, std::move(shader));
	}
	auto get_shader(const std::string& name) -> OpenGL_Shader& {
		if (m_shaders.contains(name)) {
			return m_shaders.at(name);
		}
		__debugbreak();
		return m_shaders.at(name);
	}

	auto set_texture(const std::string& name, const GLTexture& texture) -> void {
		m_textures.insert({ name, texture });
	}
	auto get_texture(const std::string& name) -> GLTexture& {
		if (m_textures.contains(name)) {
			return m_textures.at(name);
		}
		__debugbreak();
		return m_textures.at(name);
	}

	auto set_material(const std::string& material_name, const std::string& shader_name, const std::string& texture_name) -> void {
		if (m_shaders.contains(shader_name) && m_textures.contains(texture_name)) {
			m_materials[material_name].m_shader = &m_shaders[shader_name];
			m_materials[material_name].m_texture = &m_textures[texture_name];
			return;
		}
		__debugbreak();
	}
	auto set_material(const std::string& material_name, const OpenGL_Shader& shader, const GLTexture& texture) -> void {
		m_materials[material_name].m_shader = &shader;
		m_materials[material_name].m_texture = &texture;
		__debugbreak();
	}

	auto get_material(const std::string& name) -> Material& {
		if (m_materials.contains(name)) {
			return m_materials.at(name);
		}
		__debugbreak();
		return m_materials.at(name);
	}
private:
	std::unordered_map<std::string, OpenGL_Shader> m_shaders;
	std::unordered_map<std::string, GLTexture> m_textures;
	std::unordered_map<std::string, Material> m_materials;
};

struct TestApp : public BaseApp {
	TestApp(const std::string& title, const Vec2& size, const Vec2& position = { -1, -1 });
	virtual ~TestApp() = default;

	virtual auto on_init() -> void override;
	virtual auto on_update() -> void override;
	virtual auto on_draw() -> void override;

public:
	ResourceStorage m_resources;
	std::deque<Mesh> m_meshes;
	std::vector<Object> m_objs;
};
