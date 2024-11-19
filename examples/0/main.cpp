#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/mesh.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/gui.h>
#include <JadeFrame/utils/utils.h>

#include <filesystem>

namespace jf = JadeFrame;

struct State {
    jf::RenderSystem* m_render_system = nullptr;
    jf::BaseApp*      m_app = nullptr;

    jf::ShaderHandle* flat_shader = nullptr;

    jf::MaterialHandle* flat_color_mat = nullptr;

    jf::VertexData rectangle_vd;

    jf::GPUMeshData* rectangle_mesh = nullptr;
};

static State g_state;

static auto draw_rectangle(jf::f32 x, jf::f32 y, jf::f32 width, jf::f32 height) -> void {
    auto scale = jf::mat4x4::scale(jf::v3::create(width, height, 1.0F));
    auto trans = jf::mat4x4::translation(jf::v3::create(x, y, 0.0F));

    jf::Object obj;
    obj.m_transform = scale * trans;

    jf::VertexData& m = g_state.rectangle_vd;
    m.set_color(jf::RGBAColor::from_rgba_u32(138, 43, 226, 255));
    obj.m_mesh = g_state.rectangle_mesh;

    obj.m_vertex_data = &g_state.rectangle_vd;
    obj.m_material = g_state.flat_color_mat;

    g_state.m_render_system->submit(obj);
}

struct Drop {
    Drop()
        : width(10.0F)
        , height(80.0F) {
        jf::BaseApp* app = g_state.m_app;
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

    auto show() -> void { draw_rectangle(x, y, width, height); }

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

        jf::BaseApp* app = g_state.m_app;
        const jf::f32 window_width = app->m_current_window_p->get_size().x;
    }

    auto show() -> void { draw_rectangle(x, y, m_size, m_size); }

    jf::Object obj = {};
    jf::f32    x = 10;
    jf::f32    y = 10;
    jf::f32    m_size = 0;
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
};

Example_0::Example_0(const Desc& desc)
    : BaseApp(desc) {
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

    // Load Resources
    {
        namespace fs = std::filesystem;
        fs::path wall_picture_path = fs::path("resource") / "wall.jpg";

        auto  wall_image = jf::Image::load_from_path(wall_picture_path.string());
        auto* wall_texture = m_render_system.register_texture(wall_image);

        jf::ShaderHandle::Desc sh_0;
        sh_0.shading_code = jf::GLSLCodeLoader::get_by_name("with_texture_0");
        auto* flat_shader = m_render_system.register_shader(sh_0);
        auto* flat_mat = m_render_system.register_material(flat_shader, wall_texture);

        jf::VertexData::Desc vdf_desc;
        vdf_desc.has_normals = false;
        jf::VertexData rectangle_vd = jf::VertexData::rectangle(
            jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc
        );
        rectangle_vd.set_color(jf::RGBAColor::solid_blue());
        auto* rectangle_mesh = m_render_system.register_mesh(rectangle_vd);

        g_state.flat_shader = flat_shader;
        g_state.flat_color_mat = flat_mat;
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
    auto& input = jf::Instance::get_singleton()->m_current_app_p->m_windows[0]->m_input_state;
    if (input.is_key_released(jf::KEY::P)) {
        // std::thread t(&Renderer::take_screenshot, &m_renderer);
        m_render_system.m_renderer->take_screenshot("im.png");
    }
}

auto Example_0::on_draw() -> void {

    for (jf::u32 i = 0; i < drops.size(); i++) { drops[i].show(); }
    for (jf::u32 i = 0; i < m_checkerbox.size(); i++) { m_checkerbox[i].show(); }
    // m_render_system.submit(thingies[0].obj);

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
    win_desc.api = JadeFrame::GRAPHICS_API::OPENGL;

    GApp* app = jade_frame.request_app<GApp>(win_desc);
    jade_frame.run();
    return 0;
}