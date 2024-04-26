#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/math/vec.h"
#include <string>
#include <memory>

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
        name, {major, minor, patch}
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
#elif
    name = "UNKNOWN";
#endif
    return name;
}

enum class INPUT_STATE {
    RELEASED,
    PRESSED
};

enum class BUTTON {
    LEFT,
    RIGHT,
    MIDDLE,
    X1,
    X2,
    MAX
};
enum class KEY {
    SPACE,
    ESCAPE,
    ENTER,
    TAB,
    BACKSPACE,
    INSERT,
    DELET,
    RIGHT,
    LEFT,
    DOWN,
    UP,
    PAGE_UP,
    PAGE_DOWN,
    HOME,
    END,
    CAPS_LOCK,
    SCROLL_LOCK,
    NUM_LOCK,
    PRINT_SCREEN,
    PAUSE,

    // Extra. Names may be changed
    OEM_1,
    OEM_PLUS,
    OEM_COMMA,
    OEM_MINUS,
    OEM_PERIOD,
    OEM_2,
    OEM_3,
    OEM_4,
    OEM_5,
    OEM_6,
    OEM_7,
    OEM_8,
    OEM_AX,
    OEM_102,
    ICO_HELP,
    ICO_00,
    // The F keys
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    LEFT_SHIFT,
    LEFT_CONTROL,
    LEFT_ALT,
    RIGHT_SHIFT,
    RIGHT_CONTROL,
    RIGHT_ALT,
    GRAVE,
    SLASH,
    BACKSLASH,

    LEFT_SUPER, // Windows key

    // The number keys
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,

    // The letter keys
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    MAX
};

class NativeWindow;

class IWindow {
public:
    enum class WINDOW_STATE {
        WINDOWED,
        MINIMIZED,
        MAXIMIZED,
    };

    struct Desc {
        std::string title;
        v2u32       size;
        v2u32       position; // NOTE: -1 means randomly chosen by OS
        // bool is_vsync;
        WINDOW_STATE window_state = WINDOW_STATE::WINDOWED;
        bool         visable = true;
        bool         accept_drop_files = false;
    };

    IWindow() = default;

    explicit IWindow(const IWindow::Desc& desc);

    auto               handle_events(bool& running) -> void;
    [[nodiscard]] auto get_window_state() const -> WINDOW_STATE;
    auto               set_title(const std::string& title) -> void;
    [[nodiscard]] auto get_title() const -> std::string;
    [[nodiscard]] auto get_size() const -> const v2u32&;

public:
    std::unique_ptr<NativeWindow> m_native_window;
    WINDOW_STATE                  m_window_state;
};

// This class represents an interface for a native window, that is a win32, x11, wayland
// etc window. The purpose of this class is to abstract the platform specific window
// creation and event handling.
class NativeWindow {
public:

public:
    NativeWindow() = default;
    NativeWindow(const NativeWindow&) = delete;
    auto operator=(const NativeWindow&) -> NativeWindow& = delete;
    NativeWindow(NativeWindow&&) = delete;
    auto operator=(NativeWindow&&) -> NativeWindow& = delete;
    virtual ~NativeWindow() = default;

public:
    virtual auto               handle_events(bool& running) -> void = 0;
    virtual auto               set_title(const std::string& title) -> void = 0;
    [[nodiscard]] virtual auto get_title() const -> std::string = 0;
    [[nodiscard]] virtual auto get_size() const -> const v2u32& = 0;
};

template<typename T>
class ISystemManager {
public:
    virtual ~ISystemManager() = default;
    virtual auto initialize() -> void = 0;
    virtual auto log() const -> void = 0;

    virtual auto request_window(IWindow::Desc desc) -> IWindow* = 0;

    // time management
    auto get_time() const -> f64 { return static_cast<const T*>(this)->get_time(); }

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
    virtual auto get_time() const -> f64 = 0;
    virtual auto calc_elapsed() -> f64 = 0;
    virtual auto frame_control(f64 delta_time) -> void = 0;
    virtual auto set_FPS(f64 FPS) -> void = 0;
};

template<typename T>
class IInputManager {
public:
    virtual auto handle_input() -> void { static_cast<T*>(this)->handle_input(); }

    static auto is_key_down(const KEY key) -> bool {
        return IInputManager<T>::is_key_down(key);
    }

    static auto is_key_up(const KEY key) -> bool {
        return IInputManager<T>::is_key_up(key);
    }

    static auto is_key_pressed(const KEY key) -> bool {
        return IInputManager<T>::is_key_pressed(key);
    }

    static auto is_key_released(const KEY key) -> bool {
        return IInputManager<T>::is_key_released(key);
    }
};
} // namespace JadeFrame
