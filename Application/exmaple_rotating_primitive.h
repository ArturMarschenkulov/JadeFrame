#pragma once
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>

namespace JadeFrame {

struct Example_Rotating_Primitive : public BaseApp {

    Example_Rotating_Primitive(const Desc& desc);
    virtual ~Example_Rotating_Primitive() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

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

    ShaderHandle::Desc shader_desc;
    shader_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_1");
    shader_desc.vertex_format = VertexFormat{
        {"v_position", SHADER_TYPE::V_3_F32},
        {   "v_color", SHADER_TYPE::V_4_F32},
    };
    u32 shader = m_render_system.register_shader(shader_desc);

    MaterialHandle material;
    material.m_shader_id = shader;
    material.m_texture_id = 0;

    const f32 s = 0.5F;
    auto      pos_v2 = std::vector<v3>{
        {-s, +s, 0.0F},
        {+s, +s, 0.0F},
        {-s, -s, 0.0F}
    };

    const auto opacity = 0.1F;

    auto* vertex_data = new VertexData();
    vertex_data->m_positions = pos_v2;
    vertex_data->m_colors = {
        RGBAColor::solid_red().set_opacity(opacity),
        RGBAColor::solid_green().set_opacity(opacity),
        RGBAColor::solid_blue(),
    };

    m_obj.m_vertex_data = vertex_data;
    auto mesh_id_0 = m_render_system.register_mesh(*m_obj.m_vertex_data);
    m_obj.m_vertex_data_id = mesh_id_0;

    m_obj.m_material_handle = material;

    auto* vertex_data_2 = new VertexData();
    vertex_data_2->m_positions = std::vector<v3>{
        {-s, +s + 0.1F, 0.0F},
        {+s,        +s, 0.0F},
        {-s,        -s, 0.0F}
    };
    vertex_data_2->m_colors = {
        RGBAColor::solid_yellow(),
        RGBAColor::solid_yellow().set_opacity(opacity),
        RGBAColor::solid_black(),
    };
    m_obj_2.m_material_handle = material;
    m_obj_2.m_vertex_data = vertex_data_2;
    auto mesh_id_2 = m_render_system.register_mesh(*m_obj_2.m_vertex_data);
    m_obj_2.m_vertex_data_id = mesh_id_2;
}

auto Example_Rotating_Primitive::on_update() -> void {

    static auto start_time = std::chrono::high_resolution_clock::now();

    auto current_time = std::chrono::high_resolution_clock::now();
    f32  time = std::chrono::duration<f32, std::chrono::seconds::period>(
                   current_time - start_time
    )
                   .count();

    m_obj.m_transform =
        Matrix4x4::rotation(time * to_radians(90.0F), v3(0.0F, 0.0F, 1.0F));
    m_obj_2.m_transform =
        Matrix4x4::rotation(time * to_radians(45.0F), v3(0.0F, 0.0F, 1.0F));
}

auto Example_Rotating_Primitive::on_draw() -> void {

    m_render_system.m_renderer->submit(m_obj);   // rainbow triangle
    m_render_system.m_renderer->submit(m_obj_2); // yellow triangle
}

using TestApp = Example_Rotating_Primitive;

} // namespace JadeFrame