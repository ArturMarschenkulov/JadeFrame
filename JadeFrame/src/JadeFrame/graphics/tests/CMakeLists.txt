jadeframe_add_project_test(test_camera
    SOURCES
        test_camera.cpp
    LIBRARIES
        JF_MODULE_math
)

jadeframe_add_project_test(test_reflection
    SOURCES
        test_reflection.cpp
    LIBRARIES
        JF_MODULE_graphics
)
