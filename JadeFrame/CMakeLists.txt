set(PROJECT_NAME JadeFrame)

# Add Vulkan
find_package(Vulkan REQUIRED)

add_subdirectory(src/extern/VulkanMemoryAllocator/)

# Add GLFW
add_subdirectory(src/extern/glfw/)


# Add ImGUI
set(imgui_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/extern/imgui/
)

include_directories(${imgui_DIR} 
    ${imgui_DIR}/backends
)


if(WIN32)
    set(imgui_platform_specific_sources
        ${imgui_DIR}/backends/imgui_impl_win32.h
        ${imgui_DIR}/backends/imgui_impl_dx11.h
        ${imgui_DIR}/backends/imgui_impl_win32.cpp
        ${imgui_DIR}/backends/imgui_impl_dx11.cpp
    )
elseif(LINUX)
    set(imgui_platform_specific_sources
        ${imgui_DIR}/backends/imgui_impl_x11.h
        ${imgui_DIR}/backends/imgui_impl_x11.h
    )
endif()
add_library(imgui
    STATIC
        ${imgui_DIR}/imgui_demo.cpp
        ${imgui_DIR}/imgui_draw.cpp
        ${imgui_DIR}/imgui_tables.cpp
        ${imgui_DIR}/imgui_widgets.cpp
        ${imgui_DIR}/imgui.cpp

        ${imgui_DIR}/backends/imgui_impl_opengl3.h
        ${imgui_DIR}/backends/imgui_impl_opengl3.cpp

        ${imgui_DIR}/backends/imgui_impl_glfw.h
        ${imgui_DIR}/backends/imgui_impl_glfw.cpp


        ${imgui_platform_specific_sources}
)

################################################################################
# Source groups
################################################################################

# if(WIN32)
#     add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#     add_definitions(-DNOMINMAX)
# endif()
add_subdirectory(src/extern/stb/)
add_subdirectory(src/extern/glad/)

add_subdirectory(src/extern/spdlog/)

# TODO: shaderc and SPIRV-Cross take a long time to compile. Find ways to cache their compilation.

# if(
#     NOT EXISTS "${CMAKE_SOURCE_DIR}/JadeFrame/src/lib/libshaderc.a" AND 
#     NOT EXISTS "${CMAKE_SOURCE_DIR}/JadeFrame/src/lib/libshaderc_util.a" AND
#     NOT EXISTS "${CMAKE_SOURCE_DIR}/JadeFrame/src/lib/libspirv-cross-core.a" AND
#     NOT EXISTS "${CMAKE_SOURCE_DIR}/JadeFrame/src/lib/libspirv-cross-glsl.a" AND
#     NOT EXISTS "${CMAKE_SOURCE_DIR}/JadeFrame/src/lib/libspirv-cross-hlsl.a" AND
#     NOT EXISTS "${CMAKE_SOURCE_DIR}/JadeFrame/src/lib/libspirv-cross-msl.a"
# )
message("Did not found prebuild shaderc and SPIRV-Cross libraries. Building them now. This will take a while.")
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)


add_subdirectory(src/extern/shaderc/ EXCLUDE_FROM_ALL)

# TODO: as of right now we do not need metal and hlsl support.
# set(SPIRV_CROSS_CLI OFF)
# set(SPIRV_CROSS_ENABLE_HLSL OFF)
# set(SPIRV_CROSS_ENABLE_MSL OFF)
# set(SPIRV_CROSS_ENABLE_TESTS OFF)
add_subdirectory(src/extern/SPIRV-Cross/ EXCLUDE_FROM_ALL)

# endif()



add_subdirectory(src/JadeFrame/graphics/opengl)
add_subdirectory(src/JadeFrame/graphics/vulkan)

set(Files_Base
    "src/JadeFrame.h"
    "src/pch.h"
    "src/JadeFrame/base_app.h"
    "src/JadeFrame/core.h"
    "src/JadeFrame/macros.h"
    "src/JadeFrame/types.h"
    "src/JadeFrame/prelude.h"
    "src/JadeFrame/gui.h"

    "src/JadeFrame.cpp"
    "src/pch.cpp"
    "src/JadeFrame/base_app.cpp"
    "src/JadeFrame/core.cpp"
    "src/JadeFrame/gui.cpp"
)
set(Files_Graphics_DirectX
    "src/JadeFrame/graphics/d3d11/d3d11_renderer.h"
    "src/JadeFrame/graphics/d3d12/d3d12_renderer.h"
)
set(Files_Graphics
    "src/JadeFrame/graphics/camera.h"

    "src/JadeFrame/graphics/graphics_shared.h"
    "src/JadeFrame/graphics/mesh.h"
    "src/JadeFrame/graphics/camera.cpp"
    "src/JadeFrame/graphics/graphics_shared.cpp"
    "src/JadeFrame/graphics/mesh.cpp"
    "src/JadeFrame/graphics/shader_loader.h"
    "src/JadeFrame/graphics/shader_loader.cpp"
    "src/JadeFrame/graphics/reflect.h"
    "src/JadeFrame/graphics/reflect.cpp"
    "src/JadeFrame/graphics/graphics_language.h"
    "src/JadeFrame/graphics/graphics_language.cpp"


    "src/JadeFrame/graphics/software/software_renderer.h"
    "src/JadeFrame/graphics/software/software_renderer.cpp"
)

