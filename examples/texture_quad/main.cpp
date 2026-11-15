
#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/camera.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/utils/logger.h"
#include "JadeFrame/math/math.h"
#include "JadeFrame/types.h"
#include "JadeFrame/assets/asset_loader.h"
#include <JadeFrame.h>

#include <filesystem>

namespace jf = JadeFrame;

struct State {
    jf::RenderSystem* m_render_system = nullptr;

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

    jf::Mesh         vd_teapot;
    jf::GPUMeshData* mesh_teapot = nullptr;
};

static auto draw_rectangle(
    State&              state,
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

    obj.m_mesh = state.mesh_rectangle;

    obj.m_vertex_data = &state.vd_rectangle;
    obj.m_material = material;

    state.m_render_system->submit(obj);
}

static auto draw_gizmo(State& state, jf::f32 x, jf::f32 y) -> void {
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
        obj_x.m_mesh = state.mesh_gizmo_x;
        obj_x.m_vertex_data = &state.vd_gizmo_x;
        obj_x.m_material = state.material_flat;
        state.m_render_system->submit(obj_x);
    }
    {
        jf::mat4x4 scale_y = jf::mat4x4::scale(jf::v3::create(thickness, length, depth));
        jf::Object obj_y;
        obj_y.m_transform.m_translation = trans;
        obj_y.m_transform.m_scale = scale_y;
        obj_y.m_transform.m_rotation = jf::mat4x4::identity();
        obj_y.m_mesh = state.mesh_gizmo_y;
        obj_y.m_vertex_data = &state.vd_gizmo_y;
        obj_y.m_material = state.material_flat;
        state.m_render_system->submit(obj_y);
    }
    {
        jf::mat4x4 scale_z = jf::mat4x4::scale(jf::v3::create(length, thickness, depth));
        jf::mat4x4 rot_z = jf::mat4x4::rotation_y_rh(jf::to_radians(-90.0F));
        jf::Object obj_z;
        obj_z.m_transform.m_translation = trans;
        obj_z.m_transform.m_rotation = rot_z;
        obj_z.m_transform.m_scale = scale_z;
        obj_z.m_mesh = state.mesh_gizmo_z;
        obj_z.m_vertex_data = &state.vd_gizmo_z;
        obj_z.m_material = state.material_flat;
        state.m_render_system->submit(obj_z);
    }
}

