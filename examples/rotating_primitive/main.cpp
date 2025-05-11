#include "JadeFrame/graphics/graphics_shared.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Rotating_Primitive : public jf::BaseApp {

    Example_Rotating_Primitive(const Desc& desc);
    virtual ~Example_Rotating_Primitive() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    jf::Object m_tri_rainbow;
    jf::Object m_tri_yellow;
};

inline Example_Rotating_Primitive::Example_Rotating_Primitive(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Rotating_Primitive::on_init() -> void {
    m_render_system.m_renderer->set_clear_color(
        jf::RGBAColor::from_rgba(0.2f, 0.0f, 0.0f, 1.0f)
    );
    // m_camera.perspective_mode({1.0, 1.0, 1.0}, 1.0f, 0.1f, 100.0f);
    m_camera = jf::Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);

    jf::ShaderHandle::Desc shader_desc;
    shader_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");
    jf::ShaderHandle*   shader = m_render_system.register_shader(shader_desc);
    jf::MaterialHandle* material = m_render_system.register_material(shader, nullptr);

    const jf::f32 s = 0.5F;
    const jf::f32 opacity = 0.1F;
    auto*         mesh_rainbow = new jf::Mesh;
    auto          positions = std::vector<jf::v3>{
        jf::v3::create(-s, +s, -0.01F),
        jf::v3::create(-s, -s, -0.01F),
        jf::v3::create(+s, +s, -0.01F)
    };

    auto colors = {
        jf::RGBAColor::solid_red().set_opacity(opacity),
        jf::RGBAColor::solid_green().set_opacity(opacity),
        jf::RGBAColor::solid_blue(),
    };
    mesh_rainbow->m_attributes[jf::Mesh::POSITION.m_id] =
        jf::Mesh::AttributeData{jf::Mesh::POSITION, jf::to_list(positions)};
    mesh_rainbow->m_attributes[jf::Mesh::COLOR.m_id] =
        jf::Mesh::AttributeData{jf::Mesh::COLOR, jf::to_list(colors)};

    jf::GPUMeshData* mesh = m_render_system.register_mesh(*mesh_rainbow);

    auto* mesh_yellow = new jf::Mesh();
    auto  positions_2 = std::vector<jf::v3>{
        jf::v3::create(-s, +s + 0.1F, -0.02F),
        jf::v3::create(-s, -s, -0.02F),
        jf::v3::create(+s, +s, -0.02F)
    };
    auto colors_2 = {
        jf::RGBAColor::solid_yellow(),
        jf::RGBAColor::solid_yellow().set_opacity(opacity),
        jf::RGBAColor::solid_black(),
    };
    mesh_yellow->m_attributes[jf::Mesh::POSITION.m_id] =
        jf::Mesh::AttributeData{jf::Mesh::POSITION, jf::to_list(positions_2)};
    mesh_yellow->m_attributes[jf::Mesh::COLOR.m_id] =
        jf::Mesh::AttributeData{jf::Mesh::COLOR, jf::to_list(colors_2)};
    jf::GPUMeshData* mesh_2 = m_render_system.register_mesh(*mesh_yellow);

    m_tri_rainbow.m_mesh = mesh;
    m_tri_rainbow.m_vertex_data = mesh_rainbow;
    m_tri_rainbow.m_material = material;
    m_tri_rainbow.m_transform = {};

    m_tri_yellow.m_material = material;
    m_tri_yellow.m_vertex_data = mesh_yellow;
    m_tri_yellow.m_mesh = mesh_2;
    m_tri_yellow.m_transform = {};
    jf::Logger::info("Example_Rotating_Primitive initialized");
}

auto Example_Rotating_Primitive::on_update() -> void {

    using namespace std::chrono;
    static auto start_time = high_resolution_clock::now();

    auto    current_time = high_resolution_clock::now();
    jf::f32 time = duration<jf::f32, seconds::period>(current_time - start_time).count();

    m_tri_rainbow.m_transform.m_rotation =
        jf::mat4x4::rotation_rh(time * jf::to_radians(90.0F), jf::v3::Z());
    m_tri_yellow.m_transform.m_rotation =
        jf::mat4x4::rotation_rh(time * jf::to_radians(45.0F), jf::v3::Z());
}

auto Example_Rotating_Primitive::on_draw() -> void {
    m_render_system.submit(m_tri_rainbow); // rainbow triangle
    m_render_system.submit(m_tri_yellow);  // yellow triangle
}

using TestApp = Example_Rotating_Primitive;

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