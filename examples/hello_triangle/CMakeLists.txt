cmake_minimum_required (VERSION 3.6)

project(Example_hello_triangle CXX)


set(Source_Files
    main.cpp
)

add_executable(${PROJECT_NAME} ${Source_Files})

add_dependencies(${PROJECT_NAME}
    JadeFrame
)
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