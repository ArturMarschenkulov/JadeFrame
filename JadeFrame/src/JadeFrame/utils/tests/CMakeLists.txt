
enable_testing()

add_executable(test_option test_option.cpp)
target_link_libraries(test_option PUBLIC JadeFrame JF_MODULE_utils GTest::gtest_main)
add_test(NAME test_option COMMAND test_option)

add_executable(test_result test_result.cpp)
target_link_libraries(test_result PUBLIC JadeFrame JF_MODULE_utils GTest::gtest_main)
add_test(NAME test_result COMMAND test_result)


# include(GoogleTest)
# gtest_discover_tests(test0)