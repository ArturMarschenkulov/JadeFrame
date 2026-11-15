jadeframe_add_project_test(test_camera
    SOURCES
        test_camera.cpp
    LIBRARIES
        jadeframe_core
)

jadeframe_add_project_test(test_reflection
    SOURCES
        test_reflection.cpp
    LIBRARIES
        jadeframe_core
)

# Shader lookup and compilation failures. Needs no window or graphics API.
jadeframe_add_project_test(test_shader_errors
    SOURCES
        test_shader_errors.cpp
    LIBRARIES
        jadeframe_core
)

jadeframe_add_project_test(test_mesh
    SOURCES
        test_mesh.cpp
    LIBRARIES
        jadeframe_core
)

# GPU-free checks of the renderer contract (error reporting, image output and the pure
# parts of the Vulkan viewport and readback translation).
set(JADEFRAME_RENDERER_CONTRACT_LIBRARIES jadeframe_graphics)
if(JADEFRAME_ENABLE_VULKAN)
    # The Vulkan headers pull in the logger, whose formatting is partly inline.
    list(APPEND JADEFRAME_RENDERER_CONTRACT_LIBRARIES
        jadeframe_backend_vulkan
        jadeframe_logger
    )
endif()
jadeframe_add_project_test(test_renderer_contract
    SOURCES
        test_renderer_contract.cpp
    LIBRARIES
        ${JADEFRAME_RENDERER_CONTRACT_LIBRARIES}
)

# The same contract scenario against every compiled-in backend. Opens a window, so it
# skips itself where no display or no usable graphics API is available.
jadeframe_add_project_test(test_renderer_backends
    SOURCES
        test_renderer_backends.cpp
    LIBRARIES
        JadeFrame
)
set_tests_properties(test_renderer_backends PROPERTIES LABELS "jadeframe;project;gpu")

# Initialization failures and their cleanup: backend selection, the render system with
# a fake renderer and application startup. Needs no display and no GPU.
jadeframe_add_project_test(test_initialization
    SOURCES
        test_initialization.cpp
    LIBRARIES
        JadeFrame
)

# Successful initialization and shutdown of every compiled-in backend, through the
# render system and through the application. Opens windows, so it skips itself where no
# display or no usable graphics API is available.
jadeframe_add_project_test(test_initialization_backends
    SOURCES
        test_initialization_backends.cpp
    LIBRARIES
        JadeFrame
)
set_tests_properties(test_initialization_backends PROPERTIES
    LABELS "jadeframe;project;gpu"
)

foreach(renderer_test
    test_renderer_contract
    test_renderer_backends
    test_initialization
    test_initialization_backends
)
    target_compile_definitions(${renderer_test} PRIVATE
        JADEFRAME_ENABLE_OPENGL=$<BOOL:${JADEFRAME_ENABLE_OPENGL}>
        JADEFRAME_ENABLE_VULKAN=$<BOOL:${JADEFRAME_ENABLE_VULKAN}>
    )
endforeach()
