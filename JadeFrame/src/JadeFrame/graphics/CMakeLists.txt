add_subdirectory(opengl)
add_subdirectory(vulkan)

set(Files_DirectX
    "d3d11/d3d11_renderer.h"
    "d3d12/d3d12_renderer.h"
)
set(Files
    "camera.h"

    "graphics_shared.h"
    "mesh.h"
    "camera.cpp"
    "graphics_shared.cpp"
    "mesh.cpp"
    "color.h"
    "shader_loader.h"
    "shader_loader.cpp"
    "reflect.h"
    "reflect.cpp"
    "graphics_language.h"
    "graphics_language.cpp"


    "software/software_renderer.h"
    "software/software_renderer.cpp"
)

add_library(JF_MODULE_graphics STATIC ${Files})
target_link_libraries(JF_MODULE_graphics
    INTERFACE
        JF_MODULE_prelude
    PRIVATE
        stb
        JF_MODULE_prelude
        JF_MODULE_math
        JF_MODULE_platform
        shaderc
        spirv-cross-hlsl
        spirv-cross-glsl
        spirv-cross-msl
        gfx-opengl
        gfx-vulkan

)
add_subdirectory(tests)