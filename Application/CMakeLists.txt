set(PROJECT_NAME Application)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "example_0.h"
    "example_empty.h"
    "example_texture_quad.h"
    "exmaple_rotating_primitive.h"
    "example_hello_triangle.h"
    "example_moving_primitive.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "main.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(
    ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})


################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
    "../JadeFrame/src"
    "../JadeFrame/src/extern"
    "../JadeFrame/src/extern/glad/include"

    
    # ${CMAKE_CURRENT_SOURCE_DIR}/../JadeFrame/src/extern/spdlog/include
)

################################################################################
# Compile definitions
################################################################################
        # _CONSOLE
        # UNICODE
        # _UNICODE
target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

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

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    JadeFrame
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    JadeFrame
)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        ${ADDITIONAL_LIBRARY_DEPENDENCIES}
)

