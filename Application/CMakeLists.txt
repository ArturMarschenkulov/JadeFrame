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
    "${CMAKE_CURRENT_SOURCE_DIR}/../JadeFrame/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../JadeFrame/src/extern"
    "${CMAKE_CURRENT_SOURCE_DIR}/../JadeFrame/src/extern/glad/include"

    
    ${CMAKE_CURRENT_SOURCE_DIR}/../JadeFrame/src/extern/spdlog/include
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../VulkanSDK/1.2.176.1/Include"
)

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
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
            /Od;
            /Oi;
            /Gy
        >
        /permissive-;
        /std:c++latest;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /INCREMENTAL
        >
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
        >
        /DEBUG;
        /SUBSYSTEM:CONSOLE;
        /LTCG:incremental
    )
endif()

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
    PRIVATE 
        ${ADDITIONAL_LIBRARY_DEPENDENCIES}
)

# NOTE: CMAKE_VS_PLATFORM_NAME was removed. Change this later.
target_link_directories(${PROJECT_NAME} 
    PRIVATE
        ${CMAKE_SOURCE_DIR}/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/
)

