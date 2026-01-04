IF (WIN32)
    add_subdirectory(windows/)
ELSEIF(UNIX AND NOT APPLE)
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

IF(WIN32)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        platform-win32
        opengl32
        ${Vulkan_LIBRARIES}
    )
ELSEIF(UNIX AND NOT APPLE)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        platform-linux
        pthread
        X11
        # GL
        # libvulkan.so
    )
ENDIF()
target_link_libraries(JF_MODULE_platform
    PRIVATE
        ${ADDITIONAL_LIBRARY_DEPENDENCIES}
        JF_MODULE_prelude
        imgui
)

