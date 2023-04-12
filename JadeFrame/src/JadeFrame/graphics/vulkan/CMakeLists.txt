add_library(GraphicsAPI_Vulkan
    "vulkan_buffer.h"
    "vulkan_command_buffers.h"
    "vulkan_context.h"
    "vulkan_debug.h"
    "vulkan_descriptor_set.h"
    "vulkan_logical_device.h"
    "vulkan_physical_device.h"
    "vulkan_pipeline.h"
    "vulkan_renderer.h"
    "vulkan_shader.h"
    "vulkan_shared.h"
    "vulkan_surface.h"
    "vulkan_swapchain.h"
    "vulkan_sync_object.h"
    "vulkan_buffer.cpp"
    "vulkan_command_buffers.cpp"
    "vulkan_context.cpp"
    "vulkan_debug.cpp"
    "vulkan_descriptor_set.cpp"
    "vulkan_logical_device.cpp"
    "vulkan_physical_device.cpp"
    "vulkan_pipeline.cpp"
    "vulkan_renderer.cpp"
    "vulkan_shader.cpp"
    "vulkan_surface.cpp"
    "vulkan_swapchain.cpp"
    "vulkan_sync_object.cpp"

    "platform/win32/surface.h"
    "platform/win32/surface.cpp"
)

target_precompile_headers(GraphicsAPI_Vulkan PRIVATE pch.h)

# PROJECT_SOURCE_DIR
target_include_directories(GraphicsAPI_Vulkan
    PUBLIC
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
        ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/spdlog/include


)
target_link_libraries(GraphicsAPI_Vulkan
    PUBLIC
        ${stb}
        stb
        spdlog
)
target_link_options(GraphicsAPI_Vulkan
    PUBLIC
        /incremental
        # /NODEFAULTLIB:LIBCMT
        # /NODEFAULTLIB:LIBCMTD
        # /NODEFAULTLIB:MSVCRT
        # /NODEFAULTLIB:MSVCRTD
)

target_compile_definitions(GraphicsAPI_Vulkan PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _CRT_SECURE_NO_WARNINGS
    _CONSOLE
    UNICODE
    _UNICODE
)
if(MSVC)
    target_compile_options(GraphicsAPI_Vulkan PRIVATE
        /EHsc
        /Z7
        /Od
        /W4
        /MP
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

    target_compile_options(GraphicsAPI_Vulkan PRIVATE
        -Wall 
        -Wextra 
        -Wpedantic 
        # -Werror
    )
elseif(CMKAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(ERROR "GCC is not supported yet")
endif()