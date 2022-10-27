#pragma once
#include "JadeFrame/prelude.h"
#include "../platform_shared.h"
#include <cstdint>

namespace JadeFrame {
    namespace win32 {
class TimeManager : public ITimeManager {
public:
    virtual auto initialize() -> void override;
    virtual auto get_time() const -> f64 override;

private:
    auto get_timer_frequency() const -> u64;

private:
    bool m_has_performance_counter;
    u64  m_frequency;
    u64  m_offset;

public:
    virtual auto calc_elapsed() -> f64 override;
    virtual auto frame_control(f64 delta_time) -> void override;
    virtual auto set_FPS(f64 FPS) -> void override;
    struct Time {
        f64 previous = 0;
        f64 target = 0;
    };
    Time time;
    f32  max_FPS;
};
    }
} // namespace JadeFrame