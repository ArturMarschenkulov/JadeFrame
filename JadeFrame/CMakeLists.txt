# set(PROJECT_NAME JadeFrame)
if(NOT COMMAND jadeframe_enable_sanitizers)
    include("${CMAKE_CURRENT_LIST_DIR}/../cmake/JadeFrameSanitizers.cmake")
    jadeframe_configure_sanitizers()
endif()

function(disable_warnings_for_target target)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES COMPILE_OPTIONS "")
    endif()
endfunction()

# Add Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan: ${Vulkan_FOUND}. Includes: ${Vulkan_INCLUDE_DIRS}")

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    message(STATUS "Found GTest: ${GTEST_FOUND}. Includes: ${GTEST_INCLUDE_DIRS}")

    function(jadeframe_add_project_test target)
        cmake_parse_arguments(JF_TEST "" "" "SOURCES;LIBRARIES" ${ARGN})
        if(NOT JF_TEST_SOURCES)
            message(FATAL_ERROR "jadeframe_add_project_test(${target}) requires SOURCES")
        endif()

        add_executable(${target} ${JF_TEST_SOURCES})
        target_link_libraries(${target}
            PRIVATE
                JadeFrame
                ${JF_TEST_LIBRARIES}
                GTest::gtest_main
        )
        add_test(NAME ${target} COMMAND ${target})
        set_tests_properties(${target} PROPERTIES LABELS "jadeframe;project")
    endfunction()
endif()

