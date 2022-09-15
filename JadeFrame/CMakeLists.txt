set(PROJECT_NAME JadeFrame)

find_package(Vulkan REQUIRED)
set(imgui_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/extern/imgui/
)

include_directories(${imgui_DIR} 
    ${imgui_DIR}/backends
)

add_library(imgui
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

################################################################################
# Source groups
################################################################################
add_subdirectory(src/extern/stb/)
add_subdirectory(src/extern/glad/)
add_subdirectory(src/extern/spdlog/)
add_subdirectory(src/JadeFrame/graphics/opengl)
add_subdirectory(src/JadeFrame/graphics/vulkan)

set(Files_Extern_shaderc
    "src/extern/shaderc/env.h"
    "src/extern/shaderc/shaderc.h"
    "src/extern/shaderc/shaderc.hpp"
    "src/extern/shaderc/status.h"
    "src/extern/shaderc/visibility.h"
)
set(Files_Extern_SPIRV-Cross
    "src/extern/SPIRV-Cross/GLSL.std.450.h"
    "src/extern/SPIRV-Cross/spirv.h"
    "src/extern/SPIRV-Cross/spirv.hpp"
    "src/extern/SPIRV-Cross/spirv_cfg.hpp"
    "src/extern/SPIRV-Cross/spirv_common.hpp"
    "src/extern/SPIRV-Cross/spirv_cpp.hpp"
    "src/extern/SPIRV-Cross/spirv_cross.hpp"
    "src/extern/SPIRV-Cross/spirv_cross_c.h"
    "src/extern/SPIRV-Cross/spirv_cross_containers.hpp"
    "src/extern/SPIRV-Cross/spirv_cross_error_handling.hpp"
    "src/extern/SPIRV-Cross/spirv_cross_parsed_ir.hpp"
    "src/extern/SPIRV-Cross/spirv_cross_util.hpp"
    "src/extern/SPIRV-Cross/spirv_glsl.hpp"
    "src/extern/SPIRV-Cross/spirv_hlsl.hpp"
    "src/extern/SPIRV-Cross/spirv_msl.hpp"
    "src/extern/SPIRV-Cross/spirv_parser.hpp"
    "src/extern/SPIRV-Cross/spirv_reflect.hpp"

    "src/extern/SPIRV-Cross/spirv_cfg.cpp"
    "src/extern/SPIRV-Cross/spirv_cpp.cpp"
    "src/extern/SPIRV-Cross/spirv_cross.cpp"
    "src/extern/SPIRV-Cross/spirv_cross_c.cpp"
    "src/extern/SPIRV-Cross/spirv_cross_parsed_ir.cpp"
    "src/extern/SPIRV-Cross/spirv_cross_util.cpp"
    "src/extern/SPIRV-Cross/spirv_glsl.cpp"
    "src/extern/SPIRV-Cross/spirv_hlsl.cpp"
    "src/extern/SPIRV-Cross/spirv_msl.cpp"
    "src/extern/SPIRV-Cross/spirv_parser.cpp"
    "src/extern/SPIRV-Cross/spirv_reflect.cpp"
)

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
    set(Files_Platform
        "src/JadeFrame/platform/platform_shared.h"  
        "src/JadeFrame/platform/windows/windows_input_manager.h"
        "src/JadeFrame/platform/windows/windows_input_manager.cpp"

        "src/JadeFrame/platform/windows/windows_message_map.h"
        "src/JadeFrame/platform/windows/windows_message_map.cpp"
        
        "src/JadeFrame/platform/windows/windows_shared.h"
        "src/JadeFrame/platform/windows/windows_system_manager.h"
        "src/JadeFrame/platform/windows/windows_system_manager.cpp"

        "src/JadeFrame/platform/windows/windows_time_manager.h"
        "src/JadeFrame/platform/windows/windows_time_manager.cpp"

        "src/JadeFrame/platform/windows/windows_window.h"
        "src/JadeFrame/platform/windows/windows_window.cpp"
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
    # ${Files_Extern_shaderc}
    # ${Files_Extern_SPIRV-Cross}
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
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern
    ${CMAKE_SOURCE_DIR}/JadeFrame/src
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/JadeFrame
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glad/include
    
    ${Vulkan_INCLUDE_DIRS}
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
        winmm

        opengl32
        vulkan-1
        # "shadercd;"
        # "shaderc_combinedd;"
        # "shaderc_sharedd;"
        # "shaderc_utild;"
        
        
    )
ELSEIF(UNIX AND NOT APPLE)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "pthread;"
        "X11"
        "GL"
        # "shaderc;"
        # "shaderc_combined;"
        # "shaderc_shared;"
        # "shaderc_util;"

        "libvulkan.so"
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
        imgui
        stb
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