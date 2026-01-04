#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Moving_Quad : public jf::BaseApp {
    explicit Example_Moving_Quad(const Desc& desc);
    ~Example_Moving_Quad() override = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
};

inline Example_Moving_Quad::Example_Moving_Quad(const Desc& desc)
    : BaseApp(desc) {}

inline auto Example_Moving_Quad::on_init() -> void {
    m_render_system.m_renderer->set_clear_color(jf::RGBAColor::from_hex(0x9E1030FF));
    auto win_size = m_windows[0]->get_size();
    m_camera = jf::Camera::orthographic(
        0.0F,
        static_cast<jf::f32>(win_size.x),
        0.0F,
        static_cast<jf::f32>(win_size.y),
        -1.0F,
        1.0F
    );

    jf::ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");

    jf::ShaderHandle*   shader = m_render_system.register_shader(shader_handle_desc);
    jf::MaterialHandle* material = m_render_system.register_material(shader, nullptr);

    jf::MeshBuilder::Desc vdf_desc;
    vdf_desc.has_normals = true;
    auto mesh_builder = jf::Mesh::builder();
    auto rectangle = mesh_builder.rectangle(
        jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc
    );
    rectangle.set_color(jf::RGBAColor::solid_red());

    jf::GPUMeshData* mesh_data = m_render_system.register_mesh(rectangle);
}

inline auto Example_Moving_Quad::on_update() -> void {
    auto input_state = m_windows[0]->m_input_state;
}

inline auto Example_Moving_Quad::on_draw() -> void {}

using TestApp = Example_Moving_Quad;

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