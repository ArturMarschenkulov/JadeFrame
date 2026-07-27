# Asset loading is deliberately separate from the general utility library.
# The public header remains under utils/ for source compatibility, but Assimp
# must not become a dependency of core utilities or concrete renderers.
add_library(jadeframe_assets STATIC
    "asset_loader.h"
    "asset_loader.cpp"
)
jadeframe_enable_sanitizers(jadeframe_assets)

target_link_libraries(jadeframe_assets
    PUBLIC
        jadeframe_core
    PRIVATE
        jadeframe_utils
        jadeframe_logger
        assimp
)
