# Input state handling. Needs no window or display.
set(JADEFRAME_INPUT_TEST_LIBRARIES jadeframe_platform)
if(UNIX AND NOT APPLE)
    # For the X11 key symbol and mouse button translation.
    list(APPEND JADEFRAME_INPUT_TEST_LIBRARIES platform-linux X11)
endif()
jadeframe_add_project_test(test_input
    SOURCES
        test_input.cpp
    LIBRARIES
        ${JADEFRAME_INPUT_TEST_LIBRARIES}
)
