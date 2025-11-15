#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Hello_Triangle : public jf::BaseApp {

    Example_Hello_Triangle(const Desc& desc);
    virtual ~Example_Hello_Triangle() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    jf::Object m_obj;
};

inline Example_Hello_Triangle::Example_Hello_Triangle(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Hello_Triangle::on_init() -> void {
    m_render_system.m_renderer->set_clear_color(jf::RGBAColor::solid_black());
    // m_camera.perspective_mode({1.0, 1.0, 1.0}, 1.0f, 0.1f, 100.0f);
    m_camera = jf::Camera::orthographic(-1.0F, 1.0F, -1.0F, 1.0F, -1.0F, 1.0F);

    jf::ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");
    jf::ShaderHandle*   shader = m_render_system.register_shader(shader_handle_desc);
    jf::MaterialHandle* material = m_render_system.register_material(shader, nullptr);

    const jf::f32 s = 0.5F;

    auto* vertex_data = new jf::Mesh;
    auto  positions = std::vector<jf::v3>{
        jf::v3::create(-s, -s, 0.0F),
        jf::v3::create(+s, -s, 0.0F),
        jf::v3::create(0.0F, +s, 0.0F)
    };
    auto colors = {
        jf::RGBAColor::solid_red(),
        jf::RGBAColor::solid_green(),
        jf::RGBAColor::solid_blue(),
    };
    vertex_data->m_attributes[jf::Mesh::POSITION.m_id] = jf::Mesh::AttributeData{
        .m_attribute = jf::Mesh::POSITION, .m_data = jf::to_list(positions)
    };
    vertex_data->m_attributes[jf::Mesh::COLOR.m_id] = jf::Mesh::AttributeData{
        .m_attribute = jf::Mesh::COLOR, .m_data = jf::to_list(colors)
    };

    jf::GPUMeshData* mesh = m_render_system.register_mesh(*vertex_data);

    m_obj.m_mesh = mesh;
    m_obj.m_vertex_data = vertex_data;
    m_obj.m_material = material;
    m_obj.m_transform = {};
}

auto Example_Hello_Triangle::on_update() -> void {}

auto Example_Hello_Triangle::on_draw() -> void { m_render_system.submit(m_obj); }

using TestApp = Example_Hello_Triangle;

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