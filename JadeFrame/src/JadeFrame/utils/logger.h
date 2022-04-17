#pragma once
//#include <format>
#include <iostream>
#include <memory>
#include <string>

// #include "extern/spdlog/spdlog.h"
// #include "extern/spdlog/fmt/ostr.h"
// #include "extern/spdlog/sinks/base_sink.h"
// #include "extern/spdlog/sinks/stdout_color_sinks.h"


namespace JadeFrame {
class Logger {
public:
    enum class LEVEL {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERR,
        CRITICAL,
        OFF
    };
    template<class... Types>
    static auto log(const char* text, const Types&... args) -> void;
    template<class... Types>
    static auto log(LEVEL level, const char* text, const Types&... args) -> void;

    template<class... Types>
    static auto trace(const char* text, const Types&... args) -> void;
    template<class... Types>
    static auto debug(const char* text, const Types&... args) -> void;
    template<class... Types>
    static auto info(const char* text, const Types&... args) -> void;
    template<class... Types>
    static auto warn(const char* text, const Types&... args) -> void;
    template<class... Types>
    static auto err(const char* text, const Types&... args) -> void;
    template<class... Types>
    static auto critical(const char* text, const Types&... args) -> void;


    static auto init() -> void;
    static auto deinit() -> void;

    // static std::shared_ptr<spdlog::logger> s_core;
    // static std::shared_ptr<spdlog::logger> s_client;
    // static std::shared_ptr<spdlog::logger> s_editor;
};

template<class... Types>
auto Logger::log(const char* text, const Types&... args) -> void {
    Logger::log(LEVEL::WARN, text, args...);
    // std::string s = std::format<Types...>(text, args...);
    // std::cout << s << "\n";
}
template<class... Types>
auto Logger::log(LEVEL /*level*/, const char* text, const Types&... /*args*/) -> void {
    // warn(text, args...);
    // std::string s = std::format<Types...>(text, args...);
    std::cout << text << "\n";
}




template<class... Types>
auto Logger::trace(const char* text, const Types&... args) -> void {
    // s_core->trace(text, args...);
    Logger::log(LEVEL::TRACE, text, args...);
}
template<class... Types>
auto Logger::debug(const char* text, const Types&... args) -> void {
    // s_core->debug(text, args...);
    Logger::log(LEVEL::DEBUG, text, args...);
}
template<class... Types>
auto Logger::info(const char* text, const Types&... args) -> void {
    // s_core->info(text, args...);
    Logger::log(LEVEL::INFO, text, args...);
}
template<class... Types>
auto Logger::warn(const char* text, const Types&... args) -> void {
    // s_core->warn(text, args...);
    Logger::log(LEVEL::WARN, text, args...);
}
template<class... Types>
auto Logger::err(const char* text, const Types&... args) -> void {
    // s_core->error(text, args...);
    Logger::log(LEVEL::ERR, text, args...);
}
template<class... Types>
auto Logger::critical(const char* text, const Types&... args) -> void {
    // s_core->critical(text, args...);
    Logger::log(LEVEL::CRITICAL, text, args...);
}
} // namespace JadeFrame