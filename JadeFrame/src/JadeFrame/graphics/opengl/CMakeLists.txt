



set(imgui2_DIR 
    ${PROJECT_SOURCE_DIR}/JadeFrame/src/extern/imgui/
)
include_directories(${imgui2_DIR} 
    ${imgui2_DIR}/backends
)
add_library(imgui2
STATIC
    ${imgui_DIR}/imgui_demo.cpp
    ${imgui_DIR}/imgui_draw.cpp
    ${imgui_DIR}/imgui_tables.cpp
    ${imgui_DIR}/imgui_widgets.cpp
    ${imgui_DIR}/imgui.cpp

    ${imgui_DIR}/backends/imgui_impl_opengl3.h
    ${imgui_DIR}/backends/imgui_impl_opengl3.cpp

    # TODO: This is Windows only!!!!
    ${imgui_DIR}/backends/imgui_impl_win32.h
    ${imgui_DIR}/backends/imgui_impl_dx11.h
    ${imgui_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_DIR}/backends/imgui_impl_dx11.cpp
)
set(
    OPENGL_FILES
    "opengl_buffer.h"
    "opengl_context.h"
    "opengl_debug.h"
    "opengl_renderer.h"
    "opengl_shader.h"
    "opengl_shader_loader.h"
    "opengl_texture.h"
    "opengl_windows.h"
    "opengl_wrapper.h"
    "opengl_buffer.cpp"
    "opengl_context.cpp"
    "opengl_debug.cpp"
    "opengl_renderer.cpp"
    "opengl_shader.cpp"
    "opengl_shader_loader.cpp"
    "opengl_texture.cpp"
    "opengl_windows.cpp"
    "opengl_wrapper.cpp"
)

add_library(
    Files_Graphics_OpenGL
    "opengl_buffer.h"
    "opengl_context.h"
    "opengl_debug.h"
    "opengl_renderer.h"
    "opengl_shader.h"
    "opengl_shader_loader.h"
    "opengl_texture.h"
    "opengl_windows.h"
    "opengl_wrapper.h"

    "opengl_buffer.cpp"
    "opengl_context.cpp"
    "opengl_debug.cpp"
    "opengl_renderer.cpp"
    "opengl_shader.cpp"
    "opengl_shader_loader.cpp"
    "opengl_texture.cpp"
    "opengl_windows.cpp"
    "opengl_wrapper.cpp"
)

# PROJECT_SOURCE_DIR
target_include_directories(Files_Graphics_OpenGL
    PUBLIC
        ${PROJECT_SOURCE_DIR}/JadeFrame/src/
        ${PROJECT_SOURCE_DIR}/JadeFrame/src/extern/glad/include/

)
target_link_libraries(Files_Graphics_OpenGL
    PUBLIC
        ${imgui2}

)