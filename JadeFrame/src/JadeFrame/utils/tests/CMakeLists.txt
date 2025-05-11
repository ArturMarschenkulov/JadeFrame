
enable_testing()

add_executable(test_option test_option.cpp)
target_link_libraries(test_option PRIVATE JadeFrame JF_MODULE_utils GTest::gtest_main)
add_test(NAME test_option COMMAND test_option)

add_executable(test_result test_result.cpp)
target_link_libraries(test_result PRIVATE JadeFrame JF_MODULE_utils GTest::gtest_main)
add_test(NAME test_result COMMAND test_result)


# add_executable(test_box test_box.cpp)
# target_link_libraries(test_result PRIVATE JadeFrame JF_MODULE_utils GTest::gtest_main)
# add_test(NAME test_box COMMAND test_box)

# include(GoogleTest)
# gtest_discover_tests(test0)