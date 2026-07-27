
if(WIN32)
    set(PLATFORM_DEPENDENT_FILES
        "platform/win32/win32.h"
        "platform/win32/win32.cpp"
    )
elseif(LINUX)
    set(PLATFORM_DEPENDENT_FILES
        "platform/linux/linux.h"
        "platform/linux/linux.cpp"
    )
endif()


add_library(jadeframe_backend_opengl
    "opengl_buffer.h"
    "opengl_context.h"
    "opengl_debug.h"
    "opengl_renderer.h"
    "opengl_shader.h"
    "opengl_texture.h"
    "opengl_wrapper.h"

    "opengl_buffer.cpp"
    "opengl_context.cpp"
    "opengl_debug.cpp"
    "opengl_renderer.cpp"
    "opengl_shader.cpp"
    "opengl_texture.cpp"
    "opengl_wrapper.cpp"

    ${PLATFORM_DEPENDENT_FILES}
)
jadeframe_enable_sanitizers(jadeframe_backend_opengl)
IF(WIN32)
    set(PLATFORM_DEPENDENT_DEPENDENCIES
        opengl32
    )
ELSEIF(UNIX AND NOT APPLE)
    set(PLATFORM_DEPENDENT_DEPENDENCIES
        GL
    )
ENDIF()

target_link_libraries(jadeframe_backend_opengl
    PUBLIC
        jadeframe_core
        glad
    PRIVATE
        ${PLATFORM_DEPENDENT_DEPENDENCIES}
        stb
        jadeframe_platform
        jadeframe_logger
        spirv-cross-hlsl
        spirv-cross-glsl
        spirv-cross-msl
)
