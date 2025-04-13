
enable_testing()

add_executable(test_camera test_camera.cpp)
target_link_libraries(test_camera PRIVATE JadeFrame JF_MODULE_math GTest::gtest_main)
add_test(NAME test_camera COMMAND test_camera)

# include(GoogleTest)
# gtest_discover_tests(test0)