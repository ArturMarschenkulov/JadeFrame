#include <JadeFrame.h>
#include <JadeFrame/math/math.h>

namespace jf = JadeFrame;

struct Example_Moving_Primitive : public jf::BaseApp {

    Example_Moving_Primitive(const Desc& desc);
    virtual ~Example_Moving_Primitive() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    // Object              m_obj;
    std::vector<jf::Object> m_objs;
};

Example_Moving_Primitive::Example_Moving_Primitive(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Moving_Primitive::on_init() -> void {
    m_render_system.m_renderer->set_clear_color(jf::RGBAColor::from_hex(0x9E1030FF));
    m_camera = jf::Camera::orthographic(
        0.0f, m_windows[0]->get_size().x, 0.0f, m_windows[0]->get_size().y, -1.0f, 1.0f
    );

    constexpr auto block_count = 11;

    jf::u32 win_width = m_windows[0]->get_size().x;
    auto block_width = win_width / (jf::f32)block_count;

    jf::ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");

    auto*           shader = m_render_system.register_shader(shader_handle_desc);
    jf::MaterialHandle* material = m_render_system.register_material(shader, nullptr);

    std::array<std::array<jf::RGBAColor, block_count>, 3> col = {};
    // x
    // x^(2.2)
    // x^(1/2.2)
    for (int i = 0; i < col[0].size(); i++) {
        auto p = 0.1F * i;
        col[0][i] = jf::RGBAColor(p, p, p);
    }
    for (int i = 0; i < col[1].size(); i++) {
        auto p = 0.1F * i;
        col[1][i] = jf::RGBAColor(p, p, p).gamma_decode();
    }
    for (int i = 0; i < col[2].size(); i++) {
        auto p = 0.1F * i;
        col[2][i] = jf::RGBAColor(p, p, p).gamma_encode();
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < block_count; j++) {
            auto* vertex_data = new jf::VertexData();
            vertex_data->m_positions = std::vector<jf::v3>{
                jf::v3::create(0.0F, 0.0F, 0.0F),
                jf::v3::create(block_width, 0.0F, 0.0F),
                jf::v3::create(block_width, block_width, 0.0F),
                jf::v3::create(0.0F, block_width, 0.0F),
                jf::v3::create(0.0F, 0.0F, 0.0F),
                jf::v3::create(block_width, block_width, 0.0F)
            };

            vertex_data->set_color(col[i][j]);

            jf::Object obj;
            obj.m_transform = jf::mat4x4::identity() *
                              jf::mat4x4::translation(
                                  jf::v3::create(block_width * j, block_width * i, 0.0f)
                              );
            obj.m_vertex_data = vertex_data;

            obj.m_mesh = m_render_system.register_mesh(*obj.m_vertex_data);
            obj.m_material = material;
            m_objs.push_back(obj);
        }
    }
}

auto Example_Moving_Primitive::on_update() -> void {
    jf::Instance* s = jf::Instance::get_singleton();
}

auto Example_Moving_Primitive::on_draw() -> void {
    for (auto& obj : m_objs) { m_render_system.submit(obj); }
}

using TestApp = Example_Moving_Primitive;

int main() {
    jf::Instance jade_frame;

    using GApp = TestApp;
    GApp::Desc win_desc;
    win_desc.title = "Test";
    win_desc.size.x = 800; // = 1280;
    win_desc.size.y = 800; // = 720;
    win_desc.api = jf::GRAPHICS_API::OPENGL;

    GApp* app = jade_frame.request_app<GApp>(win_desc);
    jade_frame.run();
    return 0;
}