
#include "JadeFrame/graphics/camera.h"
#include <JadeFrame.h>


#include <filesystem>

namespace jf = JadeFrame;

struct Example_Texture_Quad : public jf::BaseApp {
    Example_Texture_Quad(const Desc& desc);
    virtual ~Example_Texture_Quad() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    jf::Object         m_obj;
    jf::MaterialHandle m_material;
};

Example_Texture_Quad::Example_Texture_Quad(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Texture_Quad::on_init() -> void {
    using namespace jf::literals;
    m_render_system.m_renderer->set_clear_color({50_u8, 230_u8, 250_u8, 253_u8});
    // m_camera.orthographic_mode(0, m_windows[0]->get_size().x,,
    // m_windows[0]->get_size().y, 0, -1, 1);
    m_camera = jf::Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);

    jf::ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("with_texture_0");

    auto* shader = m_render_system.register_shader(shader_handle_desc);

    const jf::f32 s = 0.5F;
    auto*         vertex_data = new jf::VertexData();

    vertex_data->m_positions = std::vector<jf::v3>{
        jf::v3::create(-s, +s, 0.0F),
        jf::v3::create(-s, -s, 0.0F),
        jf::v3::create(+s, +s, 0.0F)
    };
    vertex_data->m_texture_coordinates = {
        jf::v2::create(-1, +1),
        jf::v2::create(+1, +1),
        jf::v2::create(-1, -1),
    };
    vertex_data->m_colors = {
        jf::RGBAColor::solid_black().set_opacity(0.1F), //
        jf::RGBAColor::solid_black().set_opacity(0.1F), //
        jf::RGBAColor::solid_black().set_opacity(0.1F)  //
    };
    vertex_data->m_normals = {};
    m_obj.m_vertex_data = vertex_data;

    auto* mesh = m_render_system.register_mesh(*m_obj.m_vertex_data);
    m_obj.m_mesh = mesh;

    namespace fs = std::filesystem;
    auto texture_path = fs::path("resource") / "wall.jpg";

    auto  img = jf::Image::load_from_path(texture_path.string());
    auto* texture = m_render_system.register_texture(img);
    auto* material = m_render_system.register_material(shader, texture);

    m_obj.m_material = material;
    m_obj.m_transform = jf::mat4x4::identity();
}

auto Example_Texture_Quad::on_update() -> void {}

auto Example_Texture_Quad::on_draw() -> void { m_render_system.submit(m_obj); }

using TestApp = Example_Texture_Quad;

int main() {
    jf::Instance jade_frame;

    using GApp = TestApp;
    GApp::Desc win_desc;
    win_desc.title = "Test";
    win_desc.size.x = 800; // = 1280;
    win_desc.size.y = 800; // = 720;
    win_desc.api = jf::GRAPHICS_API::OPENGL;

    GApp* app = jade_frame.request_app<GApp>(win_desc);
    jade_frame.run();
    return 0;
}