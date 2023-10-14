#include "linux_system_manager.h"
#include "linux_window.h"

namespace JadeFrame {

auto Linux_SystemManager::initialize() -> void {

    struct timespec timer_resolution;
    timer_resolution.tv_sec = 0;
    timer_resolution.tv_nsec = 1000000;
    clock_settime(CLOCK_MONOTONIC, &timer_resolution);

    struct timespec frequency;
    clock_getres(CLOCK_MONOTONIC, &frequency);

    struct timespec offset;
    clock_gettime(CLOCK_MONOTONIC, &offset);

    m_instance = getpid();
}

auto Linux_SystemManager::log() const -> void {}

auto Linux_SystemManager::request_window(IWindow::Desc desc) -> IWindow* {
    m_windows[m_window_counter] = std::make_unique<Linux_Window>(desc);
    m_window_counter++;
    return m_windows[m_window_counter - 1].get();
}

auto Linux_SystemManager::get_time() const -> f64 {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    const f64 frequency = 1e9; // 1 second

    return static_cast<f64>(ts.tv_sec) + static_cast<f64>(ts.tv_nsec) / frequency;
}

auto Linux_SystemManager::calc_elapsed() -> f64 {
    f64 current = this->get_time();
    f64 update = current - m_time.previous;
    m_time.previous = current;
    return update;
}

auto Linux_SystemManager::frame_control(f64 delta_time) -> void {
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

auto Linux_SystemManager::set_target_FPS(f64 FPS) -> void {
    m_max_FPS = FPS;
    m_time.target = 1.0 / FPS;
}
} // namespace JadeFrame