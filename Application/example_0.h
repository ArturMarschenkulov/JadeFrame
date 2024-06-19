#pragma once
#include "JadeFrame/base_app.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/gui.h>
#include <JadeFrame/utils/utils.h>

namespace JadeFrame {
struct State {
    RenderSystem* m_render_system = nullptr;
    BaseApp*      m_app = nullptr;

    ShaderHandle* flat_shader = nullptr;
    ShaderHandle* flat_shader_0_test_0 = nullptr;
    ShaderHandle* framebuffer_shader_0 = nullptr;

    MaterialHandle* flat_color_mat = nullptr;
    MaterialHandle* flat_color_mat_test_0 = nullptr;
    MaterialHandle* framebuffer_mat = nullptr;

    VertexData rectangle_vd;
    VertexData rectangle_vd_1;

    GPUMeshData* rectangle_mesh = nullptr;
    GPUMeshData* rectangle_mesh_1 = nullptr;
};

static State g_state;

struct Drop {
    Drop() {
        BaseApp*  app = g_state.m_app;
        const f32 window_width = app->m_current_window_p->get_size().x;

        x = static_cast<f32>(get_random_number(0, window_width));
        const f32 rando = static_cast<f32>(get_random_number(1, 30));
        y_speed = static_cast<f32>(map_range(rando, 1, 30, 1, 3));

        obj.m_transform =
            Matrix4x4::scale({10.0F, 80.0F, 1.0F}) * Matrix4x4::translation({x, y, 0.0F});

        VertexData& m = g_state.rectangle_vd;
        for (auto& position : m.m_positions) {
            m.m_colors.emplace_back(138_u8, 43_u8, 226_u8, 255_u8);
        }
        // set_color({ 138_u8, 43_u8, 226_u8, 255_u8 });

        obj.m_vertex_data = &g_state.rectangle_vd;
        obj.m_mesh = g_state.rectangle_mesh;
        obj.m_material = g_state.flat_color_mat;
    }

    auto fall() -> void {
        BaseApp* app = g_state.m_app;
        y = y + y_speed;
        obj.m_transform =
            Matrix4x4::scale({10.0F, 80.0F, 1.0F}) * Matrix4x4::translation({x, y, 0.0F});
        const f32 window_height = app->m_current_window_p->get_size().y;
        if (y >= window_height) { y = -100; }
    }

    auto show() const -> void {
        BaseApp*   app = g_state.m_app;
        IRenderer* renderer = app->m_render_system.m_renderer;
        g_state.m_render_system->submit(obj);
    }

    Object obj = {};
    f32    x = 10;
    f32    y = 10;
    f32    y_speed = 1;
};

struct Checkerbox {
    Checkerbox(f32 size, v2 pos) {
        BaseApp*  app = g_state.m_app;
        const f32 window_width = app->m_current_window_p->get_size().x;
        x = pos.x;
        y = pos.y;
        obj.m_transform = Matrix4x4::scale({size, size, 1.0f}) *
                          Matrix4x4::translation({pos.x, pos.y, 0.0f});

        VertexData& m = g_state.rectangle_vd;
        // VertexData& m = app->m_resources.get_mesh("rectangle"
        // ); // .set_color({ 138_u8, 43_u8, 226_u8, 255_u8 });
        for (auto& position : m.m_positions) {
            m.m_colors.emplace_back(138_u8, 43_u8, 226_u8, 255_u8);
        }

        obj.m_vertex_data = &g_state.rectangle_vd;
        obj.m_mesh = g_state.rectangle_mesh;
        obj.m_material = g_state.flat_color_mat;
    }

    auto show() const -> void {
        // auto* renderer = Instance::get_singleton()->m_apps[0]->m_render_system;
        g_state.m_render_system->submit(obj);
    }

    Object obj = {};
    f32    x = 10;
    f32    y = 10;
};

struct Thingy {
    Thingy() {
        BaseApp*  app = g_state.m_app;
        const f32 window_width = app->m_current_window_p->get_size().x;

        pos.x = static_cast<f32>(get_random_number(0, window_width));

        obj.m_transform = Matrix4x4::scale({1.0F, 1.0F, 1.0F}) *
                          Matrix4x4::translation({pos.x, pos.y, 0.0f});

        VertexData& m = g_state.rectangle_vd_1;
        for (auto& position : m.m_positions) {
            m.m_colors.emplace_back(138_u8, 43_u8, 226_u8, 255_u8);
        }
        obj.m_vertex_data = &g_state.rectangle_vd_1;
        obj.m_mesh = g_state.rectangle_mesh_1;
        obj.m_material = g_state.flat_color_mat_test_0;
    }

    auto update() -> void {
        BaseApp* app = Instance::get_singleton()->m_current_app_p;
        auto&    im = Instance::get_singleton()->m_input_manager;
        // const v2 mp = im.get_mouse_position();

        // obj.m_transform = Matrix4x4::scale({10.0f, 10.0f, 1.0f}) *
        //                   Matrix4x4::translation({mp.x, mp.y, 0.0f});
    }

