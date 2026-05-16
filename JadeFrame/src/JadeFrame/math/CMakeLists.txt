

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


add_library(JF_MODULE_math STATIC ${Files})
jadeframe_enable_sanitizers(JF_MODULE_math)
target_link_libraries(JF_MODULE_math
    PRIVATE
        JF_MODULE_prelude
)

# Math tests are wired through jadeframe_add_project_test(), but they are not
# part of the default test suite yet.
# add_subdirectory(tests)
