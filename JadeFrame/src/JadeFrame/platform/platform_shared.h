#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/math/vec.h"
#include <string>
namespace JadeFrame {
    class IWindow {
    public:
        enum class WINDOW_STATE {
            WINDOWED,
            MINIMIZED,
            MAXIMIZED,
        };

        struct Desc {
            std::string title;
            v2u32 size;
            v2u32 position; // NOTE: -1 means randomly chosen by OS
            //bool is_vsync;
            WINDOW_STATE window_state = WINDOW_STATE::WINDOWED;
            bool visable = true;
            bool accept_drop_files = false;
        };

        virtual auto set_title(const std::string& title) -> void = 0;
        virtual auto get_title() const -> std::string = 0;
        virtual auto get_window_state() const -> WINDOW_STATE = 0;
    };
    class ITimeManager {
        virtual auto initialize() -> void = 0;
        virtual auto get_time() const -> f64 = 0;
        virtual auto calc_elapsed() -> f64 = 0;
        virtual auto frame_control(f64 delta_time) -> void = 0;
        virtual auto set_FPS(f64 FPS) -> void = 0;
    };
    class ISystemManager {
    public:
        virtual auto initialize() -> void = 0;
        virtual auto log() const -> void = 0;
    };
    class IInputManager {
    public:
        //static auto is_key_down(const KEY key) -> bool;
        //static auto is_key_up(const KEY key) -> bool;
        //static auto is_key_pressed(const KEY key) -> bool;
        //static auto is_key_released(const KEY key) -> bool;
    
    };
}
