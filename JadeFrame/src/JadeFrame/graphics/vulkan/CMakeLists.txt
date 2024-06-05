set(SOURCE_FILES
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
)
if(WIN32)
    list(APPEND SOURCE_FILES "platform/win32/surface.h" "platform/win32/surface.cpp")
elseif(LINUX)
    list(APPEND SOURCE_FILES "platform/x11/surface.h" "platform/x11/surface.cpp")
else()
    message(ERROR "Unsupported platform")
endif()

add_library(gfx-vulkan ${SOURCE_FILES})

target_precompile_headers(gfx-vulkan PRIVATE pch.h)

target_link_libraries(gfx-vulkan
    PUBLIC
        Vulkan::Vulkan
        ${Vulkan_LIBRARIES}
        JadeFrame
)