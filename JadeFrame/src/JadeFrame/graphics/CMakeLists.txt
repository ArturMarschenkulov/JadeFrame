set(Core_Files
    "camera.h"

    "graphics_shared.h"
    "mesh.h"
    "camera.cpp"
    "graphics_core.cpp"
    "mesh.cpp"
    "color.h"
    "shader_loader.h"
    "shader_loader.cpp"
    "reflect.h"
    "reflect.cpp"
    "graphics_language.h"
    "graphics_language.cpp"

)

add_library(jadeframe_core STATIC ${Core_Files})
jadeframe_enable_sanitizers(jadeframe_core)
target_link_libraries(jadeframe_core
    PUBLIC
        jadeframe_math
        jadeframe_utils
    PRIVATE
        stb
        shaderc
        spirv-cross-hlsl
        spirv-cross-glsl
        spirv-cross-msl
)

if(JADEFRAME_ENABLE_OPENGL)
    add_subdirectory(opengl)
endif()

if(JADEFRAME_ENABLE_VULKAN)
    add_subdirectory(vulkan)
endif()

# This target owns the façade implementation that selects and constructs a
# concrete backend. Backends depend only on jadeframe_core/platform and never
# link back to this target.
add_library(jadeframe_graphics STATIC
    "graphics_shared.cpp"
)
jadeframe_enable_sanitizers(jadeframe_graphics)
target_link_libraries(jadeframe_graphics
    PUBLIC
        jadeframe_core
    PRIVATE
        jadeframe_platform
        stb
)

if(JADEFRAME_ENABLE_OPENGL)
    target_link_libraries(jadeframe_graphics PRIVATE jadeframe_backend_opengl)
    target_compile_definitions(jadeframe_graphics PRIVATE JADEFRAME_ENABLE_OPENGL=1)
endif()

if(JADEFRAME_ENABLE_VULKAN)
    target_link_libraries(jadeframe_graphics PRIVATE jadeframe_backend_vulkan)
    target_compile_definitions(jadeframe_graphics PRIVATE JADEFRAME_ENABLE_VULKAN=1)
endif()


if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
