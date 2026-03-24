#include "linux_system_manager.h"
#include "JadeFrame/utils/logger.h"
#include "linux_window.h"

namespace JadeFrame {

static auto print_linux_warn(const char* message) -> void {
    int         err = errno;
    std::string err_string = std::string(std::strerror(err));
    Logger::warn("{} Error code: {}, message: {}", message, err, err_string);
}

auto Linux_SystemManager::initialize() -> void { m_instance = getpid(); }

auto Linux_SystemManager::log() const -> void {}

auto Linux_SystemManager::request_window(Window::Desc desc) -> Window* {
    m_windows[m_window_counter] = Window(desc);
    m_window_counter++;
    return &m_windows[m_window_counter - 1];
}

auto Linux_SystemManager::get_time() const -> f64 {
    timespec ts = {};
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        print_linux_warn("Failed to get current time.");
        return 0.0;
    }
    const f64 nanoseconds_per_second = 1'000'000'000.0; // 1 second
    return static_cast<f64>(ts.tv_sec) +
           (static_cast<f64>(ts.tv_nsec) / nanoseconds_per_second);
}

namespace {
auto to_timespec(f64 seconds) -> timespec {
    if (seconds <= 0.0) { return timespec{}; }
    const f64 nanoseconds_per_second = 1'000'000'000.0; // 1 second

    const auto whole_seconds = static_cast<time_t>(seconds);
    const auto fractional = seconds - static_cast<f64>(whole_seconds);
    const auto nanoseconds = static_cast<long>(fractional * nanoseconds_per_second);

    return timespec{.tv_sec = whole_seconds, .tv_nsec = nanoseconds};
}
} // namespace

auto Linux_SystemManager::frame_control(f64 frame_time) -> void {
    if (m_time.target <= 0.0) { return; }
    if (frame_time >= m_time.target) { return; }
    const auto remaining = m_time.target - frame_time;

    auto sleep_time = to_timespec(remaining);
    while (nanosleep(&sleep_time, &sleep_time) == -1 && errno == EINTR) {}
}

auto Linux_SystemManager::set_target_FPS(f64 FPS) -> void {
    m_max_FPS = static_cast<f32>(FPS);
    m_time.target = 1.0 / FPS;
}
} // namespace JadeFrame