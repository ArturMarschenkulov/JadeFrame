#pragma once
#include "../platform_shared.h"

namespace JadeFrame {

class Linux_SystemManager : public ISystemManager {
public:
    virtual auto initialize() -> void override {}
    virtual auto log() const -> void override {}
    virtual auto request_window(IWindow::Desc desc) -> IWindow* override { return nullptr; }


public: // Time stuff
private:
    bool m_has_performance_counter;
    u64  m_frequency;
    u64  m_offset;

public:
    virtual auto get_time() const -> f64 override {
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        const f64 frequency = 1e9; // 1 second

        return static_cast<f64>(ts.tv_sec) + static_cast<f64>(ts.tv_nsec) / frequency;
    }
    virtual auto calc_elapsed() -> f64 override {
        f64 current = this->get_time();
        f64 update = current - m_time.previous;
        m_time.previous = current;
        return update;
    }
    virtual auto frame_control(f64 delta_time) -> void override {
        f64 current = this->get_time();
        f64 draw = current - m_time.previous;
        m_time.previous = current;

        f64 frame = delta_time + draw;

        if (frame < m_time.target) {
            struct timespec sleep_time;
            sleep_time.tv_sec = 0;
            sleep_time.tv_nsec = (m_time.target - frame) * 1e9;
            nanosleep(&sleep_time, nullptr);

            f64 current = this->get_time();
            f64 time_wait = current - m_time.previous;
            m_time.previous = current;
            frame += time_wait;
        }
    }
    virtual auto set_target_FPS(f64 FPS) -> void override {
		m_max_FPS = FPS;
		m_time.target = 1.0 / FPS;
	}

    struct Time {
        f64 previous = 0;
        f64 target = 0;
    };
    Time m_time;
    f32  m_max_FPS;
};

} // namespace JadeFrame