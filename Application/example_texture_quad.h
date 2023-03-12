#pragma once
#include <JadeFrame.h>

namespace JadeFrame {

struct Example_Texture_Quad : public BaseApp {
    Example_Texture_Quad(const Desc& desc);
    virtual ~Example_Texture_Quad() = default;

    virtual auto on_init() -> void override;
    virtual auto on_update() -> void override;
    virtual auto on_draw() -> void override;

public:
    Object         m_obj;
    MaterialHandle m_material;
};

Example_Texture_Quad::Example_Texture_Quad(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Texture_Quad::on_init() -> void {
    m_render_system.m_renderer->set_clear_color({50_u8, 230_u8, 250_u8, 253_u8});
    // m_camera.othographic_mode(0, m_windows[0]->get_size().x, m_windows[0]->get_size().y, 0, -1, 1);
    m_camera.othographic_mode(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    const f32   s = 0.5f;
    VertexData* vertex_data = new VertexData();

    vertex_data->m_positions = std::vector<v3>{
        {-s, +s, 0.0f},
        {-s, -s, 0.0f},
        {+s, +s, 0.0f}
    };
    vertex_data->m_texture_coordinates = {
        {-1, +1},
        {+1, +1},
        {-1, -1},
    };
    vertex_data->m_colors = {
        RGBAColor::solid_black().set_opacity(0.1f), //
        RGBAColor::solid_black().set_opacity(0.1f), //
        RGBAColor::solid_black().set_opacity(0.1f)  //
    };
    vertex_data->m_normals = {};
    m_obj.m_vertex_data = vertex_data;

    m_obj.m_vertex_format = VertexFormat{
        {     "v_position", SHADER_TYPE::F32_3},
        {        "v_color", SHADER_TYPE::F32_4},
        {"v_texture_coord", SHADER_TYPE::F32_2},
    };
    auto mesh_id = m_render_system.register_mesh(m_obj.m_vertex_format, *m_obj.m_vertex_data);
    m_obj.m_vertex_data_id = mesh_id;


    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("with_texture_0");
    shader_handle_desc.vertex_format = m_obj.m_vertex_format;
    auto shader_id = m_render_system.register_shader(std::move(shader_handle_desc));

    auto texture_path = "C:\\dev\\proj\\JadeFrame\\JadeFrame\\resource\\wall.jpg";
    auto img = Image::load(texture_path);
    auto texture_id = m_render_system.register_texture(std::move(img));



    m_material.m_shader_id = shader_id;
    m_material.m_texture_id = texture_id;
    m_obj.m_material_handle = m_material;

    m_obj.m_transform = Matrix4x4::identity();
}
auto Example_Texture_Quad::on_update() -> void {}
auto Example_Texture_Quad::on_draw() -> void { m_render_system.m_renderer->submit(m_obj); }
using TestApp = Example_Texture_Quad;
} // namespace JadeFrame