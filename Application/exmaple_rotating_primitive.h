#pragma once
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>


namespace JadeFrame {

struct Example_Rotating_Primitive : public BaseApp {

    Example_Rotating_Primitive(const DESC& desc);
    virtual ~Example_Rotating_Primitive() = default;

    virtual auto on_init() -> void override;
    virtual auto on_update() -> void override;
    virtual auto on_draw() -> void override;

public:
    Object         m_obj;
    Object         m_obj_2;
    VertexData     m_vertex_data;
    MaterialHandle m_material;
};

Example_Rotating_Primitive::Example_Rotating_Primitive(const DESC& desc)
    : BaseApp(desc) {}

auto Example_Rotating_Primitive::on_init() -> void {
    m_render_system.m_renderer->set_clear_color({0.2f, 0.0f, 0.0f, 1.0f});
    m_camera.othographic_mode(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    // m_camera.perspective_mode({1.0, 1.0, 1.0}, 1.0f, 0.1f, 100.0f);


    const f32   s = 0.5f;
    VertexData* vertex_data = new VertexData();
    auto        pos_v1 = std::vector<v3>{
        {+0, +s, 0.0f},
        {+s, -s, 0.0f},
        {-s, -s, 0.0f}
    };
    auto pos_v2 = std::vector<v3>{
        {-s, +s, 0.0f},
        {+s, +s, 0.0f},
        {-s, -s, 0.0f}
    };



    vertex_data->m_positions = pos_v2;
    vertex_data->m_colors = {
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
    };

    m_obj.m_vertex_data = vertex_data;

    m_obj.m_vertex_format = VertexFormat{
        {"v_position", SHADER_TYPE::FLOAT_3},
        {   "v_color", SHADER_TYPE::FLOAT_4},
    };
    auto mesh_id_0 = m_render_system.register_mesh(m_obj.m_vertex_format, *m_obj.m_vertex_data);
    m_obj.m_vertex_data_id = mesh_id_0;

    ShaderHandle::DESC shader_handle_desc;
    if (m_render_system.m_api == GRAPHICS_API::VULKAN) {
        shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_1");
    } else if (m_render_system.m_api == GRAPHICS_API::OPENGL) {
        shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_0");
    }
    shader_handle_desc.vertex_format = m_obj.m_vertex_format;
    auto shader_id = m_render_system.register_shader(ShaderHandle(shader_handle_desc));
    auto texture_id = 0;

    m_material.m_shader_id = shader_id;
    m_material.m_texture_id = texture_id;
    m_obj.m_material_handle = &m_material;

    VertexData* vertex_data_2 = new VertexData();
    vertex_data_2->m_positions = std::vector<v3>{
        {-s, +s + 0.1f, 0.0f},
        {+s,        +s, 0.0f},
        {-s,        -s, 0.0f}
    };
    vertex_data_2->m_colors = {
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    m_obj_2.m_material_handle = &m_material;
    m_obj_2.m_vertex_data = vertex_data_2;
    m_obj_2.m_vertex_format = VertexFormat{
        {"v_position", SHADER_TYPE::FLOAT_3},
        {   "v_color", SHADER_TYPE::FLOAT_4},
    };
    auto mesh_id_2 = m_render_system.register_mesh(m_obj_2.m_vertex_format, *m_obj_2.m_vertex_data);
    m_obj_2.m_vertex_data_id = mesh_id_2;
}
auto Example_Rotating_Primitive::on_update() -> void {}
auto Example_Rotating_Primitive::on_draw() -> void {
    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    f32  time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time).count();

    m_obj.m_transform = Matrix4x4::rotation_matrix(time * to_radians(90.0f), v3(0.0f, 0.0f, 1.0f));
    m_obj_2.m_transform = Matrix4x4::rotation_matrix(time * to_radians(45.0f), v3(0.0f, 0.0f, 1.0f));


    m_render_system.m_renderer->submit(m_obj_2); // yellow triangle
    m_render_system.m_renderer->submit(m_obj);   // rainbow triangle
}
using TestApp = Example_Rotating_Primitive;

} // namespace JadeFrame