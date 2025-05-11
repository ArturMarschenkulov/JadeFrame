cmake_minimum_required(VERSION 3.25 FATAL_ERROR)





# Prevent in-source builds (good practice)
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds are not allowed. Please create a separate build directory and run CMake from there."
  )
endif()

#set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)


# NOTE(artur): 
# For it to compile you need at least clang++-13 or gcc-10
# std::format is as of right now only by msvc library supported

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0148 NEW)
set(CMAKE_POLICY_WARNING_CMP0048 FALSE)  # Suppress warnings about project() command
set(CMAKE_POLICY_WARNING_CMP0148 FALSE)  # Suppress warnings about Python find modules

add_definitions(-Wno-dev)  # Suppress developer warnings

set(JADEFRAME_USED_COMPILER
    "gcc"
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (JADEFRAME_USED_COMPILER STREQUAL "clang")
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "-lc++abi -lpthread")
elseif (JADEFRAME_USED_COMPILER STREQUAL "gcc")
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
else()
    message(FATAL_ERROR "Compiler not supported")
endif()


# set(BUILD_SHARED_LIBS OFF)

project(
    JadeFrame
    VERSION 0.0.1
    DESCRIPTION "JadeFrame: A cross platform graphics framework"
    LANGUAGES C CXX
)

include(cmake/my_utils.cmake)
my_print_platform_data()


set(ENABLE_DEVELOPER_MODE
    TRUE
    CACHE BOOL "Enable 'developer mode'"
)

my_print_generator_data()
my_print_compiler_data()
my_print_cmake_data()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type" FORCE)
  # Set the possible values for GUI/ccmake
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;MinSizeRel;RelWithDebInfo")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Use solution folders feature

enable_testing()
################################################################################
# Sub-projects
################################################################################
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(JadeFrame)

option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    # add_subdirectory(examples)
    # add_subdirectory(examples/texture_quad)
    # add_subdirectory(examples/moving_primitive)
    # add_subdirectory(examples/rotating_primitive)
    add_subdirectory(examples/hello_triangle)
endif()
#       _           _      ______                        
#      | |         | |    |  ____|                       
#      | | __ _  __| | ___| |__ _ __ __ _ _ __ ___   ___ 
#  _   | |/ _` |/ _` |/ _ \  __| '__/ _` | '_ ` _ \ / _ \
# | |__| | (_| | (_| |  __/ |  | | | (_| | | | | | |  __/
#  \____/ \__,_|\__,_|\___|_|  |_|  \__,_|_| |_| |_|\___|
message(STATUS "=======================================================")
message(STATUS "                                                       ")
message(STATUS "      _           _      ______                        ")
message(STATUS "     | |         | |    |  ____|                       ")
message(STATUS "     | | __ _  __| | ___| |__ _ __ __ _ _ __ ___   ___ ")
message(STATUS " _   | |/ _` |/ _` |/ _ \\  __| '__/ _` | '_ ` _ \\ / _ \\")
message(STATUS "| |__| | (_| | (_| |  __/ |  | | | (_| | | | | | |  __/")
message(STATUS " \\____/ \\__,_|\\__,_|\\___|_|  |_|  \\__,_|_| |_| |_|\\___|")
message(STATUS "=======================================================")