    Object obj = {};
    v2     pos;
};

struct Example_0 : public BaseApp {
    Example_0(const Desc& desc);
    virtual ~Example_0() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    std::deque<Drop>       drops;
    std::deque<Thingy>     thingies;
    std::deque<Checkerbox> m_checkerbox;
};

Example_0::Example_0(const Desc& desc)
    : BaseApp(desc) {
    g_state.m_render_system = &m_render_system;
    g_state.m_app = this;
}

auto Example_0::on_init() -> void {
    m_render_system.m_renderer->set_clear_color({230_u8, 230_u8, 250_u8, 253_u8});

    // Set Up Camera
    m_camera.orthographic_mode(
        0, m_windows[0]->get_size().x, m_windows[0]->get_size().y, 0, -1, 1
    );

    // Load Resources
    {
        const char* wall_picture_path =
            "/home/artur/dev/proj/Jadeframe/JadeFrame/resource/wall.jpg";
        auto wall_image = Image::load_from_path(wall_picture_path);
        auto wall_texture = m_render_system.register_texture(wall_image);
        auto flat_0_code = GLSLCodeLoader::get_by_name("with_texture_0");

        ShaderHandle::Desc sh_0;
        sh_0.shading_code = flat_0_code;
        sh_0.vertex_format = VertexFormat{
            {     "v_position", SHADER_TYPE::V_3_F32},
            {        "v_color", SHADER_TYPE::V_4_F32},
            {"v_texture_coord", SHADER_TYPE::V_2_F32},
        };

        auto* flat_shader = m_render_system.register_shader(sh_0);
        auto* flat_shader_0_test_0 = m_render_system.register_shader(sh_0);
        auto* framebuffer_shader_0 = m_render_system.register_shader(sh_0);

        auto* flat_color_mat =
            m_render_system.register_material(flat_shader, wall_texture);
        auto* flat_color_mat_test_0 =
            m_render_system.register_material(flat_shader_0_test_0, wall_texture);
        auto* framebuffer_mat =
            m_render_system.register_material(framebuffer_shader_0, wall_texture);
        VertexData::Desc vdf_desc;
        vdf_desc.has_normals = false;
        VertexData rectangle_vd =
            VertexData::make_rectangle({0.0F, 0.0F, 0.0F}, {1.0F, 1.0F, 0.0F}, vdf_desc);

        auto* rectangle_mesh = m_render_system.register_mesh(rectangle_vd);

        VertexData rectangle_vd_1 = VertexData::make_rectangle(
            {0.0F, 0.0F, 0.0F}, {40.0F, 10.0F, 0.0F}, vdf_desc
        );
        auto* rectangle_mesh_1 = m_render_system.register_mesh(rectangle_vd_1);

        g_state.flat_shader = flat_shader;
        g_state.flat_shader_0_test_0 = flat_shader_0_test_0;
        g_state.framebuffer_shader_0 = framebuffer_shader_0;

        g_state.flat_color_mat = flat_color_mat;
        g_state.flat_color_mat_test_0 = flat_color_mat_test_0;
        g_state.framebuffer_mat = framebuffer_mat;

        g_state.rectangle_vd = rectangle_vd;
        g_state.rectangle_vd_1 = rectangle_vd_1;

        g_state.rectangle_mesh = rectangle_mesh;
        g_state.rectangle_mesh_1 = rectangle_mesh_1;

        thingies.emplace_back();
        thingies[0].obj.m_material = flat_color_mat_test_0;
        thingies[0].obj.m_mesh = rectangle_mesh_1;
        thingies[0].obj.m_vertex_data = &rectangle_vd_1;
        thingies[0].obj.m_transform = Matrix4x4::scale({10.0f, 10.0f, 1.0f}) *
                                      Matrix4x4::translation({0.0f, 0.0f, 0.0f});

        for (u32 i = 0; i < 2; i++) { drops.emplace_back(); }

        {
            const i32 amount = 10;
            const i32 size = 30;
            for (u32 i = 0; i < amount; i++) {
                for (u32 j = 0; j < amount; j++) {
                    if ((i + j) % 2 == 0) {
                        m_checkerbox.emplace_back(size, v2(i * size, j * size));
                    }
                }
            }
        }
    }
}

auto Example_0::on_update() -> void {
    // m_camera.control();
    // camera_control(&m_camera);

    thingies[0].update();
    for (u32 i = 0; i < drops.size(); i++) { drops[i].fall(); }

    if (Instance::get_singleton()->m_input_manager.is_key_released(KEY::P)) {
        // std::thread t(&Renderer::take_screenshot, &m_renderer);
        m_render_system.m_renderer->take_screenshot("im.png");
    }
}

auto Example_0::on_draw() -> void {

    for (u32 i = 0; i < drops.size(); i++) {
        // drops[i].show();
        m_render_system.submit(drops[i].obj);
    }
    for (u32 i = 0; i < m_checkerbox.size(); i++) {
        // m_checkerbox[i].show();
        m_render_system.submit(m_checkerbox[i].obj);
    }
    m_render_system.submit(thingies[0].obj);

    // draw_GUI(*this);
}

using TestApp = Example_0;
} // namespace JadeFrame