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