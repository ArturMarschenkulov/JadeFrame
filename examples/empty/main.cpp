#include <JadeFrame.h>
#include <JadeFrame/math/math.h>
#include <JadeFrame/graphics/mesh.h>

namespace jf = JadeFrame;

struct Example_Empty : public jf::BaseApp {
    explicit Example_Empty(const Desc& desc);
    ~Example_Empty() override = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
};

inline Example_Empty::Example_Empty(const Desc& desc)
    : BaseApp(desc) {}

auto Example_Empty::on_init() -> void {}

auto Example_Empty::on_update() -> void {}

auto Example_Empty::on_draw() -> void {}

using TestApp = Example_Empty;

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
