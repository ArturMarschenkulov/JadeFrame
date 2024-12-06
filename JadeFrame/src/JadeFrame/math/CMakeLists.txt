

set(Files
    "i128.h"
    "mat.h"
    "mat_4.h"
    "math.h"
    "vec.h"

    "math.cpp"
    "vec.cpp"
)


add_library(JF_MODULE_math STATIC ${Files})
target_link_libraries(JF_MODULE_math
    PUBLIC
        JadeFrame
)

# add_subdirectory(tests)