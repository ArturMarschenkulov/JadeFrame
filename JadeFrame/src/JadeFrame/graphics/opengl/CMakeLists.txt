
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


add_library(gfx-opengl
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
IF(WIN32)
    set(PLATFORM_DEPENDENT_DEPENDENCIES
        opengl32
    )
ELSEIF(UNIX AND NOT APPLE)
    set(PLATFORM_DEPENDENT_DEPENDENCIES
        GL
    )
ENDIF()

target_link_libraries(gfx-opengl
    PUBLIC
        glad
    PRIVATE
        ${PLATFORM_DEPENDENT_DEPENDENCIES}
        
        stb
        JF_MODULE_prelude
        # JF_MODULE_math
        
        spirv-cross-hlsl
        spirv-cross-glsl
        spirv-cross-msl
        
)
