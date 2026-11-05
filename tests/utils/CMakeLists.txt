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
