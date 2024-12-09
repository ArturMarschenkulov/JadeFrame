
enable_testing()

add_executable(test0 test0.cpp)
target_link_libraries(test0 PRIVATE JadeFrame JF_MODULE_math GTest::gtest_main)
add_test(NAME test0 COMMAND test0)

add_executable(test_math test_math.cpp)
target_link_libraries(test_math PRIVATE JadeFrame JF_MODULE_math GTest::gtest_main)
add_test(NAME test_math COMMAND test_math)


# include(GoogleTest)
# gtest_discover_tests(test0)