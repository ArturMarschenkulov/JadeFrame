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

#include <deque>

/*
	The struct JadeFrame should act more or less like the ultimate global scope of JadeFrame.
	In here goes everything which would not make sense to be part of the application itself.

	It also allows an abtraction, which may allow to have several applications at the same time in a JadeFrame context.
*/

class BaseApp;
class JadeFrame {
public:
	JadeFrame(const JadeFrame&) = delete;
	JadeFrame(JadeFrame&&) = delete;
	auto operator=(const JadeFrame&) -> JadeFrame& = delete;
	auto operator=(JadeFrame&&) -> JadeFrame& = delete;

	JadeFrame();
	auto run() -> void;
	auto add(BaseApp* app) -> void;
	static auto get_singleton() -> JadeFrame*;

public:
	SystemManager m_system_manager;
	InputManager m_input_manager;

	std::deque<BaseApp*> m_apps;
	BaseApp* m_current_app_p = nullptr;

	static JadeFrame* m_singleton;
};

#include "graphics/camera.h"
#include <map>
#include "graphics/opengl/opengl_renderer.h"
#include "graphics/vulkan/vulkan_renderer.h"

using Window = Windows_Window;
using Renderer = OpenGL_Renderer;

/*
	This is the storage of various resources which should be accessible "globally" in JadeFrame.
	TODO: Think whether std::unordered_map is the best way to store it. 
		Technically, it shoud be fine as accessing should not be a frequent thing.

	This struct should be used for all "default" stuff and should be able to have custom stuff
*/
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

	auto set_texture(const std::string& name, OpenGL_Texture&& texture) -> void {
		//m_textures.insert(std::pair<std::string, OpenGL_Texture>(name, texture));
		m_textures.emplace(name, std::move(texture));

	}
	auto get_texture(const std::string& name) -> OpenGL_Texture& {
		if (m_textures.contains(name)) {
			return m_textures.at(name);
		}
		__debugbreak();
		return m_textures.at(name);
	}

	auto set_material(const std::string& material_name, const std::string& shader_name, const std::string& texture_name) -> void {
		if (m_shaders.contains(shader_name)) {
			m_materials[material_name].m_shader = &m_shaders[shader_name];
			if (m_textures.contains(texture_name)) {
				m_materials[material_name].m_texture = &m_textures[texture_name];
				return;
			} else if (texture_name == "") {
				return;
			}
		}
		__debugbreak();

	}
	auto set_material(const std::string& material_name, OpenGL_Shader& shader, OpenGL_Texture& texture) -> void {
		m_materials[material_name].m_shader = &shader;
		m_materials[material_name].m_texture = &texture;
		//__debugbreak();
	}

	auto get_material(const std::string& name) -> Material& {
		if (m_materials.contains(name)) {
			return m_materials.at(name);
		}
		__debugbreak();
		return m_materials.at(name);
	}

	auto set_mesh(const std::string& name, const Mesh& mesh) -> void {
		m_meshes.emplace(name, std::move(mesh));
	}
	auto get_mesh(const std::string& name) -> Mesh& {
		if(m_meshes.contains(name)) {
			return m_meshes.at(name);
		}
		__debugbreak();
		return m_meshes.at(name);
	}
private:
	std::unordered_map<std::string, OpenGL_Shader> m_shaders;
	std::unordered_map<std::string, OpenGL_Texture> m_textures;
	std::unordered_map<std::string, Material> m_materials;
	std::unordered_map<std::string, Mesh> m_meshes;
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
	//Window stuff
	i32 m_window_counter = 0;
	using WindowID = i32;
	std::map<WindowID, Window> m_windows;
	Window* m_main_window_p = nullptr;
	Window* m_current_window_p = nullptr;


	BaseApp* m_current_app_p = nullptr;

	Renderer m_renderer;
	Vulkan_Renderer m_vulkan_renderer;
	Camera1 m_camera;

	bool m_is_running = true;

	TimeManager m_time_manager;

	ResourceStorage m_resources;
};





/*
	********************
	*
	*
	CLIENT PLACEHOLDER
	*
	*
	********************
*/

//namespace Test1 {
//struct TestApp0 : public BaseApp {
//	TestApp0(const std::string& title, const Vec2& size, const Vec2& position = { -1, -1 });
//	virtual ~TestApp0() = default;
//
//	virtual auto on_init() -> void override;
//	virtual auto on_update() -> void override;
//	virtual auto on_draw() -> void override;
//
//public:
//	ResourceStorage m_resources;
//	std::deque<Mesh> m_meshes;
//	std::vector<Object> m_objs;
//};
//}
//using TestApp = Test1::TestApp0;