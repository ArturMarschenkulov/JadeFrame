add_library(Platform_Linux
    "linux_input_manager.h"

    "linux_system_manager.h"
    "linux_system_manager.cpp"
    
    "linux_window.h"
    "linux_window.cpp"
)

target_link_libraries(Platform_Linux
    PUBLIC
        spdlog
        JadeFrame
)

target_compile_definitions(Platform_Linux PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)
if(MSVC)
    target_compile_options(Platform_Linux PRIVATE
        /EHsc
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(Platform_Linux PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -Wconversion
        -Wpedantic
        # -Wno-unused
        # -Wno-unused-parameter
        -Werror
    )
endif()