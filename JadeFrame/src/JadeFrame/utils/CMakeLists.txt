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

    # "box.h"
)

add_library(JF_MODULE_utils STATIC ${Files})
target_link_libraries(JF_MODULE_utils
    PRIVATE
        JF_MODULE_prelude
        JF_MODULE_graphics
        assimp
)
add_subdirectory(tests)