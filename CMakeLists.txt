cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

#set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

# For it to compile you need at least clang++-13 or gcc-10
# std::format is as of right now only by msvc library supported
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-lc++abi -lpthread")

project(
    JadeFrame
    VERSION 0.0.1
    LANGUAGES C CXX
)

set(ENABLE_DEVELOPER_MODE
    TRUE
    CACHE BOOL "Enable 'developer mode'"
)
include(cmake/my_utils.cmake)





#Listing default values
if(NOT "")
    my_print_cmake_data()
    message("${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${GIT_SHORT_SHA}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
    message("PROJECT_SOURCE_DIR         -> ${PROJECT_SOURCE_DIR}")
    message("CMAKE_CURRENT_SOURCE_DIR   -> ${CMAKE_CURRENT_SOURCE_DIR}")
    message("CMAKE_CURRENT_BINARY_DIR   -> ${CMAKE_CURRENT_BINARY_DIR}")
    message("CMAKE_PROJECT_NAME         -> ${CMAKE_PROJECT_NAME}")
    message("CMAKE_PROJECT_VERSION      -> ${CMAKE_PROJECT_VERSION}")
    message("CMAKE_SYSTEM_NAME          -> ${CMAKE_SYSTEM_NAME}")
    message("CMAKE_SYSTEM_PROCESSOR     -> ${CMAKE_SYSTEM_PROCESSOR}")
    message("CMAKE_SYSTEM_VERSION       -> ${CMAKE_SYSTEM_VERSION}")
    message("CMAKE_DL_LIBS              -> ${CMAKE_DL_LIBS}")

    message("=========Default values=========") 
    message("CMAKE_CONFIGURATION_TYPES  -> ${CMAKE_CONFIGURATION_TYPES}")
    message("CMAKE_C_FLAGS              -> ${CMAKE_C_FLAGS}")
    message("CMAKE_C_FLAGS_DEBUG        -> ${CMAKE_C_FLAGS_DEBUG}")
    message("CMAKE_C_FLAGS_RELEASE      -> ${CMAKE_C_FLAGS_RELEASE}")
    message("CMAKE_CXX_FLAGS            -> ${CMAKE_CXX_FLAGS}")
    message("CMAKE_CXX_FLAGS_DEBUG      -> ${CMAKE_CXX_FLAGS_DEBUG}")
    message("CMAKE_CXX_FLAGS_RELEASE    -> ${CMAKE_CXX_FLAGS_RELEASE}")

    message("CMAKE_EXE_LINKER_FLAGS    -> ${CMAKE_EXE_LINKER_FLAGS}")
    message("CMAKE_MODULE_LINKER_FLAGS    -> ${CMAKE_MODULE_LINKER_FLAGS}")
    message("CMAKE_SHARED_LINKER_FLAGS    -> ${CMAKE_SHARED_LINKER_FLAGS}")
    message("CMAKE_STATIC_LINKER_FLAGS    -> ${CMAKE_STATIC_LINKER_FLAGS}")
    message("=========Default values=========")
endif()

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
     OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86"))
    message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()




################################################################################
# Global configuration types
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)

#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Release")

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
if (WIN32)
  message("INFO: OS: WIN32")
elseif (UNIX AND NOT APPLE)
    message("INFO: OS: LINUX")
else()
    message(FATAL_ERROR "INFO: OS: UNKNOWN")
endif()

message("INFO: Generator: ${CMAKE_GENERATOR}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message("INFO: Compiler: CLANG")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message("INFO: Compiler: GCC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    message("INFO: Compiler: INTEL")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("INFO: Compiler: MSVC")
else()
    message(FATAL_ERROR "INFO: Compiler: UNKNOWN")
endif()


message("INFO: DEFAULT_CXX_DEBUG_INFORMATION_FORMAT: ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}")
message("INFO: DEFAULT_CXX_EXCEPTION_HANDLING: ${DEFAULT_CXX_EXCEPTION_HANDLING}")
message("INFO: Architecture: ${CMAKE_VS_PLATFORM_NAME}")


include(cmake/Utils.cmake) # Common utils
include(cmake/GlobalSettingsInclude.cmake OPTIONAL) # Additional Global Settings(add specific info there)
set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Use solution folders feature

################################################################################
# Sub-projects
################################################################################

add_subdirectory(JadeFrame)
add_subdirectory(Application)