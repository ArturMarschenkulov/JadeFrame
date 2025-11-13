
#include "JadeFrame/graphics/camera.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/math/math.h"
#include "JadeFrame/types.h"
#include "JadeFrame/utils/asset_loader.h"
#include <JadeFrame.h>

#include <filesystem>

namespace jf = JadeFrame;

struct State {
    jf::RenderSystem* m_render_system = nullptr;
    jf::BaseApp*      m_app = nullptr;

    // jf::ShaderHandle* shader_texture = nullptr;

    jf::MaterialHandle* material_wall = nullptr;
    jf::MaterialHandle* material_cont = nullptr;
    jf::MaterialHandle* material_face = nullptr;
    jf::MaterialHandle* material_flat = nullptr;

    jf::Mesh vd_rectangle;
    jf::Mesh vd_gizmo_x;
    jf::Mesh vd_gizmo_y;
    jf::Mesh vd_gizmo_z;

    jf::GPUMeshData* mesh_rectangle = nullptr;
    jf::GPUMeshData* mesh_gizmo_x = nullptr;
    jf::GPUMeshData* mesh_gizmo_y = nullptr;
    jf::GPUMeshData* mesh_gizmo_z = nullptr;
    jf::GPUMeshData* mesh_teapot = nullptr;
};

static State g_state;

static auto draw_rectangle(
    jf::f32             x,
    jf::f32             y,
    jf::f32             width,
    jf::f32             height,
    jf::MaterialHandle* material
) -> void {
    jf::mat4x4 scale = jf::mat4x4::scale(jf::v3::create(width, height, 1.0F));
    jf::mat4x4 trans = jf::mat4x4::translation(jf::v3::create(x, y, 0.0F));

    jf::Object obj;
    obj.m_transform.m_translation = trans;
    obj.m_transform.m_scale = scale;
    obj.m_transform.m_rotation = jf::mat4x4::identity();

    jf::Mesh& m = g_state.vd_rectangle;
    obj.m_mesh = g_state.mesh_rectangle;

    obj.m_vertex_data = &g_state.vd_rectangle;
    obj.m_material = material;

    g_state.m_render_system->submit(obj);
}

static auto draw_gizmo(jf::f32 x, jf::f32 y) -> void {
    jf::mat4x4 trans = jf::mat4x4::translation(jf::v3::create(x, y, 0.0F));

    constexpr jf::f32 length = 1000.0F;
    constexpr jf::f32 thickness = 10.0F;
    constexpr jf::f32 depth = 1.0F;
    {
        jf::mat4x4 scale_x = jf::mat4x4::scale(jf::v3::create(length, thickness, depth));
        jf::Object obj_x;

        obj_x.m_transform.m_translation = trans;
        obj_x.m_transform.m_scale = scale_x;
        obj_x.m_transform.m_rotation = jf::mat4x4::identity();
        obj_x.m_mesh = g_state.mesh_gizmo_x;
        obj_x.m_vertex_data = &g_state.vd_gizmo_x;
        obj_x.m_material = g_state.material_flat;
        g_state.m_render_system->submit(obj_x);
    }
    {
        jf::mat4x4 scale_y = jf::mat4x4::scale(jf::v3::create(thickness, length, depth));
        jf::Object obj_y;
        obj_y.m_transform.m_translation = trans;
        obj_y.m_transform.m_scale = scale_y;
        obj_y.m_transform.m_rotation = jf::mat4x4::identity();
        obj_y.m_mesh = g_state.mesh_gizmo_y;
        obj_y.m_vertex_data = &g_state.vd_gizmo_y;
        obj_y.m_material = g_state.material_flat;
        g_state.m_render_system->submit(obj_y);
    }
    {
        jf::mat4x4 scale_z = jf::mat4x4::scale(jf::v3::create(length, thickness, depth));
        jf::mat4x4 rot_z = jf::mat4x4::rotation_y_rh(jf::to_radians(-90.0F));
        jf::Object obj_z;
        obj_z.m_transform.m_translation = trans;
        obj_z.m_transform.m_rotation = rot_z;
        obj_z.m_transform.m_scale = scale_z;
        obj_z.m_mesh = g_state.mesh_gizmo_z;
        obj_z.m_vertex_data = &g_state.vd_gizmo_z;
        obj_z.m_material = g_state.material_flat;
        g_state.m_render_system->submit(obj_z);
    }
}

struct Example_Texture_Quad : public jf::BaseApp {
    Example_Texture_Quad(const Desc& desc);
    virtual ~Example_Texture_Quad() = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
};

Example_Texture_Quad::Example_Texture_Quad(const Desc& desc)
    : BaseApp(desc) {
    g_state.m_render_system = &m_render_system;
    g_state.m_app = this;
}

