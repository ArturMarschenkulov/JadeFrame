#include "JadeFrame/utils/logger.h"
#include "JadeFrame/base_app.h"
#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Empty {
    static auto on_init(jf::Application& app) -> void { (void)app; }

    static auto on_update(jf::Application& app) -> void { (void)app; }

    static auto on_draw(jf::Application& app) -> void { (void)app; }
};

auto main() -> int {
    jf::Instance          jade_frame;
    jf::Application::Desc app_desc;
    app_desc.title = "Empty Example";
    app_desc.size = jf::v2u32::create(800, 800);
    app_desc.api = jf::GRAPHICS_API::OPENGL;

    jf::Application* app = jade_frame.request_application(app_desc);
    app->m_on_init_fn = [&]() -> void { Example_Empty::on_init(*app); };
    app->m_on_update_fn = [&]() -> void { Example_Empty::on_update(*app); };
    app->m_on_draw_fn = [&]() -> void { Example_Empty::on_draw(*app); };
    auto started = app->start();
    if (started.is_err()) {
        // Initialization errors are not logged by the framework, this is the one
        // place that reports them.
        jf::Logger::err(
            "Failed to start the application: {}", jf::to_string(started.error())
        );
        return 1;
    }
    return 0;
}
