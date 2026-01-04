#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Moving_Primitive : public jf::BaseApp {

    Example_Moving_Primitive(const Desc& desc);
    ~Example_Moving_Primitive() override = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    std::vector<jf::Object> m_objs;
};

Example_Moving_Primitive::Example_Moving_Primitive(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Moving_Primitive::on_init() -> void {
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

    constexpr jf::i32 block_count = 11;
    jf::u32           win_width = win_size.x;
    auto              block_width =
        static_cast<jf::f32>(win_width) / static_cast<jf::f32>(block_count);
    std::array<std::array<jf::RGBAColor, block_count>, 3> col = {};
    // x
    // x^(2.2)
    // x^(1/2.2)
    const jf::f32 factor = 0.1F;
    for (int i = 0; i < col[0].size(); i++) {
        auto p = factor * static_cast<jf::f32>(i);
        col[0][i] = jf::RGBAColor::from_rgb(p, p, p);
    }
    for (int i = 0; i < col[1].size(); i++) {
        auto p = factor * static_cast<jf::f32>(i);
        col[1][i] = jf::RGBAColor::from_rgb(p, p, p).gamma_decode();
    }
    for (int i = 0; i < col[2].size(); i++) {
        auto p = factor * static_cast<jf::f32>(i);
        col[2][i] = jf::RGBAColor::from_rgb(p, p, p).gamma_encode();
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < block_count; j++) {
            auto* vertex_data = new jf::Mesh();
            auto  positions = std::vector<jf::v3>{
                jf::v3::create(0.0F, 0.0F, 0.0F),
                jf::v3::create(block_width, 0.0F, 0.0F),
                jf::v3::create(block_width, block_width, 0.0F),
                jf::v3::create(0.0F, block_width, 0.0F),
                jf::v3::create(0.0F, 0.0F, 0.0F),
                jf::v3::create(block_width, block_width, 0.0F)
            };
            vertex_data->m_attributes[jf::Mesh::POSITION.m_id] = jf::Mesh::AttributeData{
                .m_attribute = jf::Mesh::POSITION, .m_data = jf::to_list(positions)
            };

            vertex_data->set_color(col[i][j]);
            jf::Transform transform = {};
            transform.m_translation = jf::mat4x4::translation(
                jf::v3::create(
                    block_width * static_cast<jf::f32>(j),
                    block_width * static_cast<jf::f32>(i),
                    0.0F
                )
            );

            jf::GPUMeshData* mesh = m_render_system.register_mesh(*vertex_data);

            jf::Object obj;
            obj.m_transform = transform;
            obj.m_vertex_data = vertex_data;
            obj.m_mesh = mesh;
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