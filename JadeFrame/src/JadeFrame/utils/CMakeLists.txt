set(Files
    "assert.h"
    "dynamic_array.h"
    "logger.h"
    "utils.h"

    "logger.cpp"
    "utils.cpp"

    "asset_loader.h"
    "asset_loader.cpp"
    "option.h"
    "result.h"
)

add_library(JF_MODULE_utils STATIC ${Files})
target_link_libraries(JF_MODULE_utils
    PUBLIC
        JadeFrame
)
# add_subdirectory(tests)