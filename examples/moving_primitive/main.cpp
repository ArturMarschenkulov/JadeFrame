#include "JadeFrame/base_app.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Moving_Primitive_2 {
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

        constexpr jf::i32 block_count = 11;
        jf::u32           win_width = win_size.x;
        auto              block_width =
            static_cast<jf::f32>(win_width) / static_cast<jf::f32>(block_count);
        std::array<std::array<jf::RGBAColor, block_count>, 3> col = {};
        // x
        // x^(2.2)
        // x^(1/2.2)
        const jf::f32 factor = 0.1F;
        for (int i = 0; i < col[0].size(); i++) {
            auto p = factor * static_cast<jf::f32>(i);
            col[0][i] = jf::RGBAColor::from_rgb(p, p, p);
        }
        for (int i = 0; i < col[1].size(); i++) {
            auto p = factor * static_cast<jf::f32>(i);
            col[1][i] = jf::RGBAColor::from_rgb(p, p, p).gamma_decode();
        }
        for (int i = 0; i < col[2].size(); i++) {
            auto p = factor * static_cast<jf::f32>(i);
            col[2][i] = jf::RGBAColor::from_rgb(p, p, p).gamma_encode();
        }

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < block_count; j++) {
                auto* vertex_data = new jf::Mesh();
                auto  positions = std::vector<jf::v3>{
                    jf::v3::create(0.0F, 0.0F, 0.0F),
                    jf::v3::create(block_width, 0.0F, 0.0F),
                    jf::v3::create(block_width, block_width, 0.0F),
                    jf::v3::create(0.0F, block_width, 0.0F),
                    jf::v3::create(0.0F, 0.0F, 0.0F),
                    jf::v3::create(block_width, block_width, 0.0F)
                };
                vertex_data->m_attributes[jf::Mesh::POSITION.m_id] =
                    jf::Mesh::AttributeData{
                        .m_attribute = jf::Mesh::POSITION,
                        .m_data = jf::to_list(positions)
                    };

                vertex_data->set_color(col[i][j]);
                jf::Transform transform = {};
                transform.m_translation = jf::mat4x4::translation(
                    jf::v3::create(
                        block_width * static_cast<jf::f32>(j),
                        block_width * static_cast<jf::f32>(i),
                        0.0F
                    )
                );

                jf::GPUMeshData* mesh = app.m_render_system.register_mesh(*vertex_data);

                jf::Object obj;
                obj.m_transform = transform;
                obj.m_vertex_data = vertex_data;
                obj.m_mesh = mesh;
                obj.m_material = material;
                m_objs.push_back(obj);
            }
        }
    }

    auto on_update(jf::Application& app) -> void {
        jf::Instance* s = jf::Instance::get_singleton();
    }

    auto on_draw(jf::Application& app) -> void {
        for (auto& obj : m_objs) { app.m_render_system.submit(obj); }
    }

public:
    std::vector<jf::Object> m_objs;
};

auto main() -> int {
    jf::Instance          jade_frame;
    jf::Application::Desc app_desc;
    app_desc.title = "Moving Primitive Example";
    app_desc.size = jf::v2u32::create(800, 800);
    app_desc.api = jf::GRAPHICS_API::OPENGL;

    jf::Application*           app = jade_frame.request_application(app_desc);
    Example_Moving_Primitive_2 example_app;
    app->m_on_init_fn = [&]() -> void { example_app.on_init(*app); };
    app->m_on_update_fn = [&]() -> void { example_app.on_update(*app); };
    app->m_on_draw_fn = [&]() -> void { example_app.on_draw(*app); };
    app->start();
}