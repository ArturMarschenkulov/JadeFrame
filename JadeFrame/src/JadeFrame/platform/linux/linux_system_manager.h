#pragma once

#include <map>
#include <memory>

#include "../platform_shared.h"

#include <unistd.h>

namespace JadeFrame {

class Linux_SystemManager : public ISystemManager<Linux_SystemManager> {
public:
    virtual auto initialize() -> void override;
    virtual auto log() const -> void override;
    virtual auto request_window(IWindow::Desc desc) -> IWindow* override;

public: // Window stuff
    i32 m_window_counter = 0;
    using WindowID = i32;
    std::map<WindowID, IWindow> m_windows;
    IWindow*                    m_curr_window = nullptr;

public: // Time stuff
    auto get_time() const -> f64;
    auto calc_elapsed() -> f64;
    auto frame_control(f64 delta_time) -> void;
    auto set_target_FPS(f64 FPS) -> void;

    struct Time {
        f64 previous = 0;
        f64 target = 0;
    };

    Time m_time;
    f32  m_max_FPS;

    // private:
    //     bool m_has_performance_counter;
    //     u64  m_frequency;
    //     u64  m_offset;

public:
    pid_t m_instance;
};

} // namespace JadeFrame