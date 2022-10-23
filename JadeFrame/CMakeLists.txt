set(PROJECT_NAME JadeFrame)

find_package(Vulkan REQUIRED)
set(imgui_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/extern/imgui/
)

include_directories(${imgui_DIR} 
    ${imgui_DIR}/backends
)


add_subdirectory(src/extern/glfw/)
if(WIN32)
    set(imgui_platform_specific_sources
        ${imgui_DIR}/backends/imgui_impl_win32.h
        ${imgui_DIR}/backends/imgui_impl_dx11.h
        ${imgui_DIR}/backends/imgui_impl_win32.cpp
        ${imgui_DIR}/backends/imgui_impl_dx11.cpp
    )
elseif(UNIX AND NOT APPLE)
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

if(WIN32)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DNOMINMAX)
endif()
add_subdirectory(src/extern/stb/)
add_subdirectory(src/extern/glad/)

add_subdirectory(src/extern/spdlog/)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)
add_subdirectory(src/extern/shaderc/)
add_subdirectory(src/extern/SPIRV-Cross/)
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
    "src/JadeFrame/graphics/material_handle.h"
    "src/JadeFrame/graphics/mesh.h"
    "src/JadeFrame/graphics/camera.cpp"
    "src/JadeFrame/graphics/graphics_shared.cpp"
    "src/JadeFrame/graphics/material_handle.cpp"
    "src/JadeFrame/graphics/mesh.cpp"
    "src/JadeFrame/graphics/shader_loader.h"
    "src/JadeFrame/graphics/shader_loader.cpp"


    "src/JadeFrame/graphics/software/software_renderer.h"
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

    "src/JadeFrame/math/mat_4.cpp"
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
    set(Files_Platform
        "src/JadeFrame/platform/platform_shared.h"
    )
ELSEIF(UNIX)
    set(Files_Platform
        "src/JadeFrame/platform/platform_shared.h"
        "src/JadeFrame/platform/linux/linux_window.h" 
        "src/JadeFrame/platform/linux/linux_window.cpp" 
        "src/JadeFrame/platform/linux/linux_time_manager.h"
        "src/JadeFrame/platform/linux/linux_system_manager.h"
        "src/JadeFrame/platform/linux/linux_input_manager.h"
    )
  # set stuff for other systems
ENDIF()

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
# add_library(JF_MODULE_pch STATIC src/pch.cpp src/pch.h)
# target_include_directories(JF_MODULE_pch INTERFACE src/)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE JadeFrame)

################################################################################
# Output directory
################################################################################
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
#     OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
# )
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
# )
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/JadeFrame/src
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/JadeFrame
    

    
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
    _CRT_SECURE_NO_WARNINGS
    _CONSOLE
    UNICODE
    _UNICODE
)

################################################################################
# Compile and link options
################################################################################

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        # /permissive-;
        /std:c++latest;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        -W4;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )



    string(CONCAT FILE_CL_OPTIONS
        "/Y-"
    )

    source_file_compile_options(src/extern/imgui/imgui.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/imgui/imgui_demo.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/imgui/imgui_draw.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/imgui/imgui_impl_dx11.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/imgui/imgui_impl_opengl3.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/imgui/imgui_impl_win32.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/imgui/imgui_widgets.cpp ${FILE_CL_OPTIONS})

    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cfg.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cpp.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross_c.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross_parsed_ir.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross_util.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_glsl.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_hlsl.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_msl.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_parser.cpp ${FILE_CL_OPTIONS})
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_reflect.cpp ${FILE_CL_OPTIONS})
    
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /SUBSYSTEM:CONSOLE
    )

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall 
        -Wextra 
        -Wpedantic 
        # -Werror
    )
elseif(CMKAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall 
        -Wextra 
        -Wpedantic 
        # -Werror
    )
endif()

################################################################################
# Dependencies
################################################################################
IF(WIN32)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        Platform_Win32
        opengl32
        vulkan-1
    )
ELSEIF(UNIX AND NOT APPLE)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
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
        GraphicsAPI_OpenGL
        GraphicsAPI_Vulkan
)

target_link_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/
    ${CMAKE_SOURCE_DIR}/JadeFrame/
    # "C:/VulkanSDK/1.2.176.1/Lib/
    C:/VulkanSDK/1.3.224.1/Lib/ # TODO: make this less hard-coded
    # "~/proj_3rd/shaderc/build/"
)

message("JadeFrame cmake: Finished")