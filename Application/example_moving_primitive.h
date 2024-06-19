#pragma once
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>

namespace JadeFrame {

struct Example_Moving_Primitive : public BaseApp {

    Example_Moving_Primitive(const Desc& desc);
    virtual ~Example_Moving_Primitive() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    // Object              m_obj;
    std::vector<Object> m_objs;
};

Example_Moving_Primitive::Example_Moving_Primitive(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Moving_Primitive::on_init() -> void {
    m_render_system.m_renderer->set_clear_color(RGBAColor::from_hex(0x9E1030FF));
    m_camera.orthographic_mode(
        0.0f, m_windows[0]->get_size().x, 0.0f, m_windows[0]->get_size().y, -1.0f, 1.0f
    );

    constexpr auto block_count = 11;

    u32  win_width = m_windows[0]->get_size().x;
    auto block_width = win_width / (f32)block_count;

    auto format = VertexFormat{
        {"v_position", SHADER_TYPE::V_3_F32},
        {   "v_color", SHADER_TYPE::V_4_F32},
    };

    ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_1");
    shader_handle_desc.vertex_format = format;
    auto*           shader = m_render_system.register_shader(shader_handle_desc);
    MaterialHandle* material = m_render_system.register_material(shader, nullptr);

    std::array<std::array<RGBAColor, block_count>, 3> col = {};
    // x
    // x^(2.2)
    // x^(1/2.2)
    for (int i = 0; i < col[0].size(); i++) {
        auto p = 0.1F * i;
        col[0][i] = RGBAColor(p, p, p);
    }
    for (int i = 0; i < col[1].size(); i++) {
        auto p = 0.1F * i;
        col[1][i] = RGBAColor(p, p, p).gamma_decode();
    }
    for (int i = 0; i < col[2].size(); i++) {
        auto p = 0.1F * i;
        col[2][i] = RGBAColor(p, p, p).gamma_encode();
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < block_count; j++) {
            auto* vertex_data = new VertexData();
            vertex_data->m_positions = std::vector<v3>{
                {       0.0F,        0.0F, 0.0F},
                {block_width,        0.0F, 0.0F},
                {block_width, block_width, 0.0F},
                {       0.0F, block_width, 0.0F},
                {       0.0F,        0.0F, 0.0F},
                {block_width, block_width, 0.0F}
            };
            vertex_data->m_colors = {
                col[i][j], col[i][j], col[i][j], col[i][j], col[i][j], col[i][j]
            };
            Object obj;
            obj.m_transform =
                Matrix4x4::identity() *
                Matrix4x4::translation(v3(block_width * j, block_width * i, 0.0f));
            obj.m_vertex_data = vertex_data;

            obj.m_mesh = m_render_system.register_mesh(*obj.m_vertex_data);
            obj.m_material = material;
            m_objs.push_back(obj);
        }
    }
}

auto Example_Moving_Primitive::on_update() -> void {
    Instance* s = Instance::get_singleton();
}

auto Example_Moving_Primitive::on_draw() -> void {
    for (auto& obj : m_objs) { m_render_system.submit(obj); }
}

using TestApp = Example_Moving_Primitive;

} // namespace JadeFrame