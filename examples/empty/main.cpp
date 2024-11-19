#include <JadeFrame.h>

namespace JadeFrame {
struct Example_Empty : public BaseApp {
    explicit Example_Empty(const Desc& desc);
    ~Example_Empty() override = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    // std::deque<Mesh> m_meshes;
    // std::vector<Object> m_objs;
};

inline Example_Empty::Example_Empty(const Desc& desc)
    : BaseApp(desc) {
    // m_current_app_p = reinterpret_cast<Example_Empty*>(this);
}

inline auto Example_Empty::on_init() -> void {}

inline auto Example_Empty::on_update() -> void {}

inline auto Example_Empty::on_draw() -> void {}

using TestApp = Example_Empty;
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