#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/camera.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/utils/logger.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/gui.h>
#include <JadeFrame/utils/utils.h>

#include <filesystem>

namespace jf = JadeFrame;

struct State {
    jf::RenderSystem* m_render_system = nullptr;
    jf::BaseApp*      m_app = nullptr;

    // jf::ShaderHandle* shader_texture = nullptr;

    jf::MaterialHandle* material_texture_face = nullptr;
    jf::MaterialHandle* material_texture_wall = nullptr;
    jf::MaterialHandle* material_color_flat = nullptr;

    jf::Mesh rectangle_vd;

    jf::GPUMeshData* rectangle_mesh = nullptr;
};

static State g_state;

static auto draw_rectangle(
    jf::f32             x,
    jf::f32             y,
    jf::f32             width,
    jf::f32             height,
    jf::MaterialHandle* material
) -> void {
    jf::mat4x4 scale = jf::mat4x4::scale(jf::v3::create(width, height, 1.0F));
    jf::mat4x4 trans = jf::mat4x4::translation(jf::v3::create(x, y, 0.0F));

    jf::Object obj;
    obj.m_transform.m_translation = trans;
    obj.m_transform.m_scale = scale;

    jf::Mesh& m = g_state.rectangle_vd;
    m.set_color(jf::RGBAColor::from_rgba_u32(138, 43, 226, 255));
    obj.m_mesh = g_state.rectangle_mesh;

    obj.m_vertex_data = &g_state.rectangle_vd;
    obj.m_material = material;

    g_state.m_render_system->submit(obj);
}

struct Drop {
    Drop()
        : width(10.0F)
        , height(80.0F) {
        jf::BaseApp*  app = g_state.m_app;
        const jf::f32 window_width = app->m_current_window_p->get_size().x;

        x = static_cast<jf::f32>(jf::get_random_number(0, window_width));
        const jf::f32 rando = static_cast<jf::f32>(jf::get_random_number(1, 30));
        y_speed = static_cast<jf::f32>(jf::map_range(rando, 1, 30, 1, 3));
    }

    auto fall() -> void {
        const jf::BaseApp* app = g_state.m_app;
        y = y + y_speed;

        const jf::f32 window_height = app->m_current_window_p->get_size().y;
        if (y >= window_height) {
            const jf::i32 restart_hight = -100;
            y = restart_hight;
        }
    }

    auto show() -> void {
        draw_rectangle(x, y, width, height, g_state.material_texture_wall);
    }

    jf::f32 x = 10;
    jf::f32 y = 10;
    jf::f32 width = 0;
    jf::f32 height = 0;
    jf::f32 y_speed = 1;
};

struct Checkerbox {
    Checkerbox(jf::f32 size, jf::v2 pos)
        : x(pos.x)
        , y(pos.y)
        , m_size(size) {

        jf::BaseApp*  app = g_state.m_app;
        const jf::f32 window_width = app->m_current_window_p->get_size().x;
    }

    auto show() -> void {
        draw_rectangle(x, y, m_size, m_size, g_state.material_texture_face);
    }

    jf::Object obj = {};
    jf::f32    x = 10;
    jf::f32    y = 10;
    jf::f32    m_size = 0;
};

struct Coordinates {

    Coordinates(jf::v2 pos)
        : pos(pos) {}

    auto show() -> void {
        draw_rectangle(pos.x, pos.y, 10, 10, g_state.material_color_flat);
    }

    jf::v2 pos;
};

struct Example_0 : public jf::BaseApp {

    Example_0(const Desc& desc);
    virtual ~Example_0() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    std::deque<Drop>       drops;
    std::deque<Checkerbox> m_checkerbox;
    Coordinates            m_coordinates;
};

Example_0::Example_0(const Desc& desc)
    : BaseApp(desc)
    , m_coordinates(jf::v2::create(10, 10)) {
    g_state.m_render_system = &m_render_system;
    g_state.m_app = this;
}

