add_library(platform-win32
    "windows_input_manager.h"
    "windows_input_manager.cpp"
    "windows_shared.h"
    "windows_shared.cpp"
    "windows_system_manager.h"
    "windows_system_manager.cpp"
    "windows_window.h"
    "windows_window.cpp"
)
jadeframe_enable_sanitizers(platform-win32)
jadeframe_enable_warnings(platform-win32)

target_link_libraries(platform-win32
    PUBLIC
        winmm
        shell32
        jadeframe_prelude
    PRIVATE
        jadeframe_logger
)

target_compile_definitions(platform-win32 PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    _CONSOLE
    UNICODE
    _UNICODE
)
