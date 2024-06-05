#pragma once
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>

namespace JadeFrame {

struct Example_Hello_Triangle : public BaseApp {

    Example_Hello_Triangle(const Desc& desc);
    virtual ~Example_Hello_Triangle() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    Object m_obj;
};

inline Example_Hello_Triangle::Example_Hello_Triangle(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Hello_Triangle::on_init() -> void {
    m_render_system.m_renderer->set_clear_color(RGBAColor::solid_black());
    m_camera.orthographic_mode(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);
    // m_camera.perspective_mode({1.0, 1.0, 1.0}, 1.0f, 0.1f, 100.0f);

    const f32 s = 0.5F;

    auto* vertex_data = new VertexData;
    vertex_data->m_positions = std::vector<v3>{
        {  -s, -s, 0.0F},
        {  +s, -s, 0.0F},
        {0.0F, +s, 0.0F}
    };
    vertex_data->m_colors = {
        RGBAColor::solid_red(),
        RGBAColor::solid_green(),
        RGBAColor::solid_blue(),
    };

    m_obj.m_vertex_data = vertex_data;

    m_obj.m_vertex_data_id = m_render_system.register_mesh(*m_obj.m_vertex_data);

    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_1");
    shader_handle_desc.vertex_format = VertexFormat{
        {"v_position", SHADER_TYPE::V_3_F32},
        {   "v_color", SHADER_TYPE::V_4_F32},
    };
    m_obj.m_material_handle.m_shader_id =
        m_render_system.register_shader(shader_handle_desc);
    m_obj.m_material_handle.m_texture_id = 0;

    m_obj.m_transform = Matrix4x4::identity();
}

auto Example_Hello_Triangle::on_update() -> void {}

auto Example_Hello_Triangle::on_draw() -> void {
    m_render_system.m_renderer->submit(m_obj);
}

using TestApp = Example_Hello_Triangle;

} // namespace JadeFrame