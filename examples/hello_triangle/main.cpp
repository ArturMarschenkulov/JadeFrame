#include "JadeFrame/base_app.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Hello_Triangle {
    auto on_init(jf::Application& app) -> void {
        app.m_render_system.m_renderer->set_clear_color(jf::RGBAColor::solid_black());
        // app.m_camera.perspective_mode({1.0, 1.0, 1.0F}, 1.0f, 0.1f, 100.0f);
        app.m_camera = jf::Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);

        jf::ShaderHandle::Desc shader_handle_desc;
        shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");
        jf::ShaderHandle* shader =
            app.m_render_system.register_shader(shader_handle_desc);
        jf::MaterialHandle* material =
            app.m_render_system.register_material(shader, nullptr);

        const jf::f32 s = 0.5F;

        auto* vertex_data = new jf::Mesh;
        auto  positions = std::vector<jf::v3>{
            jf::v3::create(-s, -s, 0.0F),
            jf::v3::create(+s, -s, 0.0F),
            jf::v3::create(0.0F, +s, 0.0F)
        };
        auto colors = {
            jf::RGBAColor::solid_red(),
            jf::RGBAColor::solid_green(),
            jf::RGBAColor::solid_blue(),
        };
        vertex_data->m_attributes[jf::Mesh::POSITION.m_id] = jf::Mesh::AttributeData{
            .m_attribute = jf::Mesh::POSITION, .m_data = jf::to_list(positions)
        };
        vertex_data->m_attributes[jf::Mesh::COLOR.m_id] = jf::Mesh::AttributeData{
            .m_attribute = jf::Mesh::COLOR, .m_data = jf::to_list(colors)
        };

        jf::GPUMeshData* mesh = app.m_render_system.register_mesh(*vertex_data);

        m_obj.m_mesh = mesh;
        m_obj.m_vertex_data = vertex_data;
        m_obj.m_material = material;
        m_obj.m_transform = {};
    }

    auto on_update(jf::Application& app) -> void { (void)app; }

    auto on_draw(jf::Application& app) -> void { app.m_render_system.submit(m_obj); }

public:
    jf::Object m_obj;
};

auto main() -> int {
    jf::Instance          jade_frame;
    jf::Application::Desc app_desc;
    app_desc.title = "Hello Triangle";
    app_desc.size = jf::v2u32::create(800, 800);
    app_desc.api = jf::GRAPHICS_API::VULKAN;

    jf::Application*       app = jade_frame.request_application(app_desc);
    Example_Hello_Triangle example_app;
    app->m_on_init_fn = [&]() -> void { example_app.on_init(*app); };
    app->m_on_update_fn = [&]() -> void { example_app.on_update(*app); };
    app->m_on_draw_fn = [&]() -> void { example_app.on_draw(*app); };
    app->start();
    return 0;
}