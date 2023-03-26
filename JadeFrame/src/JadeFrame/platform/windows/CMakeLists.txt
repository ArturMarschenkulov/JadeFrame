add_library(Platform_Win32
    "windows_input_manager.h"
    "windows_input_manager.cpp"
    "windows_message_map.h"
    "windows_message_map.cpp"
    "windows_shared.h"
    "windows_shared.cpp"
    "windows_system_manager.h"
    "windows_system_manager.cpp"
    "windows_window.h"
    "windows_window.cpp"
)

target_include_directories(Platform_Win32
    PUBLIC
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/spdlog/include/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/spdlog/include/spdlog/details/

)

target_link_libraries(Platform_Win32
    PUBLIC
        winmm
        spdlog
)

target_compile_definitions(Platform_Win32 PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _CRT_SECURE_NO_WARNINGS
    _CONSOLE
    UNICODE
    _UNICODE
)
if(MSVC)
    target_compile_options(Platform_Win32 PRIVATE
        /EHsc
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Platform_Win32 PRIVATE
        -Wall 
        -Wextra 
        -Wpedantic 
        # -Werror
    )
elseif(CMKAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(ERROR "GCC is not supported yet")
endif()