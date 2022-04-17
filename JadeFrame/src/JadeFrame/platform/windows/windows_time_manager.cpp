#include "pch.h"
#include "windows_time_manager.h"
#include <Windows.h>
#include "JadeFrame/utils/option.h"
//#include <timeapi.h>
// #pragma comment(lib,"winmm.lib")

namespace JadeFrame {


auto query_performance_frequency() -> Option<u64> {
    u64 frequency;
    if (::QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
        return Option<u64>(std::move(frequency));
    } else {
        return Option<u64>();
    }
}
auto query_performance_counter() -> Option<u64> {
    u64 counter;
    if (::QueryPerformanceCounter((LARGE_INTEGER*)&counter)) {
        return Option<u64>(std::move(counter));
    } else {
        return Option<u64>();
    }
}
auto Windows_TimeManager::initialize() -> void {
    timeBeginPeriod(1);

    Option<u64> frequency = query_performance_frequency();
    if (frequency.is_some()) {
        m_has_performance_counter = true;
        m_frequency = frequency.unwrap_unchecked();
    } else {
        m_has_performance_counter = false;
        m_frequency = 1000;
    }

    m_offset = query_performance_counter().unwrap_unchecked();
}

auto Windows_TimeManager::get_time() const -> f64 {
    const u64 counter = query_performance_counter().unwrap_unchecked() - m_offset;
    const u64 frequency = this->get_timer_frequency();
    return static_cast<f64>(counter) / frequency;
}

auto Windows_TimeManager::get_timer_frequency() const -> u64 { return m_frequency; }

auto Windows_TimeManager::calc_elapsed() -> f64 {
    time.current = this->get_time();
    time.update = time.current - time.previous;
    time.previous = time.current;
    return time.update;
}

auto Windows_TimeManager::frame_control(f64 delta_time) -> void {
    // Frame time control system
    time.current = this->get_time();
    time.draw = time.current - time.previous;
    time.previous = time.current;

    time.frame = /*time.update*/ delta_time + time.draw;

    if (time.frame < time.target) {
        ::Sleep(static_cast<u32>((time.target - time.frame) * 1000.0));
        time.current = this->get_time();
        f64 time_wait = time.current - time.previous;
        time.previous = time.current;
        time.frame += time_wait;
    }
    //__debugbreak();
}

auto Windows_TimeManager::set_FPS(f64 FPS) -> void {
    max_FPS = static_cast<i32>(FPS);
    time.target = 1 / (f64)FPS;
}
} // namespace JadeFrame