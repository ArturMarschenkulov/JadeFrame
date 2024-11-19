cmake_minimum_required (VERSION 3.10)

project(Example_moving_primitive CXX)


set(Source_Files
    main.cpp
)

add_executable(${PROJECT_NAME} ${Source_Files})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC JadeFrame
)

