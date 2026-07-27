#include "logger.h"

#include <cstdio>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace JadeFrame {
std::shared_ptr<spdlog::logger> Logger::s_core;

auto report_assert_failure(
    const char* expression,
    const char* message,
    const char* file,
    int         line
) noexcept -> void {
    if (Logger::s_core) {
        Logger::err(
            "Assertion failed: {} ({}) at {}:{}",
            expression,
            message,
            file,
            line
        );
        return;
    }

    std::fprintf(
        stderr,
        "Assertion failed: %s (%s) at %s:%d\n",
        expression,
        message,
        file,
        line
    );
}

auto Logger::init() -> void {
    std::vector<spdlog::sink_ptr> jf_sinks = {
        std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/JadeFrame.log", true)};

    jf_sinks[0]->set_pattern("%^[%T] [%L] %n: %v%$");
    s_core =
        std::make_shared<spdlog::logger>("JadeFrame", jf_sinks.begin(), jf_sinks.end());
    s_core->set_level(spdlog::level::trace);
}

auto Logger::deinit() -> void {
    s_core.reset();
    spdlog::drop_all();
}
} // namespace JadeFrame
