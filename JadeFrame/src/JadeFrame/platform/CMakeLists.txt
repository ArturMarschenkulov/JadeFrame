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

add_library(jadeframe_platform STATIC ${Files})
jadeframe_enable_sanitizers(jadeframe_platform)

IF(WIN32)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        platform-win32
        opengl32
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
target_link_libraries(jadeframe_platform
    PUBLIC
        jadeframe_math
    PRIVATE
        ${ADDITIONAL_LIBRARY_DEPENDENCIES}
        jadeframe_utils
        jadeframe_logger
)
