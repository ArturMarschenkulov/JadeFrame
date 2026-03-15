#include "JadeFrame/base_app.h"
#include "JadeFrame/math/vec.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Moving_Quad {
    auto on_init(jf::Application& app) -> void {
        app.m_render_system.m_renderer->set_clear_color(
            jf::RGBAColor::from_hex(0x9E1030FF)
        );
        auto win_size = app.m_windows[0]->get_size();
        app.m_camera = jf::Camera::orthographic(
            0.0F,
            static_cast<jf::f32>(win_size.x),
            0.0F,
            static_cast<jf::f32>(win_size.y),
            -1.0F,
            1.0F
        );

        jf::ShaderHandle::Desc shader_handle_desc;
        shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");

        jf::ShaderHandle* shader =
            app.m_render_system.register_shader(shader_handle_desc);
        jf::MaterialHandle* material =
            app.m_render_system.register_material(shader, nullptr);

        jf::MeshBuilder::Desc vdf_desc;
        vdf_desc.has_normals = true;
        auto mesh_builder = jf::Mesh::builder();
        auto rectangle = mesh_builder.rectangle(
            jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc
        );
        rectangle.set_color(jf::RGBAColor::solid_red());

        jf::GPUMeshData* mesh_data = app.m_render_system.register_mesh(rectangle);
    }

    auto on_update(jf::Application& app) -> void {
        auto input_state = app.m_windows[0]->m_input_state;
    }

    auto on_draw(jf::Application& app) -> void {}
};

auto main() -> int {
    jf::Instance          jade_frame;
    jf::Application::Desc app_desc;
    app_desc.title = "Moving Quad Example";
    app_desc.size = jf::v2u32::create(800, 800);
    app_desc.api = jf::GRAPHICS_API::OPENGL;

    jf::Application*    app = jade_frame.request_application(app_desc);
    Example_Moving_Quad example_app;
    app->m_on_init_fn = [&]() -> void { example_app.on_init(*app); };
    app->m_on_update_fn = [&]() -> void { example_app.on_update(*app); };
    app->m_on_draw_fn = [&]() -> void { example_app.on_draw(*app); };
    app->start();
    return 0;
}