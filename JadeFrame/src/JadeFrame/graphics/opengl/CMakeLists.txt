add_library(GraphicsAPI_OpenGL
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

    "platform/win32/win32.h"
    "platform/win32/win32.cpp"
)

target_include_directories(GraphicsAPI_OpenGL
    PUBLIC
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glad/include/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/spdlog/include/

)
target_link_libraries(GraphicsAPI_OpenGL
    PUBLIC
        ${stb}
        stb
        spdlog
        glad
        shaderc
        spirv-cross-hlsl
        spirv-cross-glsl
        spirv-cross-msl
        JadeFrame
)

target_compile_definitions(GraphicsAPI_OpenGL PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _CRT_SECURE_NO_WARNINGS
    _CONSOLE
    UNICODE
    _UNICODE
)
if(MSVC)
    target_compile_options(GraphicsAPI_OpenGL PRIVATE
        /EHsc
        /Z7
        /Od
        /W4
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(GraphicsAPI_OpenGL PRIVATE
        -Wall 
        -Wextra 
        -Wpedantic 
        # -Werror
    )
elseif(CMKAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(ERROR "GCC is not supported yet")
endif()