jadeframe_add_project_test(test_asset_loader
    SOURCES
        test_asset_loader.cpp
    LIBRARIES
        jadeframe_assets
)

target_compile_definitions(test_asset_loader PRIVATE
    JADEFRAME_ASSET_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
