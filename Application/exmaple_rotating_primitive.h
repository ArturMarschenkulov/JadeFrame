#pragma once
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>


namespace JadeFrame {

struct Example_Rotating_Primitive : public BaseApp {

    Example_Rotating_Primitive(const Desc& desc);
    virtual ~Example_Rotating_Primitive() = default;

    virtual auto on_init() -> void override;
    virtual auto on_update() -> void override;
    virtual auto on_draw() -> void override;

public:
    Object m_obj;
    Object m_obj_2;
};

Example_Rotating_Primitive::Example_Rotating_Primitive(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Rotating_Primitive::on_init() -> void {
    m_render_system.m_renderer->set_clear_color({0.2f, 0.0f, 0.0f, 1.0f});
    m_camera.orthographic_mode(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    // m_camera.perspective_mode({1.0, 1.0, 1.0}, 1.0f, 0.1f, 100.0f);


    const f32 s = 0.5f;

    auto pos_v2 = std::vector<v3>{
        {-s, +s, 0.0f},
        {+s, +s, 0.0f},
        {-s, -s, 0.0f}
    };


    VertexData* vertex_data = new VertexData();
    vertex_data->m_positions = pos_v2;
    vertex_data->m_colors = {
        RGBAColor::solid_red(),
        RGBAColor::solid_green(),
        RGBAColor::solid_blue(),
    };

    m_obj.m_vertex_data = vertex_data;

    m_obj.m_vertex_format = VertexFormat{
        {"v_position", SHADER_TYPE::V_3_F32},
        {   "v_color", SHADER_TYPE::V_4_F32},
    };
    auto mesh_id_0 = m_render_system.register_mesh(m_obj.m_vertex_format, *m_obj.m_vertex_data);
    m_obj.m_vertex_data_id = mesh_id_0;

    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_1");
    shader_handle_desc.vertex_format = m_obj.m_vertex_format;

    MaterialHandle material;
    material.m_shader_id = m_render_system.register_shader(std::move(shader_handle_desc));
    material.m_texture_id = 0;
    m_obj.m_material_handle = material;

    VertexData* vertex_data_2 = new VertexData();
    vertex_data_2->m_positions = std::vector<v3>{
        {-s, +s + 0.1f, 0.0f},
        {+s,        +s, 0.0f},
        {-s,        -s, 0.0f}
    };
    vertex_data_2->m_colors = {
        RGBAColor::solid_yellow(),
        RGBAColor::solid_yellow(),
        RGBAColor::solid_black(),
    };
    m_obj_2.m_material_handle = material;
    m_obj_2.m_vertex_data = vertex_data_2;
    m_obj_2.m_vertex_format = VertexFormat{
        {"v_position", SHADER_TYPE::V_3_F32},
        {   "v_color", SHADER_TYPE::V_4_F32},
    };
    auto mesh_id_2 = m_render_system.register_mesh(m_obj_2.m_vertex_format, *m_obj_2.m_vertex_data);
    m_obj_2.m_vertex_data_id = mesh_id_2;
}
auto Example_Rotating_Primitive::on_update() -> void {


    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    f32  time = std::chrono::duration<f32, std::chrono::seconds::period>(current_time - start_time).count();

    m_obj.m_transform = Matrix4x4::rotation(time * to_radians(90.0f), v3(0.0f, 0.0f, 1.0f));
    m_obj_2.m_transform = Matrix4x4::rotation(time * to_radians(45.0f), v3(0.0f, 0.0f, 1.0f));
}
auto Example_Rotating_Primitive::on_draw() -> void {



    m_render_system.m_renderer->submit(m_obj_2); // yellow triangle
    m_render_system.m_renderer->submit(m_obj);   // rainbow triangle
}
using TestApp = Example_Rotating_Primitive;

} // namespace JadeFrame