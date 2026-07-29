cmake_minimum_required (VERSION 3.10)

project(Example_rotating_primitive CXX)


set(Source_Files
    main.cpp
)

add_executable(${PROJECT_NAME} ${Source_Files})
jadeframe_enable_warnings(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC JadeFrame
    PRIVATE jadeframe_logger
)
