cmake_minimum_required(VERSION 3.19 FATAL_ERROR)


#set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)


# For it to compile you need at least clang++-13 or gcc-10
# std::format is as of right now only by msvc library supported


set(JADEFRAME_USED_COMPILER
    "gcc"
)
if (JADEFRAME_USED_COMPILER STREQUAL "clang")
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "-lc++abi -lpthread")
elseif (JADEFRAME_USED_COMPILER STREQUAL "gcc")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_EXE_LINKER_FLAGS "-lpthread")
else()
    message(FATAL_ERROR "Compiler not supported")
endif()


# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(BUILD_SHARED_LIBS OFF)


project(
    JadeFrame
    VERSION 0.0.1
    DESCRIPTION "JadeFrame: A cross platform graphics framework"
    LANGUAGES C CXX
)


message(STATUS "=======================================================")
message(STATUS "                                                       ")
message(STATUS "      _           _      ______                        ")
message(STATUS "     | |         | |    |  ____|                       ")
message(STATUS "     | | __ _  __| | ___| |__ _ __ __ _ _ __ ___   ___ ")
message(STATUS " _   | |/ _` |/ _` |/ _ \\  __| '__/ _` | '_ ` _ \\ / _ \\")
message(STATUS "| |__| | (_| | (_| |  __/ |  | | | (_| | | | | | |  __/")
message(STATUS " \\____/ \\__,_|\\__,_|\\___|_|  |_|  \\__,_|_| |_| |_|\\___|")
message(STATUS "=======================================================")

#       _           _      ______                        
#      | |         | |    |  ____|                       
#      | | __ _  __| | ___| |__ _ __ __ _ _ __ ___   ___ 
#  _   | |/ _` |/ _` |/ _ \  __| '__/ _` | '_ ` _ \ / _ \
# | |__| | (_| | (_| |  __/ |  | | | (_| | | | | | |  __/
#  \____/ \__,_|\__,_|\___|_|  |_|  \__,_|_| |_| |_|\___|

# # ---- Include guards ----
# if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
#   message(
#     FATAL_ERROR
#       "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
#   )
# endif()

include(cmake/my_utils.cmake)
my_print_platform_data()



set(ENABLE_DEVELOPER_MODE
    TRUE
    CACHE BOOL "Enable 'developer mode'"
)

my_print_generator_data()
my_print_compiler_data()
my_print_cmake_data()


################################################################################
# Global configuration types
################################################################################

# Single-Configuration Generators: 
# - Makefiles
# - Ninja
# This uses CMAKE_BUILD_TYPE to set the configuration type.

# Multi-Configuration Generators:
# - Visual Studio
# - Xcode
# - Ninja Multi-Config
# This uses CMAKE_CONFIGURATION_TYPES to set the configuration types.

# Usually the default configuration types are:
# - Debug          : Development and debugging
# - Release        : Production use
# - RelWithDebInfo : Production debugging
# - MinSizeRel     : Embedded systems
# But this can be changed by the user.


set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "RelWithDebInfo"
    "MinSizeRel"
    CACHE STRING "" FORCE
)

set(CMAKE_BUILD_TYPE "Debug")


include(cmake/Utils.cmake) # Common utils
include(cmake/GlobalSettingsInclude.cmake OPTIONAL) # Additional Global Settings(add specific info there)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Use solution folders feature

################################################################################
# Sub-projects
################################################################################
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(JadeFrame)
add_subdirectory(Application)