set(Files_Datatypes
    "src/JadeFrame/datatypes/cowdata.h"
    "src/JadeFrame/datatypes/string.h"
)

set(Files_Math
    "src/JadeFrame/math/i128.h"
    "src/JadeFrame/math/mat.h"
    "src/JadeFrame/math/mat_4.h"
    "src/JadeFrame/math/math.h"
    "src/JadeFrame/math/vec.h"

    "src/JadeFrame/math/math.cpp"
    "src/JadeFrame/math/vec.cpp"
)

set(Files_Utils
    "src/JadeFrame/utils/assert.h"
    "src/JadeFrame/utils/dynamic_array.h"
    "src/JadeFrame/utils/logger.h"
    "src/JadeFrame/utils/utils.h"

    "src/JadeFrame/utils/logger.cpp"
    "src/JadeFrame/utils/utils.cpp"
)
IF (WIN32)
    add_subdirectory(src/JadeFrame/platform/windows/)
ELSEIF(LINUX)
    add_subdirectory(src/JadeFrame/platform/linux/)
ENDIF()
    set(Files_Platform
        "src/JadeFrame/platform/platform_shared.h"
        "src/JadeFrame/platform/platform_shared.cpp"
        "src/JadeFrame/platform/window.h"
        "src/JadeFrame/platform/window.cpp"
        "src/JadeFrame/platform/window_event.h"
    )

set(ALL_FILES
    ${Files_Base}
    ${Files_Graphics}
    ${Files_Platform}
    ${Files_Math}
    ${Files_Utils}
    ${Files_Datatypes}
    "src/JadeFrame/utils/option.h"
    "src/JadeFrame/utils/result.h"
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>
)

#########################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/JadeFrame/src
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern
    ${Vulkan_INCLUDE_DIRS}
)
include_directories(
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glfw/include # TODO: This should be only temporary!!!
)
################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
        _CONSOLE
        UNICODE
        _UNICODE
    )
endif()

################################################################################
# Compile options
################################################################################
set(COMPILE_OPTIONS 
    # -O3
)
if(MSVC)
    list(APPEND ${COMPILE_OPTIONS}
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        # /permissive-;
        /std:c++latest;
        /sdl;
        /W3;
        -W4;
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    
    list(APPEND ${COMPILE_OPTIONS}
        -g
        -O0
    )
    list(APPEND ${COMPILE_OPTIONS}
        -Wall
        -Wextra
        # -Weverything
        -Wpedantic
        -Werror
        # -Werror
    )
endif()
target_compile_options(${PROJECT_NAME} PRIVATE
    ${COMPILE_OPTIONS}
)

################################################################################
# Link options
################################################################################
if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:CONSOLE
    )
endif()


################################################################################
# Dependencies
################################################################################
IF(WIN32)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        platform-win32
        opengl32
        ${Vulkan_LIBRARIES}
    )
ELSEIF(UNIX AND NOT APPLE)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        platform-linux
        pthread
        X11
        GL
        libvulkan.so
    )
ENDIF()
target_link_libraries(
    ${PROJECT_NAME} 
    PUBLIC 
    
    PUBLIC spdlog::spdlog
        spdlog
        ${ADDITIONAL_LIBRARY_DEPENDENCIES}
        ${CMAKE_DL_LIBS}
        Vulkan::Vulkan
        ${Vulkan_LIBRARIES}
        glad
        glfw
        imgui
        stb
        shaderc
        spirv-cross-hlsl
        spirv-cross-glsl
        spirv-cross-msl
        # ${glad}
        # ${imgui}
        # ${stb}
        gfx-opengl
        gfx-vulkan
        GPUOpen::VulkanMemoryAllocator
)

target_link_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/
    ${CMAKE_SOURCE_DIR}/JadeFrame/
    # C:/VulkanSDK/1.3.224.1/Lib/ # TODO: make this less hard-coded
)

message("JadeFrame cmake: Finished")