auto Example_0::on_init() -> void {
    m_render_system.m_renderer->set_clear_color(
        jf::RGBAColor::from_rgba_u32(230, 230, 250, 253)
    );

    // Set Up Camera
    m_camera = jf::Camera::orthographic(
        0, m_windows[0]->get_size().x, m_windows[0]->get_size().y, 0, -1, 1
    );

    // m_camera = jf::Camera::perspective(jf::v3::zero(), 1.0f, win_width / win_height,
    // 0.1, 100);

    // Load Resources
    {
        namespace fs = std::filesystem;
        fs::path path_picture_face = fs::path("resource") / "awesomeface.png";
        fs::path wall_picture_path = fs::path("resource") / "wall.jpg";

        jf::Image image_face = jf::Image::load_from_path(path_picture_face.string());
        jf::Image wall_image = jf::Image::load_from_path(wall_picture_path.string());

        jf::TextureHandle* texture_face = m_render_system.register_texture(image_face);
        jf::TextureHandle* texture_wall = m_render_system.register_texture(wall_image);

        jf::Logger::warn(" ----- Texture loaded");

        jf::ShaderHandle::Desc sh_0;
        sh_0.shading_code = jf::GLSLCodeLoader::get_by_name("with_texture_0");
        jf::ShaderHandle* shader_texture = m_render_system.register_shader(sh_0);

        jf::ShaderHandle::Desc sh_1;
        sh_1.shading_code = jf::GLSLCodeLoader::get_by_name("flat_0");
        jf::ShaderHandle* shader_color_flat = m_render_system.register_shader(sh_1);

        jf::Logger::warn(" ----- Shader loaded");
        jf::MaterialHandle* material_texture_face =
            m_render_system.register_material(shader_texture, texture_face);
        jf::MaterialHandle* material_texture_wall =
            m_render_system.register_material(shader_texture, texture_wall);
        jf::MaterialHandle* material_color_flat =
            m_render_system.register_material(shader_color_flat, nullptr);

        jf::Mesh::Desc vdf_desc;
        vdf_desc.has_normals = false;
        jf::Mesh rectangle_vd = jf::Mesh::rectangle(
            jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc
        );
        rectangle_vd.set_color(jf::RGBAColor::solid_blue());
        jf::GPUMeshData* rectangle_mesh = m_render_system.register_mesh(rectangle_vd);

        // g_state.shader_texture = shader_texture;
        g_state.material_texture_face = material_texture_face;
        g_state.material_texture_wall = material_texture_wall;
        g_state.material_color_flat = material_color_flat;
        g_state.rectangle_vd = rectangle_vd;
        g_state.rectangle_mesh = rectangle_mesh;

        for (jf::u32 i = 0; i < 2; i++) { drops.emplace_back(); }

        {
            const jf::i32 amount = 10;
            const jf::i32 size = 30;
            for (jf::u32 i = 0; i < amount; i++) {
                for (jf::u32 j = 0; j < amount; j++) {
                    if ((i + j) % 2 == 0) {
                        m_checkerbox.emplace_back(
                            size, jf::v2::create(i * size, j * size)
                        );
                    }
                }
            }
        }
    }
}

auto Example_0::on_update() -> void {
    // m_camera.control();
    // camera_control(&m_camera);

    for (jf::u32 i = 0; i < drops.size(); i++) { drops[i].fall(); }
    const jf::InputState& input =
        jf::Instance::get_singleton()->m_current_app_p->m_windows[0]->m_input_state;
    if (input.is_key_released(jf::KEY::P)) {
        // std::thread t(&Renderer::take_screenshot, &m_renderer);
        m_render_system.m_renderer->take_screenshot("im.png");
    }
}

auto Example_0::on_draw() -> void {

    for (jf::u32 i = 0; i < drops.size(); i++) { drops[i].show(); }
    for (jf::u32 i = 0; i < m_checkerbox.size(); i++) { m_checkerbox[i].show(); }
    // m_render_system.submit(thingies[0].obj);
    m_coordinates.show();
    // draw_GUI(*this);
}

using TestApp = Example_0;

int main() {
    jf::Instance jade_frame;

    using GApp = TestApp;
    GApp::Desc win_desc;
    win_desc.title = "Test";
    win_desc.size.x = 800; // = 1280;
    win_desc.size.y = 800; // = 720;
    win_desc.api = JadeFrame::GRAPHICS_API::VULKAN;

    GApp* app = jade_frame.request_app<GApp>(win_desc);
    jade_frame.run();
    return 0;
}