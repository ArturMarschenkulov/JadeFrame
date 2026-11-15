jadeframe_add_project_test(test_option
    SOURCES
        test_option.cpp
        test_option_edge.cpp
    LIBRARIES
        jadeframe_utils
)

jadeframe_add_project_test(test_result
    SOURCES
        test_result.cpp
        test_result_edge.cpp
    LIBRARIES
        jadeframe_utils
)

jadeframe_add_project_test(test_box
    SOURCES
        test_box.cpp
    LIBRARIES
        jadeframe_utils
)

add_test(NAME test_option_compile_fail
    COMMAND ${CMAKE_COMMAND}
        "-DOPTION_CXX=${CMAKE_CXX_COMPILER}"
        "-DOPTION_CXX_ID=${CMAKE_CXX_COMPILER_ID}"
        "-DOPTION_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/JadeFrame/src"
        "-DOPTION_TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/option_compile_fail"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/option_compile_fail.cmake"
)
set_tests_properties(test_option_compile_fail PROPERTIES LABELS "jadeframe;project")

add_test(NAME test_result_compile_fail
    COMMAND ${CMAKE_COMMAND}
        "-DRESULT_CXX=${CMAKE_CXX_COMPILER}"
        "-DRESULT_CXX_ID=${CMAKE_CXX_COMPILER_ID}"
        "-DRESULT_INCLUDE_DIR=${PROJECT_SOURCE_DIR}/JadeFrame/src"
        "-DRESULT_TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/result_compile_fail"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/result_compile_fail.cmake"
)
set_tests_properties(test_result_compile_fail PROPERTIES LABELS "jadeframe;project")

# Standalone header coverage does not need the framework or GTest libraries.
add_executable(test_result_no_exceptions test_result_no_exceptions.cpp)
target_include_directories(test_result_no_exceptions PRIVATE "${PROJECT_SOURCE_DIR}/JadeFrame/src")
jadeframe_enable_warnings(test_result_no_exceptions)
jadeframe_enable_sanitizers(test_result_no_exceptions)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(test_result_no_exceptions PRIVATE -fno-exceptions)
elseif(MSVC)
    target_compile_options(test_result_no_exceptions PRIVATE /EHs-c-)
endif()
set_property(GLOBAL APPEND PROPERTY JADEFRAME_PROJECT_TEST_TARGETS test_result_no_exceptions)
add_test(NAME test_result_no_exceptions COMMAND test_result_no_exceptions)
set_tests_properties(test_result_no_exceptions PROPERTIES LABELS "jadeframe;project")
