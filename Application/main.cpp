#include <JadeFrame.h>

// #include "example_0.h"
// #include "example_empty.h"
// #include "exmaple_rotating_primitive.h"
// #include "example_texture_quad.h"
#include "example_hello_triangle.h"

#include <string>
#include <iostream>
// using namespace JadeFrame;
class App : public JadeFrame::BaseApp {
public:
public:
    App(Desc desc)
        : BaseApp(desc) {}
    virtual auto on_init() -> void override {}
    virtual auto on_update() -> void override {}
    virtual auto on_draw() -> void override {}
};


int main() {


    JadeFrame::Instance jade_frame;

    using GApp = JadeFrame::TestApp;

    GApp::Desc app_desc;
    app_desc.title = "Test";
    app_desc.size.width = 800;  // = 1280;
    app_desc.size.height = 800; // = 720;
    app_desc.api = JadeFrame::GRAPHICS_API::VULKAN;

    GApp* app = jade_frame.request_app<GApp>(app_desc);
    jade_frame.run();
    return 0;
}
