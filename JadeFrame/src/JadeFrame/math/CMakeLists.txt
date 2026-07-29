

set(Files
    "i128.h"
    "mat.h"
    "mat_4.h"
    # "mat_4.cpp"
    "math.h"
    "vec.h"

    "math.cpp"
    "vec.cpp"
)


add_library(jadeframe_math STATIC ${Files})
jadeframe_enable_sanitizers(jadeframe_math)
jadeframe_enable_warnings(jadeframe_math)
target_link_libraries(jadeframe_math
    PUBLIC
        jadeframe_utils
)

# Math tests are wired through jadeframe_add_project_test(), but they are not
# part of the default test suite yet.
# add_subdirectory(tests)
