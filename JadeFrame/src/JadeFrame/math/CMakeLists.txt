

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
target_link_libraries(JF_MODULE_math PRIVATE ${JF_MODULE_pch}) 
target_compile_definitions(JF_MODULE_math PRIVATE ${JF_MODULE_pch})

# target_precompile_headers(${Files_Math} INTERFACE
#     "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h>"
# )
# target_include_directories(
#     ${PROJECT_NAME}
#     PUBLIC
#     "${PROJECT_SOURCE_DIR}/src"
# )
