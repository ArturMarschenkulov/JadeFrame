add_library(platform-linux
    "linux_input_manager.h"
    "linux_input_manager.cpp"

    "linux_system_manager.h"
    "linux_system_manager.cpp"
    
    "linux_window.h"
    "linux_window.cpp"
)

target_link_libraries(platform-linux
    PUBLIC
        JF_MODULE_prelude
)