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
