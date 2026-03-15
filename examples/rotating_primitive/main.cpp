#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/types.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Rotating_Primitive {
    auto on_init(jf::Application& app) -> void {
        app.m_render_system.m_renderer->set_clear_color(
            jf::RGBAColor::from_rgba(0.2f, 0.0f, 0.0f, 1.0f)
        );
        // m_camera.perspective_mode({1.0, 1.0, 1.0}, 1.0f, 0.1f, 100.0f);
        app.m_camera = jf::Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);

        jf::ShaderHandle::Desc shader_desc;
        shader_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");
        jf::ShaderHandle*   shader = app.m_render_system.register_shader(shader_desc);
        jf::MaterialHandle* material =
            app.m_render_system.register_material(shader, nullptr);

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
        mesh_rainbow->m_attributes[jf::Mesh::POSITION.m_id] = jf::Mesh::AttributeData{
            .m_attribute = jf::Mesh::POSITION, .m_data = jf::to_list(positions)
        };
        mesh_rainbow->m_attributes[jf::Mesh::COLOR.m_id] = jf::Mesh::AttributeData{
            .m_attribute = jf::Mesh::COLOR, .m_data = jf::to_list(colors)
        };

        jf::GPUMeshData* mesh = app.m_render_system.register_mesh(*mesh_rainbow);

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
        mesh_yellow->m_attributes[jf::Mesh::POSITION.m_id] = jf::Mesh::AttributeData{
            .m_attribute = jf::Mesh::POSITION, .m_data = jf::to_list(positions_2)
        };
        mesh_yellow->m_attributes[jf::Mesh::COLOR.m_id] = jf::Mesh::AttributeData{
            .m_attribute = jf::Mesh::COLOR, .m_data = jf::to_list(colors_2)
        };
        jf::GPUMeshData* mesh_2 = app.m_render_system.register_mesh(*mesh_yellow);

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

    auto on_update(jf::Application& app) -> void {

        using namespace std::chrono;
        static auto start_time = high_resolution_clock::now();

        auto    current_time = high_resolution_clock::now();
        jf::f32 time =
            duration<jf::f32, seconds::period>(current_time - start_time).count();

        m_tri_rainbow.m_transform.m_rotation =
            jf::mat4x4::rotation_rh(time * jf::to_radians(90.0F), jf::v3::Z());
        m_tri_yellow.m_transform.m_rotation =
            jf::mat4x4::rotation_rh(time * jf::to_radians(45.0F), jf::v3::Z());
    }

    auto on_draw(jf::Application& app) -> void {
        app.m_render_system.submit(m_tri_rainbow); // rainbow triangle
        app.m_render_system.submit(m_tri_yellow);  // yellow triangle
    }

public:
    jf::Object m_tri_rainbow;
    jf::Object m_tri_yellow;
};

auto main() -> int {
    jf::Instance jade_frame;
    jf::Application::Desc app_desc;
    app_desc.title = "Rotating Primitive Example";
    app_desc.size = jf::v2u32::create(800, 800);
    app_desc.api = jf::GRAPHICS_API::OPENGL;

    jf::Application* app = jade_frame.request_application(app_desc);
    Example_Rotating_Primitive example_app;
    app->m_on_init_fn = [&]() -> void { example_app.on_init(*app); };
    app->m_on_update_fn = [&]() -> void { example_app.on_update(*app); };
    app->m_on_draw_fn = [&]() -> void { example_app.on_draw(*app); };
    app->start();
    return 0;
}