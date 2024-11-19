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
    // m_camera.perspective_mode({1.0, 1.0, 1.0}, 1.0f, 0.1f, 100.0f);
    m_camera = Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);

    ShaderHandle::Desc shader_desc;
    shader_desc.shading_code = GLSLCodeLoader::get_by_name("spirv_test_1");

    ShaderHandle*   shader = m_render_system.register_shader(shader_desc);
    MaterialHandle* material = m_render_system.register_material(shader, nullptr);

    const f32  s = 0.5F;
    const auto opacity = 0.1F;

    auto* vertex_data = new VertexData();
    vertex_data->m_positions = std::vector<v3>{
        v3::create(-s, +s, -0.03F), v3::create(+s, +s, -0.03F), v3::create(-s, -s, -0.03F)
    };

    vertex_data->m_colors = {
        RGBAColor::solid_red().set_opacity(opacity),
        RGBAColor::solid_green().set_opacity(opacity),
        RGBAColor::solid_blue(),
    };

    m_obj.m_vertex_data = vertex_data;
    GPUMeshData* mesh = m_render_system.register_mesh(*m_obj.m_vertex_data);
    m_obj.m_mesh = mesh;

    m_obj.m_material = material;

    auto* vertex_data_2 = new VertexData();
    vertex_data_2->m_positions = std::vector<v3>{
        v3::create(-s, +s + 0.1F, -0.02F),
        v3::create(+s, +s, -0.02F),
        v3::create(-s, -s, -0.02F)
    };
    vertex_data_2->m_colors = {
        RGBAColor::solid_yellow(),
        RGBAColor::solid_yellow().set_opacity(opacity),
        RGBAColor::solid_black(),
    };
    m_obj_2.m_material = material;
    m_obj_2.m_vertex_data = vertex_data_2;
    GPUMeshData* mesh_2 = m_render_system.register_mesh(*m_obj_2.m_vertex_data);
    m_obj_2.m_mesh = mesh_2;

    JadeFrame::Logger::info("Example_Rotating_Primitive initialized");
}

auto Example_Rotating_Primitive::on_update() -> void {

    using namespace std::chrono;
    static auto start_time = high_resolution_clock::now();

    auto current_time = high_resolution_clock::now();
    f32  time = duration<f32, seconds::period>(current_time - start_time).count();

    m_obj.m_transform = mat4x4::rotation(time * to_radians(90.0F), v3::Z());
    m_obj_2.m_transform = mat4x4::rotation(time * to_radians(45.0F), v3::Z());
}

auto Example_Rotating_Primitive::on_draw() -> void {
    m_render_system.submit(m_obj);   // rainbow triangle
    m_render_system.submit(m_obj_2); // yellow triangle
}

using TestApp = Example_Rotating_Primitive;

} // namespace JadeFrame

int main() {
    JadeFrame::Instance jade_frame;

    using GApp = JadeFrame::TestApp;
    GApp::Desc win_desc;
    win_desc.title = "Test";
    win_desc.size.x = 800; // = 1280;
    win_desc.size.y = 800; // = 720;
    win_desc.api = JadeFrame::GRAPHICS_API::OPENGL;

    GApp* app = jade_frame.request_app<GApp>(win_desc);
    jade_frame.run();
    return 0;
}