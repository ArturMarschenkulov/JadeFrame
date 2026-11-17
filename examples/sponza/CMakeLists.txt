cmake_minimum_required(VERSION 3.10)

project(Example_sponza CXX)

add_executable(${PROJECT_NAME} main.cpp)
jadeframe_enable_warnings(${PROJECT_NAME})
jadeframe_enable_sanitizers(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
    PUBLIC JadeFrame
    PRIVATE jadeframe_logger jadeframe_backend_opengl
)
