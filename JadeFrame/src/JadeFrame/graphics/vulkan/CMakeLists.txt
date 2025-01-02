set(SOURCE_FILES
    "buffer.h"
    "command_buffer.h"
    "context.h"
    "debug.h"
    "descriptor_set.h"
    "logical_device.h"
    "physical_device.h"
    "pipeline.h"
    "renderer.h"
    "shader.h"
    "shared.h"
    "surface.h"
    "swapchain.h"
    "sync_object.h"
    "queue.h"
    "buffer.cpp"
    "command_buffer.cpp"
    "context.cpp"
    "debug.cpp"
    "descriptor_set.cpp"
    "logical_device.cpp"
    "physical_device.cpp"
    "pipeline.cpp"
    "renderer.cpp"
    "shader.cpp"
    "surface.cpp"
    "swapchain.cpp"
    "sync_object.cpp"
    "queue.cpp"
)
if(WIN32)
    list(APPEND SOURCE_FILES "platform/win32/surface.h" "platform/win32/surface.cpp")
elseif(LINUX)
    list(APPEND SOURCE_FILES "platform/x11/surface.h" "platform/x11/surface.cpp")
else()
    message(ERROR "Unsupported platform")
endif()

add_library(gfx-vulkan ${SOURCE_FILES})

target_link_libraries(gfx-vulkan
    PRIVATE
        Vulkan::Vulkan
        ${Vulkan_LIBRARIES}
        JadeFrame
)