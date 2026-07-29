cmake_minimum_required (VERSION 3.10)

project(Example_0 CXX)


set(Source_Files
    main.cpp
)

add_executable(${PROJECT_NAME} ${Source_Files})
jadeframe_enable_warnings(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC JadeFrame
    PRIVATE jadeframe_logger
)

set(RESOURCE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource)
set(RESOURCE_DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/resource)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_SOURCE_DIR} ${RESOURCE_DEST_DIR}
    COMMENT "Copying resources to build directory"
)
