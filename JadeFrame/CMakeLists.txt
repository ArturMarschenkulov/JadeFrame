set(PROJECT_NAME JadeFrame)

################################################################################
# Source groups
################################################################################
set(Files_Extern_IMGUI
        "src/extern/imgui/imconfig.h"
        "src/extern/imgui/imgui.h"
        "src/extern/imgui/imgui_impl_dx11.h"
        "src/extern/imgui/imgui_impl_opengl3.h"
        "src/extern/imgui/imgui_impl_win32.h"
        "src/extern/imgui/imgui_internal.h"
        "src/extern/imgui/imstb_rectpack.h"
        "src/extern/imgui/imstb_textedit.h"
        "src/extern/imgui/imstb_truetype.h"

        "src/extern/imgui/imgui.cpp"
        "src/extern/imgui/imgui_demo.cpp"
        "src/extern/imgui/imgui_draw.cpp"
        "src/extern/imgui/imgui_impl_dx11.cpp"
        "src/extern/imgui/imgui_impl_opengl3.cpp"
        "src/extern/imgui/imgui_impl_win32.cpp"
        "src/extern/imgui/imgui_widgets.cpp"
        )
set(Files_Extern_STB
        "src/extern/stb/stb_image.h"
        "src/extern/stb/stb_image_resize.h"
        "src/extern/stb/stb_image_write.h"
        "src/extern/stb/stb_perlin.h"
        "src/extern/stb/stb_rect_pack.h"
        "src/extern/stb/stb_truetype.h"
        "src/extern/stb/stb_vorbis.h"
        )
file(GLOB Files_Extern_spdlog
    "src/extern/spdlog/*"
)
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
        "src/JadeFrame/defines.h"
        "src/JadeFrame/gui.h"

        "src/JadeFrame.cpp"
        "src/pch.cpp"
        "src/JadeFrame/base_app.cpp"
        "src/JadeFrame/core.cpp"
        "src/JadeFrame/gui.cpp"
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

        "src/JadeFrame/graphics/d3d11/d3d11_renderer.h"
        "src/JadeFrame/graphics/d3d12/d3d12_renderer.h"
        "src/JadeFrame/graphics/software/software_renderer.h"

        "src/JadeFrame/graphics/opengl/opengl_buffer.h"
        "src/JadeFrame/graphics/opengl/opengl_context.h"
        "src/JadeFrame/graphics/opengl/opengl_debug.h"
        "src/JadeFrame/graphics/opengl/opengl_renderer.h"
        "src/JadeFrame/graphics/opengl/opengl_shader.h"
        "src/JadeFrame/graphics/opengl/opengl_shader_loader.h"
        "src/JadeFrame/graphics/opengl/opengl_texture.h"
        "src/JadeFrame/graphics/opengl/opengl_windows.h"
        "src/JadeFrame/graphics/opengl/opengl_wrapper.h"
        "src/JadeFrame/graphics/opengl/opengl_buffer.cpp"
        "src/JadeFrame/graphics/opengl/opengl_context.cpp"
        "src/JadeFrame/graphics/opengl/opengl_debug.cpp"
        "src/JadeFrame/graphics/opengl/opengl_renderer.cpp"
        "src/JadeFrame/graphics/opengl/opengl_shader.cpp"
        "src/JadeFrame/graphics/opengl/opengl_shader_loader.cpp"
        "src/JadeFrame/graphics/opengl/opengl_texture.cpp"
        "src/JadeFrame/graphics/opengl/opengl_windows.cpp"
        "src/JadeFrame/graphics/opengl/opengl_wrapper.cpp"

        "src/JadeFrame/graphics/vulkan/vulkan_buffer.h"
        "src/JadeFrame/graphics/vulkan/vulkan_command_buffers.h"
        "src/JadeFrame/graphics/vulkan/vulkan_context.h"
        "src/JadeFrame/graphics/vulkan/vulkan_debug.h"
        "src/JadeFrame/graphics/vulkan/vulkan_descriptor_set.h"
        "src/JadeFrame/graphics/vulkan/vulkan_logical_device.h"
        "src/JadeFrame/graphics/vulkan/vulkan_physical_device.h"
        "src/JadeFrame/graphics/vulkan/vulkan_pipeline.h"
        "src/JadeFrame/graphics/vulkan/vulkan_renderer.h"
        "src/JadeFrame/graphics/vulkan/vulkan_shader.h"
        "src/JadeFrame/graphics/vulkan/vulkan_shared.h"
        "src/JadeFrame/graphics/vulkan/vulkan_surface.h"
        "src/JadeFrame/graphics/vulkan/vulkan_swapchain.h"
        "src/JadeFrame/graphics/vulkan/vulkan_sync_object.h"
        "src/JadeFrame/graphics/vulkan/vulkan_buffer.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_command_buffers.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_context.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_debug.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_descriptor_set.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_logical_device.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_physical_device.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_pipeline.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_renderer.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_shader.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_surface.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_swapchain.cpp"
        "src/JadeFrame/graphics/vulkan/vulkan_sync_object.cpp"
        )
