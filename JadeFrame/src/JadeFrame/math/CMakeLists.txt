

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
target_link_libraries(JF_MODULE_math
    PRIVATE
        JF_MODULE_prelude
)

# add_subdirectory(tests)