
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

target_link_libraries(gfx-opengl
    PUBLIC
        ${stb}
        stb
        spdlog
        glad
        spirv-cross-hlsl
        spirv-cross-glsl
        spirv-cross-msl
        JadeFrame
)