set(Files_Extern_OpenGL
        "src/extern/glad/glad.h"
        "src/extern/KHR/khrplatform.h"

        "src/extern/glad.c"
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

set(Files_Platform_Windows
        "src/JadeFrame/platform/windows/windows_input_manager.h"
        "src/JadeFrame/platform/windows/windows_message_map.h"
        "src/JadeFrame/platform/windows/windows_shared.h"
        "src/JadeFrame/platform/windows/windows_system_manager.h"
        "src/JadeFrame/platform/windows/windows_time_manager.h"
        "src/JadeFrame/platform/windows/windows_window.h"

        "src/JadeFrame/platform/windows/windows_input_manager.cpp"
        "src/JadeFrame/platform/windows/windows_message_map.cpp"
        "src/JadeFrame/platform/windows/windows_system_manager.cpp"
        "src/JadeFrame/platform/windows/windows_time_manager.cpp"
        "src/JadeFrame/platform/windows/windows_window.cpp"
        )

#[[source_group("Files"
        FILES

        ${Files_Base}
        ${Files_Graphics}
        ${Files_Platform_Windows}
        ${Files_Utils}
        ${Files_Datatypes}
        ${Files_Extern_OpenGL}

        ${Files_Extern_IMGUI}
        ${Files_Extern_STB}
        ${Files_Extern_shaderc}
        ${Files_Extern_SPIRV-Cross}
        )]]

set(ALL_FILES
    #${Header_Files}
    #${Source_Files}
        ${Files_Base}
        ${Files_Graphics}
        ${Files_Platform_Windows}
        ${Files_Math}
        ${Files_Utils}
        ${Files_Datatypes}
        ${Files_Extern_OpenGL}

        ${Files_Extern_IMGUI}
        ${Files_Extern_STB}
        ${Files_Extern_shaderc}
        ${Files_Extern_SPIRV-Cross}
        ${Files_Extern_spdlog}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE JadeFrame)

################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Release>:"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../VulkanSDK/1.2.176.1/Include"
        ">"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/extern;"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_include_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Release>:"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../../VulkanSDK/1.2.176.1/Include"
        ">"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/extern;"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CRT_SECURE_NO_WARNINGS;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "_CRT_SECURE_NO_WARNINGS;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++latest;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            -W4;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++latest;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            -W4;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/glad.c ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/imgui/imgui.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/imgui/imgui_demo.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/imgui/imgui_draw.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/imgui/imgui_impl_dx11.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/imgui/imgui_impl_opengl3.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/imgui/imgui_impl_win32.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/imgui/imgui_widgets.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cfg.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cpp.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross_c.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross_parsed_ir.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_cross_util.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_glsl.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_hlsl.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_msl.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_parser.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        string(CONCAT FILE_CL_OPTIONS
            "/Y-"
        )
    endif()
    source_file_compile_options(src/extern/SPIRV-Cross/spirv_reflect.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "shadercd;"
            "shaderc_combinedd;"
            "shaderc_sharedd;"
            "shaderc_utild"
        ">"
        "$<$<CONFIG:Release>:"
            "shaderc;"
            "shaderc_combined;"
            "shaderc_shared;"
            "shaderc_util"
        ">"
        "vulkan-1"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "shadercd;"
            "shaderc_combinedd;"
            "shaderc_sharedd;"
            "shaderc_utild"
        ">"
        "$<$<CONFIG:Release>:"
            "shaderc;"
            "shaderc_combined;"
            "shaderc_shared;"
            "shaderc_util"
        ">"
        "vulkan-1"
    )
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_SOURCE_DIR}/JadeFrame/src/extern;"
        "${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glfw32;"
        "${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glfw64;"
        "${CMAKE_SOURCE_DIR}/JadeFrame/src;"
        "C:/VulkanSDK/1.2.176.1/Lib"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_link_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_SOURCE_DIR}/JadeFrame/src/extern;"
        "${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glfw32;"
        "${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/glfw64;"
        "${CMAKE_SOURCE_DIR}/JadeFrame/src;"
        "C:/VulkanSDK/1.2.176.1/Lib"
    )
endif()
