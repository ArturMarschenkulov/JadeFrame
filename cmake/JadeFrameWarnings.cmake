include_guard(GLOBAL)

option(
    JADEFRAME_ENABLE_WARNINGS
    "Enable the warning policy for JadeFrame-owned targets"
    ON
)
option(
    JADEFRAME_WARNINGS_AS_ERRORS
    "Treat warnings as errors for JadeFrame-owned targets"
    OFF
)

# Apply warnings only to targets owned by JadeFrame. Third-party projects are
# configured independently and must not inherit this policy through global
# compiler flags.
function(jadeframe_enable_warnings target)
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "Cannot enable warnings for unknown target '${target}'")
    endif()

    if(NOT JADEFRAME_ENABLE_WARNINGS)
        return()
    endif()

    if(MSVC)
        target_compile_options("${target}" PRIVATE /W4)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
           OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
           OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        target_compile_options("${target}" PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()

    # CMake 3.24+ translates this target property to /WX or -Werror as
    # appropriate for the active compiler. Keeping it target-local means
    # strict diagnostics cannot affect vendored targets.
    set_property(
        TARGET "${target}"
        PROPERTY COMPILE_WARNING_AS_ERROR "${JADEFRAME_WARNINGS_AS_ERRORS}"
    )
endfunction()

# Mark include directories exported by a third-party target as system include
# directories for its consumers. This suppresses diagnostics originating in
# vendor headers without suppressing warnings in JadeFrame source files.
function(jadeframe_mark_vendor_target_as_system target)
    if(NOT TARGET "${target}")
        return()
    endif()

    get_target_property(aliased_target "${target}" ALIASED_TARGET)
    if(aliased_target)
        set(target "${aliased_target}")
    endif()

    get_target_property(include_directories "${target}" INTERFACE_INCLUDE_DIRECTORIES)
    if(NOT include_directories OR include_directories MATCHES "-NOTFOUND$")
        return()
    endif()

    # INTERFACE_SYSTEM_INCLUDE_DIRECTORIES is consumed together with
    # INTERFACE_INCLUDE_DIRECTORIES; CMake emits matching entries with the
    # compiler's system-include switch (-isystem or its MSVC equivalent).
    set_property(
        TARGET "${target}"
        APPEND PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
        "${include_directories}"
    )
endfunction()
