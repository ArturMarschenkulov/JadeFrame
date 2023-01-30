#include <JadeFrame.h>


int main() {
    JadeFrame::Instance jade_frame;

    using GApp = JadeFrame::TestApp;
    GApp::DESC win_desc;
    win_desc.title = "Test";
    win_desc.size.width = 800;  // = 1280;
    win_desc.size.height = 800; // = 720;
    GApp* app = jade_frame.request_app<GApp>(win_desc);
    jade_frame.run();
    return 0;
}