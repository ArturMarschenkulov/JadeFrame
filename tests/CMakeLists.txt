# First-party tests live outside the production source tree. The JadeFrame
# library targets and the jadeframe_add_project_test() helper are defined by
# the time this directory is entered.
add_subdirectory(math)
add_subdirectory(utils)
add_subdirectory(graphics)

get_property(JADEFRAME_PROJECT_TEST_TARGETS GLOBAL PROPERTY JADEFRAME_PROJECT_TEST_TARGETS)
if(JADEFRAME_PROJECT_TEST_TARGETS)
    add_custom_target(jadeframe_project_tests)
    add_dependencies(jadeframe_project_tests ${JADEFRAME_PROJECT_TEST_TARGETS})
endif()
