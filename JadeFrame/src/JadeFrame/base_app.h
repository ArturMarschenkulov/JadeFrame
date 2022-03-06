#pragma once

#if _WIN32 || _WIN64
#include "platform/windows/windows_input_manager.h"
#include "platform/windows/windows_system_manager.h"
#include "platform/windows/windows_time_manager.h"
#include "platform/windows/windows_window.h"
#elif __linux__
#include "platform/linux/linux_input_manager.h"
#include "platform/linux/linux_system_manager.h"
#include "platform/linux/linux_time_manager.h"
#include "platform/linux/linux_window.h"
#endif

#include "JadeFrame/math/vec.h"
#include "graphics/camera.h"
#include "graphics/graphics_shared.h"
#include "graphics/material_handle.h"
#include "graphics/opengl/opengl_renderer.h"
#include "graphics/vulkan/vulkan_renderer.h"

#include <deque>
#include <map>

namespace JadeFrame {

/*
        The struct JadeFrame should act more or less like the ultimate global
   scope of JadeFrame. In here goes everything which would not make sense to be
   part of the application itself.

        It also allows an abtraction, which may allow to have several
   applications at the same time in a JadeFrame context.
*/

#ifdef _WIN32
using SystemManager = Windows_SystemManager;
using InputManager = Windows_InputManager;
using TimeManager = Windows_TimeManager;
#elif __linux__
using SystemManager = Linux_SystemManager;
using InputManager = Linux_InputManager;
using TimeManager = Linux_TimeManager;
#endif

class BaseApp;
class IRenderer;
class JadeFrameInstance {
  public:
    JadeFrameInstance(const JadeFrameInstance&) = delete;
    JadeFrameInstance(JadeFrameInstance&&) = delete;
    auto operator=(const JadeFrameInstance&) -> JadeFrameInstance& = delete;
    auto operator=(JadeFrameInstance&&) -> JadeFrameInstance& = delete;

    JadeFrameInstance();
    auto        run() -> void;
    auto        add(BaseApp* app) -> void;
    static auto get_singleton() -> JadeFrameInstance*;

  public:
    SystemManager m_system_manager;
    InputManager  m_input_manager;

    std::deque<BaseApp*> m_apps;
    BaseApp*             m_current_app_p = nullptr;

    static JadeFrameInstance* m_singleton;
};

/*
        This is the storage of various resources which should be accessible
   "globally" in JadeFrame.
        TODO: Think whether std::unordered_map is the best way to store it.
                Technically, it shoud be fine as accessing should not be a
   frequent thing.

        This struct should be used for all "default" stuff and should be able to
   have custom stuff
*/
struct ResourceStorage {
  public:
    auto set_shader_handle(const std::string& name, ShaderHandle&& shader)
        -> void {
        // m_shaders.insert({ name, shader });
        m_shader_handles.emplace(name, std::move(shader));
    }
    auto get_shader_handle(const std::string& name) -> ShaderHandle& {
        if (m_shader_handles.find(name) != m_shader_handles.end()) {
            return m_shader_handles.at(name);
        }
        assert(false);
        return m_shader_handles.at(name);
    }

    auto set_texture_handle(const std::string& name, const std::string& path)
        -> void {
        // m_shaders.insert({ name, shader });
        m_texture_handles.emplace(name, path);
        // m_texture_handles.emplace(name, std::move(texture));
    }
    auto get_texture_handle(const std::string& name) -> TextureHandle& {
        if (m_texture_handles.find(name) != m_texture_handles.end()) {
            return m_texture_handles.at(name);
        }
        assert(false);
        return m_texture_handles.at(name);
    }

    auto set_material_handle(
        const std::string& material_name, const std::string& shader_name,
        const std::string& texture_name) -> void {

        if (m_shader_handles.find(shader_name) != m_shader_handles.end()) {
            m_material_handles[material_name].m_shader_handle =
                &m_shader_handles[shader_name];
            if (m_texture_handles.find(texture_name) !=
                m_texture_handles.end()) {
                m_material_handles[material_name].m_texture_handle =
                    &m_texture_handles[texture_name];
                return;
            } else if (texture_name == "") {
                return;
            }
        }
        assert(false);
    }
    auto get_material_handle(const std::string& name) -> MaterialHandle& {
        if (m_material_handles.find(name) != m_material_handles.end()) {
            return m_material_handles.at(name);
        }
        assert(false);
        return m_material_handles.at(name);
    }

    auto set_mesh(const std::string& name, const VertexData& vertex_data)
        -> void {
        m_meshes.emplace(name, std::move(vertex_data));
    }
    auto get_mesh(const std::string& name) -> VertexData& {
        if (m_meshes.find(name) != m_meshes.end()) { return m_meshes.at(name); }
        assert(false);
        return m_meshes.at(name);
    }

  private:
    std::unordered_map<std::string, ShaderHandle>   m_shader_handles;
    std::unordered_map<std::string, TextureHandle>  m_texture_handles;
    std::unordered_map<std::string, MaterialHandle> m_material_handles;
    std::unordered_map<std::string, VertexData>     m_meshes;
};

class BaseApp {
  public:
    struct DESC {
        std::string title;
        v2u32       size;
        v2u32       position = {0, 0};
    };
    BaseApp() = default;
    BaseApp(const DESC& desc);
    virtual ~BaseApp() = default;

    virtual auto on_init() -> void = 0;
    virtual auto on_update() -> void = 0;
    virtual auto on_draw() -> void = 0;

    auto start() -> void;
    // protected:
    auto poll_events() -> void;

  public:
    bool m_is_running = true;

    // Window stuff
    i32 m_window_counter = 0;
    using WindowID = i32;
    std::map<WindowID, Window> m_windows;
    Window*                    m_current_window_p = nullptr;

    IRenderer* m_renderer = nullptr;
    Camera1    m_camera;

    TimeManager m_time_manager;

    ResourceStorage m_resources;
};

} // namespace JadeFrame

/*
        ********************
        *
        *
        CLIENT PLACEHOLDER
        *
        *
        ********************
*/

// namespace Test1 {
// struct TestApp0 : public BaseApp {
//	TestApp0(const std::string& title, const Vec2& size, const Vec2&
// position = { -1, -1 }); 	virtual ~TestApp0() = default;
//
//	virtual auto on_init() -> void override;
//	virtual auto on_update() -> void override;
//	virtual auto on_draw() -> void override;
//
// public:
//	ResourceStorage m_resources;
//	std::deque<Mesh> m_meshes;
//	std::vector<Object> m_objs;
// };
// }
// using TestApp = Test1::TestApp0;