cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
include(cmake/my_utils.cmake)
my_print_platform_data()


#set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

# For it to compile you need at least clang++-13 or gcc-10
# std::format is as of right now only by msvc library supported
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-lc++abi -lpthread")

project(
    JadeFrame
    VERSION 0.0.1
    DESCRIPTION "JadeFrame: A cross platform graphics framework"
    LANGUAGES C CXX
)

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
    # "RelWithDebInfo"
    # "MinSizeRel"
    CACHE STRING "" FORCE
)

set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

# #set(CMAKE_CONFIGURATION_TYPES "Debug")
# set(CMAKE_CONFIGURATION_TYPES "Release")

################################################################################
# Global compiler options
################################################################################

if(MSVC)
    # remove default flags provided with CMake for MSVC
    message("Overriding the default flags for MSVC")
    set(CMAKE_C_FLAGS "") # /DWIN32 /D_WINDOWS
    set(CMAKE_C_FLAGS_DEBUG "") # /Zi /Ob0 /Od /RTC1
    set(CMAKE_C_FLAGS_RELEASE "") # /O2 /Ob2 /DNDEBUG
    set(CMAKE_CXX_FLAGS "") # /DWIN32 /D_WINDOWS /GR /EHsc /stdlib=libc++
    set(CMAKE_CXX_FLAGS_DEBUG "") # /Zi /Ob0 /Od /RTC1
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3 /DNDEBUG") # /O2 /Ob2 /DNDEBUG

    message("Now the flags are as follows:")
    message("   CMAKE_C_FLAGS           -> ${CMAKE_C_FLAGS}")
    message("   CMAKE_C_FLAGS_DEBUG     -> ${CMAKE_C_FLAGS_DEBUG}")
    message("   CMAKE_C_FLAGS_RELEASE   -> ${CMAKE_C_FLAGS_RELEASE}")
    message("   CMAKE_CXX_FLAGS         -> ${CMAKE_CXX_FLAGS}")
    message("   CMAKE_CXX_FLAGS_DEBUG   -> ${CMAKE_CXX_FLAGS_DEBUG}")
    message("   CMAKE_CXX_FLAGS_RELEASE -> ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

################################################################################
# Global linker options
################################################################################
if(MSVC)
    # remove default flags provided with CMake for MSVC
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_MODULE_LINKER_FLAGS "")
    set(CMAKE_SHARED_LINKER_FLAGS "")
    set(CMAKE_STATIC_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS}")
endif()




################################################################################
# Print out information
################################################################################
# my_print_platform_data()
# my_print_generator_data()
# my_print_compiler_data()


message("INFO: DEFAULT_CXX_DEBUG_INFORMATION_FORMAT: ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}")
message("INFO: DEFAULT_CXX_EXCEPTION_HANDLING: ${DEFAULT_CXX_EXCEPTION_HANDLING}")


include(cmake/Utils.cmake) # Common utils
include(cmake/GlobalSettingsInclude.cmake OPTIONAL) # Additional Global Settings(add specific info there)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Use solution folders feature

################################################################################
# Sub-projects
################################################################################
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(JadeFrame)
add_subdirectory(Application)