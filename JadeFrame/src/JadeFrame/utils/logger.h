#pragma once
//#include <format>
#include <iostream>
#include <memory>
#include <string>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"


/*
Little guide:
- TRACE: Very detailed information, which is not needed in production
- DEBUG: Information which is needed in production, but not in release
- INFO: Information which is needed in release
- WARN: Something which is not critical, but should be fixed
- ERROR: Something which is critical and should be fixed
- CRITICAL: Something which is critical and should be fixed as soon as possible
*/

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
    static auto log(fmt::format_string<Types...> text, Types&&... args) -> void;
    // template<class T>
    // static auto log(const T& text) -> void;
    template<class... Types>
    static auto log(LEVEL level, fmt::format_string<Types...> text, Types&&... args) -> void;
    // template<class T>
    // static auto log(LEVEL level, const T& text) -> void;



    template<class... Types>
    static auto trace(fmt::format_string<Types...> text, Types&&... args) -> void;
    template<class... Types>
    static auto debug(fmt::format_string<Types...> text, Types&&... args) -> void;
    template<class... Types>
    static auto info(fmt::format_string<Types...> text, Types&&... args) -> void;
    template<class... Types>
    static auto warn(fmt::format_string<Types...> text, Types&&... args) -> void;
    template<class... Types>
    static auto err(fmt::format_string<Types...> text, Types&&... args) -> void;
    template<class... Types>
    static auto critical(fmt::format_string<Types...> text, Types&&... args) -> void;

    // template<class T>
    // static auto trace(const T& text) -> void;
    // template<class T>
    // static auto debug(const T& text) -> void;
    // template<class T>
    // static auto info(const T& text) -> void;
    // template<class T>
    // static auto warn(const T& text) -> void;
    // template<class T>
    // static auto err(const T& text) -> void;
    // template<class T>
    // static auto critical(const T& text) -> void;



    static auto init() -> void;
    static auto deinit() -> void;

    static std::shared_ptr<spdlog::logger> s_core;
    static std::shared_ptr<spdlog::logger> s_client;
    static std::shared_ptr<spdlog::logger> s_editor;
};

template<class... Types>
auto Logger::log(fmt::format_string<Types...> text, Types&&... args) -> void {
    Logger::log(LEVEL::WARN, text, std::forward<Types>(args)...);
}
// template<class T>
// auto Logger::log(const T& text) -> void {
//     Logger::log(LEVEL::WARN, text);
// }

template<class... Types>
auto Logger::log(LEVEL level, fmt::format_string<Types...> text, Types&&... args) -> void {
    spdlog::level::level_enum lvl;
    switch (level) {
        case LEVEL::TRACE: lvl = spdlog::level::trace; break;
        case LEVEL::DEBUG: lvl = spdlog::level::debug; break;
        case LEVEL::INFO: lvl = spdlog::level::info; break;
        case LEVEL::WARN: lvl = spdlog::level::warn; break;
        case LEVEL::ERR: lvl = spdlog::level::err; break;
        case LEVEL::CRITICAL: lvl = spdlog::level::critical; break;
        default: lvl = spdlog::level::off; break;
    }
    s_core->log(lvl, text, std::forward<Types>(args)...);
}
// template<class T>
// auto Logger::log(LEVEL level, const T& text) -> void {
//     spdlog::level::level_enum lvl;
//     switch (level) {
//         case LEVEL::TRACE: lvl = spdlog::level::trace; break;
//         case LEVEL::DEBUG: lvl = spdlog::level::debug; break;
//         case LEVEL::INFO: lvl = spdlog::level::info; break;
//         case LEVEL::WARN: lvl = spdlog::level::warn; break;
//         case LEVEL::ERR: lvl = spdlog::level::err; break;
//         case LEVEL::CRITICAL: lvl = spdlog::level::critical; break;
//         default: lvl = spdlog::level::off; break;
//     }
//     s_core->log(lvl, text);
// }




template<class... Types>
auto Logger::trace(fmt::format_string<Types...> text, Types&&... args) -> void {
    // s_core->trace(text, std::forward<Types>(args)...);
    Logger::log(LEVEL::TRACE, text, std::forward<Types>(args)...);
}
template<class... Types>
auto Logger::debug(fmt::format_string<Types...> text, Types&&... args) -> void {
    // s_core->debug(text, std::forward<Types>(args)...);
    Logger::log(LEVEL::DEBUG, text, std::forward<Types>(args)...);
}
template<class... Types>
auto Logger::info(fmt::format_string<Types...> text, Types&&... args) -> void {
    // s_core->info(text, std::forward<Types>(args)...);
    Logger::log(LEVEL::INFO, text, std::forward<Types>(args)...);
}
template<class... Types>
auto Logger::warn(fmt::format_string<Types...> text, Types&&... args) -> void {
    // s_core->warn(text, std::forward<Types>(args)...);
    Logger::log(LEVEL::WARN, text, std::forward<Types>(args)...);
}
template<class... Types>
auto Logger::err(fmt::format_string<Types...> text, Types&&... args) -> void {
    // s_core->error(text, std::forward<Types>(args)...);
    Logger::log(LEVEL::ERR, text, std::forward<Types>(args)...);
}
template<class... Types>
auto Logger::critical(fmt::format_string<Types...> text, Types&&... args) -> void {
    // s_core->critical(text, std::forward<Types>(args)...);
    Logger::log(LEVEL::CRITICAL, text, std::forward<Types>(args)...);
}

// template<class T>
// auto Logger::trace(const T& text) -> void {
//     // s_core->trace(text, std::forward<Types>(args)...);
//     Logger::log(LEVEL::TRACE, text);
// }
// template<class T>
// auto Logger::debug(const T& text) -> void {
//     // s_core->debug(text, std::forward<Types>(args)...);
//     Logger::log(LEVEL::DEBUG, text);
// }
// template<class T>
// auto Logger::info(const T& text) -> void {
//     // s_core->info(text, std::forward<Types>(args)...);
//     Logger::log(LEVEL::INFO, text);
// }
// template<class T>
// auto Logger::warn(const T& text) -> void {
//     // s_core->warn(text, std::forward<Types>(args)...);
//     Logger::log(LEVEL::WARN, text);
// }
// template<class T>
// auto Logger::err(const T& text) -> void {
//     // s_core->error(text, std::forward<Types>(args)...);
//     Logger::log(LEVEL::ERR, text);
// }
// template<class T>
// auto Logger::critical(const T& text) -> void {
//     // s_core->critical(text, std::forward<Types>(args)...);
//     Logger::log(LEVEL::CRITICAL, text);
// }
} // namespace JadeFrame