auto Example_Texture_Quad::on_init() -> void {
    using namespace jf::literals;
    m_render_system.m_renderer->set_clear_color(
        jf::RGBAColor::from_rgba_u32(50_u8, 230_u8, 250_u8, 253_u8)
    );
    // m_camera.orthographic_mode(0, m_windows[0]->get_size().x,,
    // m_windows[0]->get_size().y, 0, -1, 1);
    // m_camera = jf::Camera::orthographic(-1.0F, 1.0F, 1.0F, -1.0F, -1.0F, 1.0F);

    jf::v2u32 win_size = m_windows[0]->get_size();
    // m_camera = jf::Camera::orthographic(0, win_size.x, win_size.y, 0, -10000, 10000);
    m_camera = jf::Camera::perspective(
        jf::v3::zero(), 1.0F, win_size.x / win_size.y, 0.1F, 10000.0F
    );
    m_camera.m_position = jf::v3::create(55.0F, -20.0F, 70.0F);
    m_camera.m_orientation.set_pitch_yaw(45, 150);

    jf::ShaderHandle::Desc shader_handle_desc;
    shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("with_texture_0");
    jf::ShaderHandle* shader_tex = m_render_system.register_shader(shader_handle_desc);

    shader_handle_desc.shading_code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");
    jf::ShaderHandle* shader_flat = m_render_system.register_shader(shader_handle_desc);

    namespace fs = std::filesystem;

    fs::path path_wall = fs::path("resource") / "wall.jpg";
    fs::path path_cont = fs::path("resource") / "container.jpg";
    fs::path path_face = fs::path("resource") / "awesomeface.png";
    fs::path path_teapot = fs::path("resource") / "default_blender_cube.obj";

    jf::Image img_wall = jf::Image::load_from_path(path_wall.string());
    jf::Image img_cont = jf::Image::load_from_path(path_cont.string());
    jf::Image img_face = jf::Image::load_from_path(path_face.string());

    jf::TextureHandle* texture_wall = m_render_system.register_texture(img_wall);
    jf::TextureHandle* texture_cont = m_render_system.register_texture(img_cont);
    jf::TextureHandle* texture_face = m_render_system.register_texture(img_face);

    jf::MaterialHandle* material_wall =
        m_render_system.register_material(shader_tex, texture_wall);
    jf::MaterialHandle* material_cont =
        m_render_system.register_material(shader_tex, texture_cont);
    jf::MaterialHandle* material_face =
        m_render_system.register_material(shader_tex, texture_face);

    jf::MaterialHandle* material_flat =
        m_render_system.register_material(shader_flat, nullptr);

    jf::MeshBuilder::Desc vdf_desc;
    vdf_desc.has_normals = true;
    jf::Mesh vd_rectangle = jf::MeshBuilder::rectangle(
        jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc
    );
    vd_rectangle.set_color(jf::RGBAColor::solid_blue());
    jf::GPUMeshData* mesh_rectangle = m_render_system.register_mesh(vd_rectangle);

    jf::MeshBuilder::Desc vdf_desc_;
    vdf_desc_.has_normals = false;
    vdf_desc_.has_texture_coordinates = false;

    jf::Mesh vd_gizmo_x =
        jf::MeshBuilder::rectangle(jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc_);
    vd_gizmo_x.set_color(jf::RGBAColor::solid_red());

    jf::Mesh vd_gizmo_y =
        jf::MeshBuilder::rectangle(jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc_);
    vd_gizmo_y.set_color(jf::RGBAColor::solid_green());
    jf::Mesh vd_gizmo_z =
        jf::MeshBuilder::rectangle(jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc_);
    vd_gizmo_z.set_color(jf::RGBAColor::solid_blue());

    jf::GPUMeshData* mesh_gizmo_x = m_render_system.register_mesh(vd_gizmo_x);
    jf::GPUMeshData* mesh_gizmo_y = m_render_system.register_mesh(vd_gizmo_y);
    jf::GPUMeshData* mesh_gizmo_z = m_render_system.register_mesh(vd_gizmo_z);

    jf::Mesh teapot = jf::AssetLoader::load_obj(path_teapot.string());
    teapot.set_color(jf::RGBAColor::solid_yellow());
    jf::GPUMeshData* mesh_teapot = m_render_system.register_mesh(teapot);

    g_state.material_wall = material_wall;
    g_state.material_cont = material_cont;
    g_state.material_face = material_face;
    g_state.material_flat = material_flat;

    g_state.vd_rectangle = vd_rectangle;
    g_state.vd_gizmo_x = vd_gizmo_x;
    g_state.vd_gizmo_y = vd_gizmo_y;
    g_state.vd_gizmo_z = vd_gizmo_z;

    g_state.mesh_rectangle = mesh_rectangle;
    g_state.mesh_gizmo_x = mesh_gizmo_x;
    g_state.mesh_gizmo_y = mesh_gizmo_y;
    g_state.mesh_gizmo_z = mesh_gizmo_z;
    g_state.mesh_teapot = mesh_teapot;
}

auto Example_Texture_Quad::on_update() -> void {}

auto Example_Texture_Quad::on_draw() -> void {

    draw_rectangle(0, 0, 100, 100, g_state.material_face);
    draw_rectangle(200, 200, 100, 100, g_state.material_wall);
    draw_rectangle(100, 100, 100, 100, g_state.material_cont);

    draw_gizmo(0, 0);

    jf::Object obj_teapot;
    obj_teapot.m_vertex_data = &g_state.vd_rectangle;
    obj_teapot.m_mesh = g_state.mesh_teapot;
    obj_teapot.m_material = g_state.material_flat;
    obj_teapot.m_transform.m_translation =
        jf::mat4x4::translation(jf::v3::create(10.0F, 10.0F, 10.0F));

    m_render_system.submit(obj_teapot);
}

using TestApp = Example_Texture_Quad;

auto main() -> int {
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