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

target_link_libraries(platform-win32
    PUBLIC
        winmm
        JF_MODULE_prelude
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
# if(MSVC)
#     target_compile_options(platform-win32 PRIVATE
#         /EHsc
#     )
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     target_compile_options(platform-win32 PRIVATE
#         -Wall 
#         -Wextra 
#         -Wpedantic 
#         # -Werror
#     )
# elseif(CMKAKE_CXX_COMPILER_ID STREQUAL "GNU")
#     message(ERROR "GCC is not supported yet")
# endif()