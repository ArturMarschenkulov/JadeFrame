#pragma once

#include <string>
#include <memory>
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"

#include "window.h"
#include "window_event.h"

namespace JadeFrame {

consteval auto get_cpp_version() -> u32 {

#if defined(_MSC_VER)
    constexpr u32 version_raw = _MSVC_LANG;
#else
    constexpr u32 version_raw = __cplusplus;
#endif
    if constexpr (version_raw == 0) {
        return version_raw;
    } else if constexpr (version_raw >= 202000L) {
        return 20;
    } else if constexpr (version_raw >= 201700L) {
        return 17;
    } else if constexpr (version_raw >= 201400L) {
        return 14;
    } else if constexpr (version_raw >= 202000L) {
        return 11;
    } else if constexpr (version_raw >= 199800L) {
        return 98;
    } else {
        return version_raw;
    }
}

struct CompilerInfo {
    struct Version {
        int major;
        int minor;
        int patch;
    };

    const char* name;
    Version     version;
};

consteval auto get_compiler_info() -> CompilerInfo {
#if defined(__clang__)
    constexpr const char* name = "CLANG";
    constexpr u32         major = __clang_major__;
    constexpr u32         minor = __clang_minor__;
    constexpr u32         patch = __clang_patchlevel__;
#elif defined(__GNUC__)
    constexpr const char* name = "GCC";
    constexpr u32         major = __GNUC__;
    constexpr u32         minor = __GNUC_MINOR__;
    constexpr u32         patch = __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
    constexpr const char* name = "MSVC";
    constexpr u32         major = _MSC_VER / 100;
    constexpr u32         minor = _MSC_VER - (_MSC_VER / 100 * 100);
    constexpr u32         patch = _MSC_FULL_VER - _MSC_VER * 100000;

#elif defined(__INTEL_COMPILER)
    constexpr const char* name = "ICC";
    constexpr u32         major = __INTEL_COMPILER / 100;
    constexpr u32         minor = __INTEL_COMPILER % 100;
    constexpr u32         patch = __INTEL_COMPILER % 10;
#elif defined(__MINGW64__)
    constexpr const char* name = "MINGW64";
    constexpr u32         major = __MINGW64_VERSION_MAJOR;
    constexpr u32         minor = __MINGW64_VERSION_MINOR;
    constexpr u32         patch = 0;
#elif defined(__MINGW32__)
    constexpr const char* name = "MINGW32";
    constexpr u32         major = __MINGW32_MAJOR_VERSION;
    constexpr u32         minor = __MINGW32_MAJOR_VERSION;
    constexpr u32         patch = 0;
#elif defined(CYGWIN)
    constexpr const char* name = "CYGWIN";
    constexpr u32         major = CYGWIN_VERSION_DLL_MAJOR;
    constexpr u32         minor = CYGWIN_VERSION_DLL_MINOR;
    constexpr u32         patch = 0;
#else
    constexpr const char* name = "UNKNOWN";
    constexpr u32         major = 0;
    constexpr u32         minor = 0;
    constexpr u32         patch = 0;
#endif

    constexpr CompilerInfo info = {
        .name = name, .version = {.major = major, .minor = minor, .patch = patch}
    };
    return info;
} // namespace JadeFrame

consteval auto get_plattform_info() -> const char* {
    const char* name = "UNKNOWN";
#if defined(_WIN32)
    name = "WIN32";
#elif defined(__linux__) && !defined(__ANDROID__)
    name = "LINUX";
#elif defined(__ANDROID__)
    name = "ANDROID";
#elif defined(__APPLE__)
    name = "APPLE";
#else
    name = "UNKNOWN";
#endif
    return name;
}

consteval auto get_architecture_info() -> const char* {
    const char* name = "UNKNOWN";
#if defined(__x86_64__) || defined(_M_X64)
    name = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    name = "i386";
#elif defined(__arm__) || defined(_M_ARM)
    name = "ARM";
#elif defined(__aarch64__) || defined(_M_ARM64)
    name = "ARM64";
#else
    name = "UNKNOWN";
#endif
    return name;
}

template<typename T>
class ISystemManager {
public:
    virtual ~ISystemManager() = default;
    virtual auto initialize() -> void = 0;
    virtual auto log() const -> void = 0;

    virtual auto request_window(Window::Desc desc) -> Window* = 0;

    // time management
    [[nodiscard]] auto get_time() const -> f64 {
        return static_cast<const T*>(this)->get_time();
    }

    auto calc_elapsed() -> f64 { return static_cast<T*>(this)->calc_elapsed(); }

    auto frame_control(f64 delta_time) -> void {
        return static_cast<T*>(this)->frame_control(delta_time);
    }

    auto set_target_FPS(f64 FPS) -> void {
        return static_cast<T*>(this)->set_target_FPS(FPS);
    }
};

class IPlatform {
public:
    // platform stuff

    // input stuff

    // time stuff
    [[nodiscard]] virtual auto get_time() const -> f64 = 0;
    virtual auto               calc_elapsed() -> f64 = 0;
    virtual auto               frame_control(f64 delta_time) -> void = 0;
    virtual auto               set_FPS(f64 FPS) -> void = 0;
};

auto get_program_path() -> std::string;
auto load_module(const char* path) -> void*;
auto alloc_page(u64 page_amount) -> void*;
} // namespace JadeFrame
