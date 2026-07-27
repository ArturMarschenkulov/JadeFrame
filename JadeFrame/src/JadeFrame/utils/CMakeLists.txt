set(Files
    "assert.h"
    "dynamic_array.h"
    "utils.h"

    "utils.cpp"

    "option.h"
    "result.h"

    # "box.h"
)

add_library(jadeframe_utils STATIC ${Files})
jadeframe_enable_sanitizers(jadeframe_utils)
target_link_libraries(jadeframe_utils
    PUBLIC
        jadeframe_prelude
    PRIVATE
        jadeframe_logger
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