function(jadeframe_collect_tests_under directory out_var)
    get_property(tests DIRECTORY "${directory}" PROPERTY TESTS)
    set(collected_tests ${${out_var}} ${tests})

    get_property(subdirectories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
    foreach(subdirectory IN LISTS subdirectories)
        jadeframe_collect_tests_under("${subdirectory}" collected_tests)
    endforeach()

    set(${out_var} ${collected_tests} PARENT_SCOPE)
endfunction()

set(JADEFRAME_BUILD_TESTING ${BUILD_TESTING})
set(JADEFRAME_EXTERNAL_TESTS)
set(BUILD_TESTING OFF)

# add_subdirectory(src/extern/)

add_subdirectory(src/extern/VulkanMemoryAllocator/)
target_include_directories(VulkanMemoryAllocator INTERFACE src/extern/)
# Add GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(src/extern/glfw/)


# Assimp
option(USE_PRECOMPILED_ASSIMP "Use precompiled Assimp library" ON)
set(ASSIMP_PRECOMPILED_PATH ${CMAKE_SOURCE_DIR}/libs/assimp)
set(ASSIMP_PRECOMPILED_LIB_PATH ${ASSIMP_PRECOMPILED_PATH}/lib)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

# These formats currently fail to build with the vendored Assimp subtree.
set(ASSIMP_BUILD_3MF_EXPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OPENGEX_IMPORTER OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_MS3D_IMPORTER OFF CACHE BOOL "" FORCE)

if(EXISTS ${ASSIMP_PRECOMPILED_LIB_PATH}/libassimp.a OR EXISTS ${ASSIMP_PRECOMPILED_LIB_PATH}/libassimpd.a)
    set(ASSIMP_FOUND TRUE)
else()
    set(ASSIMP_FOUND FALSE)
endif()
# For now no static import, it is always compiled by source
set(ASSIMP_FOUND FALSE)
if (ASSIMP_FOUND)
    message("Precompiled Assimp library found. Using precompiled Assimp library")
    message("ASSIMP_PRECOMPILED_LIB_PATH: ${ASSIMP_PRECOMPILED_LIB_PATH}")
    add_library(assimp STATIC IMPORTED)
    set_property(TARGET assimp PROPERTY IMPORTED_LOCATION ${ASSIMP_PRECOMPILED_LIB_PATH}/libassimp.lib)
    set_property(TARGET assimp PROPERTY IMPORTED_LOCATION_DEBUG ${ASSIMP_PRECOMPILED_LIB_PATH}/libassimpd.lib)
    target_link_libraries(assimp INTERFACE ${ASSIMP_PRECOMPILED_LIB_PATH}/libassimp.a)
    target_include_directories(assimp INTERFACE ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/assimp/include/)
else()
    message("Precompiled Assimp library not found. Building Assimp from source.")
    add_subdirectory(src/extern/assimp/)
    set_target_properties(assimp PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${ASSIMP_PRECOMPILED_LIB_PATH}"
        LIBRARY_OUTPUT_DIRECTORY "${ASSIMP_PRECOMPILED_LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY "${ASSIMP_PRECOMPILED_LIB_PATH}"
    )
    target_compile_definitions(assimp PRIVATE ASSIMP_USE_HUNTER)
endif()



# Add ImGUI
set(imgui_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/extern/imgui/
)

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
        ${imgui_DIR}/backends/imgui_impl_x11.cpp
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
target_include_directories(imgui
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/extern"
        "${imgui_DIR}"
        "${imgui_DIR}/backends"
)
target_link_libraries(imgui PRIVATE glfw)

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
option(USE_SYSTEM_SHADERC "Use the system shaderc library instead of the vendored source tree" ON)

if(USE_SYSTEM_SHADERC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SYSTEM_SHADERC REQUIRED IMPORTED_TARGET shaderc)

    add_library(shaderc INTERFACE IMPORTED)
    target_link_libraries(shaderc INTERFACE PkgConfig::SYSTEM_SHADERC)
    message(STATUS "Using system shaderc ${SYSTEM_SHADERC_VERSION}")
else()
    message("Did not find prebuilt shaderc. Building shaderc from source. This will take a while.")
    set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
    set(SHADERC_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
    set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
    set(SHADERC_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
    set(SHADERC_ENABLE_COPYRIGHT_CHECK OFF CACHE BOOL "" FORCE)
    set(SHADERC_ENABLE_WERROR_COMPILE OFF CACHE BOOL "" FORCE)
    set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
    set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
    set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
    add_subdirectory(src/extern/shaderc/ EXCLUDE_FROM_ALL)
    jadeframe_collect_tests_under(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/extern/shaderc"
        JADEFRAME_EXTERNAL_TESTS
    )
endif()

# TODO: as of right now we do not need metal and hlsl support.
# set(SPIRV_CROSS_CLI OFF)
# set(SPIRV_CROSS_ENABLE_HLSL OFF)
# set(SPIRV_CROSS_ENABLE_MSL OFF)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
add_subdirectory(src/extern/SPIRV-Cross/ EXCLUDE_FROM_ALL)
jadeframe_collect_tests_under(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/extern/SPIRV-Cross"
    JADEFRAME_EXTERNAL_TESTS
)

# endif()

list(REMOVE_DUPLICATES JADEFRAME_EXTERNAL_TESTS)
set(JADEFRAME_CTEST_CUSTOM "# Generated by JadeFrame/CMakeLists.txt.\n")
string(APPEND JADEFRAME_CTEST_CUSTOM "set(CTEST_CUSTOM_TESTS_IGNORE\n")
foreach(test_name IN LISTS JADEFRAME_EXTERNAL_TESTS)
    string(APPEND JADEFRAME_CTEST_CUSTOM "    ${test_name}\n")
endforeach()
string(APPEND JADEFRAME_CTEST_CUSTOM ")\n")
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
    CONTENT "${JADEFRAME_CTEST_CUSTOM}"
)

set(BUILD_TESTING ${JADEFRAME_BUILD_TESTING})



add_subdirectory(src/JadeFrame/graphics)

set(Files_Base
    "src/JadeFrame.h"
    "src/pch.h"
    "src/JadeFrame/base_app.h"
    "src/JadeFrame/core.h"
    # "src/JadeFrame/macros.h"
    # "src/JadeFrame/types.h"
    # "src/JadeFrame/prelude.h"
    "src/JadeFrame/gui.h"

    "src/JadeFrame.cpp"
    "src/pch.cpp"
    "src/JadeFrame/base_app.cpp"
    "src/JadeFrame/core.cpp"
    "src/JadeFrame/gui.cpp"
)

set(Files_Datatypes
    "src/JadeFrame/datatypes/cowdata.h"
    "src/JadeFrame/datatypes/string.h"
)

add_subdirectory(src/JadeFrame/math)
add_subdirectory(src/JadeFrame/utils)
add_subdirectory(src/JadeFrame/platform)

set(ALL_FILES
    ${Files_Base}
    ${Files_Datatypes}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
jadeframe_enable_sanitizers_public(${PROJECT_NAME})
# target_link_libraries(${PROJECT_NAME} PRIVATE pthread)


target_precompile_headers(${PROJECT_NAME} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>
)

#########################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/JadeFrame/src
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern
    ${Vulkan_INCLUDE_DIRS}
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
    list(APPEND COMPILE_OPTIONS
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
    
    list(APPEND COMPILE_OPTIONS
        -g
        -O0
    )
    list(APPEND COMPILE_OPTIONS
        -Wall
        -Wextra
        # -Weverything
        -Wpedantic
        # -Werror
        # -Werror
    )

    # list(APPEND COMPILE_OPTIONS
    #     -fsanitize=address,undefined
    #     -fno-omit-frame-pointer
    #     -fno-sanitize=signed-integer-overflow
    #     -fno-sanitize-recover=all
    # )
endif()
target_compile_options(${PROJECT_NAME} PRIVATE
    ${COMPILE_OPTIONS}
)

add_library(JF_MODULE_logger
    STATIC
        "src/JadeFrame/utils/logger.h"
        "src/JadeFrame/utils/logger.cpp"
)
jadeframe_enable_sanitizers(JF_MODULE_logger)
target_link_libraries(JF_MODULE_logger 
    PUBLIC 
        spdlog::spdlog
        spdlog
)
# disable_warnings_for_target(spdlog)
# target_compile_definitions(spdlog INTERFACE SPDLOG_USE_STD_FORMAT=1)
# target_compile_definitions(spdlog INTERFACE FMT_ENFORCE_COMPILE_STRING=0)
add_library(JF_MODULE_prelude INTERFACE)
target_include_directories(JF_MODULE_prelude 
    INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/src/"
)
target_link_libraries(JF_MODULE_prelude 
    INTERFACE 
        JF_MODULE_logger
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

target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE
        ${CMAKE_DL_LIBS}
        imgui
    PUBLIC
        JF_MODULE_prelude
        JF_MODULE_utils
        JF_MODULE_math
        JF_MODULE_platform
        JF_MODULE_graphics
        # -fsanitize=address,undefined
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/
    ${CMAKE_SOURCE_DIR}/JadeFrame/
)

message("JadeFrame cmake: Finished")
