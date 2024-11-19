cmake_minimum_required (VERSION 3.6)

project(Example_texture_quad CXX)


set(Source_Files
    main.cpp
)

add_executable(${PROJECT_NAME} ${Source_Files})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC JadeFrame
)

