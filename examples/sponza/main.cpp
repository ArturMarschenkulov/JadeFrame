#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/mesh.h"
#include "JadeFrame/graphics/opengl/opengl_renderer.h"
#include "JadeFrame/graphics/shader_loader.h"
#include "JadeFrame/utils/logger.h"

#include <array>
#include <string_view>
#include <vector>

namespace jf = JadeFrame;

namespace {

// Uploads a position/color cube. The local CPU mesh is released on return.
auto upload_cube(jf::RenderSystem& system, const jf::RGBAColor& color)
    -> jf::GPUMeshData* {
    auto built = jf::MeshBuilder::cube(
        jf::v3::create(-0.5F, -0.5F, -0.5F), jf::v3::create(1, 1, 1)
    );
    if (built.is_err()) {
        jf::Logger::err("Cannot build test cube: {}", built.error().m_message);
        return nullptr;
    }
    auto mesh = std::move(built).value();
    // Match the existing position/color shader. Fixed face colors make the shape
    // visible without introducing a lighting model.
    mesh.m_attributes.erase(jf::Mesh::UV.m_id);
    mesh.m_attributes.erase(jf::Mesh::NORMAL.m_id);
    std::vector<jf::RGBAColor> colors;
    for (const auto shade : std::array{0.55F, 0.85F, 0.65F, 1.0F, 0.45F, 0.95F}) {
        colors.insert(
            colors.end(),
            6,
            jf::RGBAColor::from_rgb(color.r * shade, color.g * shade, color.b * shade)
        );
    }
    auto inserted = mesh.insert_attribute(jf::Mesh::COLOR, jf::to_list(colors));
    if (inserted.is_err()) {
        jf::Logger::err("Cannot color test cube: {}", inserted.error().m_message);
        return nullptr;
    }
    return system.register_mesh(mesh);
}

struct SponzaExample {
    std::vector<jf::Object> m_objects;
    bool                    m_failed = false;

    auto add_instance(
        jf::GPUMeshData*    mesh,
        jf::MaterialHandle* material,
        const jf::v3&       position,
        const jf::v3&       size
    ) -> void {
        jf::Object object;
        object.m_mesh = mesh;
        object.m_material = material;
        object.m_transform.m_translation = jf::mat4x4::translation(position);
        object.m_transform.m_scale = jf::mat4x4::scale(size);
        m_objects.push_back(object);
    }

    // Future imported geometry can replace this function with mesh registration
    // and instance creation. The draw loop below only needs GPU resources.
    auto create_test_scene(jf::RenderSystem& system) -> bool {
        auto code = jf::GLSLCodeLoader::get_by_name("spirv_test_1");
        if (code.is_err()) {
            jf::Logger::err("Cannot load scene shader: {}", code.error().m_message);
            return false;
        }
        auto* shader = system.register_shader({std::move(code).value()});
        if (shader == nullptr) { return false; }
        auto* material = system.register_material(shader, nullptr);
        if (material == nullptr) { return false; }
        auto* warm = upload_cube(system, jf::RGBAColor::from_rgb(1.0F, 0.55F, 0.2F));
        auto* cool = upload_cube(system, jf::RGBAColor::from_rgb(0.2F, 0.55F, 1.0F));
        auto* floor = upload_cube(system, jf::RGBAColor::from_rgb(0.45F, 0.5F, 0.55F));
        if ((warm == nullptr) || (cool == nullptr) || (floor == nullptr)) {
            return false;
        }

        // Near geometry is submitted before far geometry and the floor, making
        // correct occlusion depend on the depth buffer rather than draw order.
        this->add_instance(
            warm, material, jf::v3::create(0, -1, 0), jf::v3::create(2, 2, 2)
        );
        this->add_instance(
            cool, material, jf::v3::create(0, 2, 0.5F), jf::v3::create(2, 2, 3)
        );
        this->add_instance(
            warm, material, jf::v3::create(-3, 1, 0), jf::v3::create(1, 1, 2)
        );
        this->add_instance(
            cool, material, jf::v3::create(3, 1, 0), jf::v3::create(1, 1, 2)
        );
        this->add_instance(
            floor, material, jf::v3::create(0, 1, -1.15F), jf::v3::create(12, 12, 0.3F)
        );
        return true;
    }

    auto on_init(jf::Application& app) -> void {
        auto& renderer = *app.m_render_system.m_renderer;
        if (app.m_render_system.m_api == jf::GRAPHICS_API::OPENGL) {
            auto& opengl = static_cast<jf::OpenGL_Renderer&>(renderer);
            opengl.m_context.m_state.set_depth_test(true);
        }
        // Vulkan enables depth testing in its graphics pipeline.
        renderer.set_clear_color(jf::RGBAColor::from_rgb(0.06F, 0.08F, 0.12F));
        const auto size = app.m_current_window_p->get_size();
        const auto eye = jf::v3::create(7, -11, 7);
        app.m_camera = jf::Camera::perspective(
            eye,
            60.0F,
            static_cast<jf::f32>(size.x) / static_cast<jf::f32>(size.y),
            0.1F,
            100.0F
        );
        auto& orientation = app.m_camera.m_orientation;
        orientation = jf::Camera::Orientation::create((-eye).normalize(), jf::v3::Z());
        orientation.m_right = orientation.m_forward.cross(orientation.m_up).normalize();
        orientation.m_up = orientation.m_right.cross(orientation.m_forward).normalize();
        if (!create_test_scene(app.m_render_system)) {
            m_failed = true;
            app.m_is_running = false;
        }
    }

    auto on_draw(jf::Application& app) const -> void {
        for (const auto& object : m_objects) { app.m_render_system.submit(object); }
    }
};

} // namespace

auto main(int argc, char** argv) -> int {
    const bool smoke_test = argc == 2 && std::string_view(argv[1]) == "--smoke-test";
    if (argc > 1 && !smoke_test) { return 2; }
    jf::Instance          instance;
    jf::Application::Desc desc;
    desc.title = "Sponza - 3D rendering test";
    desc.size = jf::v2u32::create(1280, 720);
    desc.api = jf::GRAPHICS_API::OPENGL;
    auto*         app = instance.request_application(desc);
    SponzaExample example;
    app->m_on_init_fn = [&]() -> void { example.on_init(*app); };
    app->m_on_draw_fn = [&]() -> void { example.on_draw(*app); };
    // Optional bounded run for integration checks on a display or Xvfb.
    app->m_on_frame_rendered_fn = [&]() -> void {
        if (smoke_test && app->m_tick >= 2) {
            auto image = app->m_render_system.m_renderer->take_screenshot(nullptr);
            if (image.is_err() || image.value().data.empty()) {
                jf::Logger::err("Sponza smoke test could not capture a rendered frame");
                example.m_failed = true;
            }
            app->m_is_running = false;
        }
    };
    auto started = app->start();
    if (started.is_err()) {
        jf::Logger::err(
            "Cannot start Sponza example: {}", jf::to_string(started.error())
        );
        return 1;
    }
    return example.m_failed ? 1 : 0;
}