struct Example_Texture_Quad {
    static auto on_init(jf::Application& app, State& state) -> void {
        state.m_render_system = &app.m_render_system;
        using namespace jf::literals;
        app.m_render_system.m_renderer->set_clear_color(
            jf::RGBAColor::from_rgba_u32(50_u8, 230_u8, 250_u8, 253_u8)
        );
        // m_camera.orthographic_mode(0, m_windows[0]->get_size().x,,
        // m_windows[0]->get_size().y, 0, -1, 1);
        // m_camera = jf::Camera::orthographic(-1.0F, 1.0F, 1.0F, -1.0F, -1.0F, 1.0F);

        jf::v2u32 win_size = app.m_windows[0]->get_size();
        // m_camera = jf::Camera::orthographic(0, win_size.x, win_size.y, 0, -10000,
        // 10000);
        app.m_camera = jf::Camera::perspective(
            jf::v3::zero(), 1.0F, win_size.x / win_size.y, 0.1F, 10000.0F
        );
        app.m_camera.m_position = jf::v3::create(55.0F, -20.0F, 70.0F);
        app.m_camera.m_orientation.set_pitch_yaw(45.0F, 150.0F);

        jf::ShaderHandle::Desc shader_handle_desc;
        auto code_with_texture_0 = jf::GLSLCodeLoader::get_by_name("with_texture_0");
        if (code_with_texture_0.is_err()) {
            jf::Logger::err("Failed to load shader: {}", code_with_texture_0.error().m_message);
            return;
        }
        shader_handle_desc.shading_code = std::move(code_with_texture_0).value();
        jf::ShaderHandle* shader_tex =
            app.m_render_system.register_shader(shader_handle_desc);

        auto code_spirv_test_1 = jf::GLSLCodeLoader::get_by_name("spirv_test_1");
        if (code_spirv_test_1.is_err()) {
            jf::Logger::err("Failed to load shader: {}", code_spirv_test_1.error().m_message);
            return;
        }
        shader_handle_desc.shading_code = std::move(code_spirv_test_1).value();
        jf::ShaderHandle* shader_flat =
            app.m_render_system.register_shader(shader_handle_desc);

        namespace fs = std::filesystem;

        fs::path path_wall = fs::path("resource") / "wall.jpg";
        fs::path path_cont = fs::path("resource") / "container.jpg";
        fs::path path_face = fs::path("resource") / "awesomeface.png";

        jf::Image img_wall = jf::Image::load_from_path(path_wall.string());
        jf::Image img_cont = jf::Image::load_from_path(path_cont.string());
        jf::Image img_face = jf::Image::load_from_path(path_face.string());

        jf::TextureHandle* texture_wall = app.m_render_system.register_texture(img_wall);
        jf::TextureHandle* texture_cont = app.m_render_system.register_texture(img_cont);
        jf::TextureHandle* texture_face = app.m_render_system.register_texture(img_face);

        auto default_material_info = jf::MaterialInfo::default_0();

        jf::MaterialHandle* material_wall =
            app.m_render_system.register_material(shader_tex, texture_wall);
        jf::MaterialHandle* material_cont =
            app.m_render_system.register_material(shader_tex, texture_cont);
        jf::MaterialHandle* material_face =
            app.m_render_system.register_material(shader_tex, texture_face);

        jf::MaterialHandle* material_flat =
            app.m_render_system.register_material(shader_flat, nullptr);

        material_wall->m_info = default_material_info;
        material_cont->m_info = default_material_info;
        material_face->m_info = default_material_info;
        material_flat->m_info = default_material_info;

        jf::MeshBuilder::Desc vdf_desc;
        vdf_desc.has_normals = false;
        auto rectangle_result = jf::MeshBuilder::rectangle(
            jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc
        );
        if (rectangle_result.is_err()) {
            jf::Logger::err(
                "Failed to build textured rectangle: {}",
                rectangle_result.error().m_message
            );
            return;
        }
        jf::Mesh vd_rectangle = std::move(rectangle_result).value();
        (void)vd_rectangle.set_color(jf::RGBAColor::solid_blue());
        jf::GPUMeshData* mesh_rectangle = app.m_render_system.register_mesh(vd_rectangle);

        jf::MeshBuilder::Desc vdf_desc_;
        vdf_desc_.has_normals = false;
        vdf_desc_.has_texture_coordinates = false;

        auto gizmo_x_result = jf::MeshBuilder::rectangle(
            jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc_
        );
        if (gizmo_x_result.is_err()) {
            jf::Logger::err("Failed to build X gizmo: {}", gizmo_x_result.error().m_message);
            return;
        }
        jf::Mesh vd_gizmo_x = std::move(gizmo_x_result).value();
        (void)vd_gizmo_x.set_color(jf::RGBAColor::solid_red());

        auto gizmo_y_result = jf::MeshBuilder::rectangle(
            jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc_
        );
        if (gizmo_y_result.is_err()) {
            jf::Logger::err("Failed to build Y gizmo: {}", gizmo_y_result.error().m_message);
            return;
        }
        jf::Mesh vd_gizmo_y = std::move(gizmo_y_result).value();
        (void)vd_gizmo_y.set_color(jf::RGBAColor::solid_green());
        auto gizmo_z_result = jf::MeshBuilder::rectangle(
            jf::v3::zero(), jf::v3::create(1.0F, 1.0F, 0.0F), vdf_desc_
        );
        if (gizmo_z_result.is_err()) {
            jf::Logger::err("Failed to build Z gizmo: {}", gizmo_z_result.error().m_message);
            return;
        }
        jf::Mesh vd_gizmo_z = std::move(gizmo_z_result).value();
        (void)vd_gizmo_z.set_color(jf::RGBAColor::solid_blue());

        jf::GPUMeshData* mesh_gizmo_x = app.m_render_system.register_mesh(vd_gizmo_x);
        jf::GPUMeshData* mesh_gizmo_y = app.m_render_system.register_mesh(vd_gizmo_y);
        jf::GPUMeshData* mesh_gizmo_z = app.m_render_system.register_mesh(vd_gizmo_z);

        fs::path path_teapot = fs::path("resource") / "default_blender_cube_1.obj";
        auto teapot_result = jf::AssetLoader::load_obj(path_teapot.string());
        if (teapot_result.is_err()) {
            jf::Logger::err(
                "Failed to load mesh ({}): {}",
                jf::to_string(teapot_result.error().m_code),
                teapot_result.error().m_message
            );
            return;
        }
        jf::Mesh teapot = std::move(teapot_result).value();
        (void)teapot.set_color(jf::RGBAColor::solid_yellow());
        jf::GPUMeshData* mesh_teapot = app.m_render_system.register_mesh(teapot);

        state.material_wall = material_wall;
        state.material_cont = material_cont;
        state.material_face = material_face;
        state.material_flat = material_flat;

        state.vd_rectangle = vd_rectangle;
        state.vd_gizmo_x = vd_gizmo_x;
        state.vd_gizmo_y = vd_gizmo_y;
        state.vd_gizmo_z = vd_gizmo_z;
        state.mesh_rectangle = mesh_rectangle;
        state.mesh_gizmo_x = mesh_gizmo_x;
        state.mesh_gizmo_y = mesh_gizmo_y;
        state.mesh_gizmo_z = mesh_gizmo_z;
        state.mesh_teapot = mesh_teapot;
        state.vd_teapot = teapot;
    }

    static auto on_update(jf::Application& app, State& state) -> void {
        (void)app;
        (void)state;
    }

    static auto on_draw(jf::Application& app, State& state) -> void {
        draw_rectangle(state, 0, 0, 100, 100, state.material_face);
        draw_rectangle(state, 200, 200, 100, 100, state.material_wall);
        draw_rectangle(state, 100, 100, 100, 100, state.material_cont);

        draw_gizmo(state, 0, 0);

        jf::Object obj_teapot;
        obj_teapot.m_vertex_data = &state.vd_teapot;
        obj_teapot.m_mesh = state.mesh_teapot;
        obj_teapot.m_material = state.material_flat;
        obj_teapot.m_transform.m_translation =
            jf::mat4x4::translation(jf::v3::create(10.0F, 10.0F, 10.0F));

        app.m_render_system.submit(obj_teapot);
    }
};

auto main() -> int {
    jf::Instance          jade_frame;
    jf::Application::Desc app_desc;
    app_desc.title = "Texture Quad Example";
    app_desc.size = jf::v2u32::create(800, 800);
    app_desc.api = jf::GRAPHICS_API::VULKAN;

    jf::Application* app = jade_frame.request_application(app_desc);
    State            state;
    app->m_on_init_fn = [&]() -> void { Example_Texture_Quad::on_init(*app, state); };
    app->m_on_update_fn = [&]() -> void { Example_Texture_Quad::on_update(*app, state); };
    app->m_on_draw_fn = [&]() -> void { Example_Texture_Quad::on_draw(*app, state); };
    app->start();
    return 0;
}
