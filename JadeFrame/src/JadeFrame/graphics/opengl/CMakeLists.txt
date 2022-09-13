add_library(GraphicsAPI_OpenGL
    "opengl_buffer.h"
    "opengl_context.h"
    "opengl_debug.h"
    "opengl_renderer.h"
    "opengl_shader.h"
    "opengl_texture.h"
    "opengl_windows.h"
    "opengl_wrapper.h"

    "opengl_buffer.cpp"
    "opengl_context.cpp"
    "opengl_debug.cpp"
    "opengl_renderer.cpp"
    "opengl_shader.cpp"
    "opengl_texture.cpp"
    "opengl_windows.cpp"
    "opengl_wrapper.cpp"
)

# PROJECT_SOURCE_DIR
target_include_directories(GraphicsAPI_OpenGL
    PUBLIC
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/JadeFrame/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glad/include/

)
target_link_libraries(GraphicsAPI_OpenGL
    PUBLIC
        ${stb}
        stb
)
target_compile_definitions(GraphicsAPI_OpenGL PRIVATE
    "$<$<CONFIG:Debug>:_DEBUG>"
    "$<$<CONFIG:Release>:NDEBUG>"
    "_CRT_SECURE_NO_WARNINGS"
    "_CONSOLE"
    "UNICODE"
    "_UNICODE"
)
target_compile_options(GraphicsAPI_OpenGL PRIVATE
    /EHsc
)