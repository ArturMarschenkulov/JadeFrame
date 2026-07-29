# set(PROJECT_NAME JadeFrame)
if(NOT COMMAND jadeframe_enable_sanitizers)
    include("${CMAKE_CURRENT_LIST_DIR}/../cmake/JadeFrameSanitizers.cmake")
    jadeframe_configure_sanitizers()
endif()
if(NOT COMMAND jadeframe_enable_warnings)
    include("${CMAKE_CURRENT_LIST_DIR}/../cmake/JadeFrameWarnings.cmake")
endif()

option(JADEFRAME_ENABLE_OPENGL "Build the OpenGL renderer backend" ON)
option(JADEFRAME_ENABLE_VULKAN "Build the Vulkan renderer backend" ON)

if(JADEFRAME_ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan: ${Vulkan_FOUND}. Includes: ${Vulkan_INCLUDE_DIRS}")
endif()

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    message(STATUS "Found GTest: ${GTEST_FOUND}. Includes: ${GTEST_INCLUDE_DIRS}")
    jadeframe_mark_vendor_target_as_system(GTest::gtest)
    jadeframe_mark_vendor_target_as_system(GTest::gtest_main)

    function(jadeframe_add_project_test target)
        cmake_parse_arguments(JF_TEST "" "" "SOURCES;LIBRARIES" ${ARGN})
        if(NOT JF_TEST_SOURCES)
            message(FATAL_ERROR "jadeframe_add_project_test(${target}) requires SOURCES")
        endif()

        add_executable(${target} ${JF_TEST_SOURCES})
        jadeframe_enable_warnings(${target})
        target_link_libraries(${target}
            PRIVATE
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

if(JADEFRAME_ENABLE_VULKAN)
    add_subdirectory(src/extern/VulkanMemoryAllocator/)
    target_include_directories(VulkanMemoryAllocator SYSTEM INTERFACE src/extern/)
    jadeframe_mark_vendor_target_as_system(VulkanMemoryAllocator)
    jadeframe_mark_vendor_target_as_system(GPUOpen::VulkanMemoryAllocator)
    jadeframe_mark_vendor_target_as_system(Vulkan::Vulkan)
endif()
# Add GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(src/extern/glfw/)
jadeframe_mark_vendor_target_as_system(glfw)


# Assimp
# Prefer a package supplied by the toolchain. If it is not installed, use the
# repository submodule. Both paths expose the same modern CMake target.
find_package(assimp CONFIG QUIET)

if(NOT TARGET assimp::assimp)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/extern/assimp/CMakeLists.txt")
        message(FATAL_ERROR
            "Assimp was not found. Install Assimp with a CMake package or "
            "initialize the assimp submodule."
        )
    endif()

    message(STATUS "Assimp package not found; using the vendored source.")

    # Configure only the vendored copy. Installed packages own their
    # dependency and build options.
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    # Keep the vendored build self-contained. Assimp's pkg-config probe for
    # minizip can report success even when the distribution installs
    # unzip.h below an extra include directory (for example
    # /usr/include/minizip). Assimp then includes <unzip.h> and the build
    # fails. Its bundled zlib/minizip sources provide the include layout that
    # this vendored Assimp revision expects.
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

    # Do not let a previous configure run leave Assimp believing that an
    # external minizip installation is usable. These are cache variables in
    # Assimp's pkg-config helper, so simply changing ASSIMP_BUILD_ZLIB is not
    # enough when reusing an existing build directory.
    set(UNZIP_FOUND FALSE CACHE INTERNAL "Use Assimp's bundled minizip" FORCE)
    unset(UNZIP_INCLUDE_DIRS CACHE)
    unset(UNZIP_LIBRARIES CACHE)
    unset(UNZIP_LINK_LIBRARIES CACHE)

    # These formats currently fail to build with the vendored Assimp subtree.
    set(ASSIMP_BUILD_3MF_EXPORTER OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_OPENGEX_IMPORTER OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_MS3D_IMPORTER OFF CACHE BOOL "" FORCE)

    add_subdirectory(src/extern/assimp/ EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET assimp::assimp)
    message(FATAL_ERROR "Assimp does not provide the assimp::assimp target")
endif()
jadeframe_mark_vendor_target_as_system(assimp::assimp)



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
    SYSTEM PUBLIC
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
jadeframe_mark_vendor_target_as_system(stb)
jadeframe_mark_vendor_target_as_system(glad)

add_subdirectory(src/extern/spdlog/)
jadeframe_mark_vendor_target_as_system(spdlog)
jadeframe_mark_vendor_target_as_system(spdlog::spdlog)
jadeframe_mark_vendor_target_as_system(spdlog_header_only)
jadeframe_mark_vendor_target_as_system(spdlog::spdlog_header_only)

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
    jadeframe_mark_vendor_target_as_system(PkgConfig::SYSTEM_SHADERC)
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
jadeframe_mark_vendor_target_as_system(shaderc)

# TODO: as of right now we do not need metal and hlsl support.
# set(SPIRV_CROSS_CLI OFF)
# set(SPIRV_CROSS_ENABLE_HLSL OFF)
# set(SPIRV_CROSS_ENABLE_MSL OFF)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
add_subdirectory(src/extern/SPIRV-Cross/ EXCLUDE_FROM_ALL)
jadeframe_mark_vendor_target_as_system(spirv-cross-core)
jadeframe_mark_vendor_target_as_system(spirv-cross-glsl)
jadeframe_mark_vendor_target_as_system(spirv-cross-hlsl)
jadeframe_mark_vendor_target_as_system(spirv-cross-msl)
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


add_library(jadeframe_logger
    STATIC
        "src/JadeFrame/utils/logger.h"
        "src/JadeFrame/utils/logger.cpp"
)
jadeframe_enable_sanitizers(jadeframe_logger)
jadeframe_enable_warnings(jadeframe_logger)
target_link_libraries(jadeframe_logger
    # jadeframe_logger is a static implementation library. Its fmt/spdlog
    # objects must remain on the final link line, but the public JadeFrame
    # target links the logger only through PRIVATE implementation targets.
    PUBLIC
        spdlog::spdlog
        spdlog
)

add_library(jadeframe_prelude INTERFACE)
target_include_directories(jadeframe_prelude
    INTERFACE
        "${CMAKE_CURRENT_LIST_DIR}/src/"
)

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
add_subdirectory(src/JadeFrame/graphics)
add_subdirectory(src/JadeFrame/assets)

set(ALL_FILES
    ${Files_Base}
    ${Files_Datatypes}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
jadeframe_enable_sanitizers(${PROJECT_NAME})
jadeframe_enable_warnings(${PROJECT_NAME})
# target_link_libraries(${PROJECT_NAME} PRIVATE pthread)


# The precompiled header is an implementation detail.  Propagating it to
# consumers forces every source file that links JadeFrame to include the
# framework's private include set and makes the consumer build depend on the
# exact PCH contents.
target_precompile_headers(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>
)

#########################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)
# The current public headers live below src/. Keep the vendor and Vulkan
# include roots private so they do not become consumer requirements.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/extern
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
        jadeframe_assets
        jadeframe_graphics
        jadeframe_platform
        jadeframe_logger
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/extern/
    ${CMAKE_SOURCE_DIR}/JadeFrame/src/
    ${CMAKE_SOURCE_DIR}/JadeFrame/
)

message("JadeFrame cmake: Finished")
