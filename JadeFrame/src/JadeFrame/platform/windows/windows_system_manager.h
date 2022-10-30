#pragma once
#include "JadeFrame/prelude.h"
#include "../platform_shared.h"
#include <string>
namespace JadeFrame {

namespace win32 {


auto to_wide_char(const char* char_array) -> wchar_t*;
auto to_multi_byte(const wchar_t* wide_char_array) -> char*;
auto from_wstring_to_string(const std::wstring& wstr) -> std::string;
auto from_string_to_wstring(const std::string& str) -> std::wstring;


class SystemManager : ISystemManager {
public:
    virtual auto initialize() -> void override;
    virtual auto log() const -> void override;

    virtual auto request_window(IWindow::Desc desc) -> IWindow* override;

public: // Window stuff
    i32 m_window_counter = 0;
    using WindowID = i32;
    std::map<WindowID, std::unique_ptr<IWindow>> m_windows;
    IWindow*                                     m_curr_window = nullptr;

public: // Time stuff
private:
    bool m_has_performance_counter;
    u64  m_frequency;
    u64  m_offset;

public:
    virtual auto get_time() const -> f64 override;
    virtual auto calc_elapsed() -> f64 override;
    virtual auto frame_control(f64 delta_time) -> void override;
    virtual auto set_target_FPS(f64 FPS) -> void override;
    struct Time {
        f64 previous = 0;
        f64 target = 0;
    };
    Time time;
    f32  max_FPS;

public:
    ::HMODULE m_instance;
    // represents Dynamically Linked Libraries (DLL) on Windows and Shared Object (SO) on Linux
    struct Module {

        std::string m_name;
        std::string m_path;
        u32         m_id;
        u32         m_process_id;
        u32         m_global_usage_count;
        u32         m_process_usage_count;
    };
    std::vector<Module> m_modules;

    std::string m_user_locale;
    std::string m_system_locale;
    std::string m_computer_name;
    std::string m_user_name;

    std::string m_cpu_name;

    i64 m_available_physical_memory;
    i64 m_total_physical_memory;
    i64 m_available_virtual_memory;
    i64 m_total_virtual_memory;

    i64 m_window_version_major;
    i64 m_window_version_minor;

    u32 m_cache_line_size;
    u32 m_L1_cache_size;
    u32 m_L2_cache_size;
    u32 m_L3_cache_size;
    u32 m_processor_package_count;
    u32 m_processor_core_count;
    u32 m_logical_processor_count;

    u16   m_processor_architecture;
    u32   m_page_size;
    void* m_minimum_application_address;
    void* m_maximum_application_address;
    u64   m_active_processor_mask;
    u32   m_number_of_processors;
    u32   m_allocation_granularity;
    u16   m_processor_level;
    u16   m_processor_revision;
};
} // namespace win32
} // namespace JadeFrame