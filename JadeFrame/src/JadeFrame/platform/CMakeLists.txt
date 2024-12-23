IF (WIN32)
    add_subdirectory(windows/)
ELSEIF(LINUX)
    add_subdirectory(linux/)
ENDIF()
set(Files
    "platform_shared.h"
    "platform_shared.cpp"
    "window.h"
    "window.cpp"
    "window_event.h"
)

add_library(JF_MODULE_platform STATIC ${Files})
target_link_libraries(JF_MODULE_platform
    PUBLIC
        JadeFrame
)
