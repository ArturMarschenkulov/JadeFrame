include_guard(GLOBAL)

option(
    JADEFRAME_ENABLE_SANITIZERS
    "Build JadeFrame targets with sanitizer instrumentation"
    OFF
)
set(
    JADEFRAME_SANITIZERS
    "address,undefined"
    CACHE STRING
    "Comma-separated sanitizer list for Clang/GCC builds"
)
option(
    JADEFRAME_SANITIZER_HALT_ON_ERROR
    "Stop on the first sanitizer finding instead of recovering"
    ON
)
option(
    JADEFRAME_SANITIZER_USE_AFTER_SCOPE
    "Enable AddressSanitizer use-after-scope checking when available"
    ON
)

function(jadeframe_configure_sanitizers)
    if(NOT JADEFRAME_ENABLE_SANITIZERS)
        return()
    endif()

    if(TARGET JadeFrame_sanitizers)
        return()
    endif()

    add_library(JadeFrame_sanitizers INTERFACE)
    add_library(JadeFrame::sanitizers ALIAS JadeFrame_sanitizers)

    string(REPLACE "," ";" sanitizer_list "${JADEFRAME_SANITIZERS}")
    string(REPLACE " " "" sanitizer_list "${sanitizer_list}")
    list(REMOVE_ITEM sanitizer_list "")

    if(NOT sanitizer_list)
        message(FATAL_ERROR "JADEFRAME_ENABLE_SANITIZERS is ON, but JADEFRAME_SANITIZERS is empty")
    endif()

    if(MSVC)
        if(NOT "address" IN_LIST sanitizer_list)
            message(FATAL_ERROR "MSVC sanitizer builds currently require JADEFRAME_SANITIZERS to include 'address'")
        endif()

        if("undefined" IN_LIST sanitizer_list)
            message(WARNING "MSVC does not support UndefinedBehaviorSanitizer; enabling AddressSanitizer only")
        endif()

        target_compile_options(JadeFrame_sanitizers INTERFACE
            /fsanitize=address
            /Zi
        )
        target_link_options(JadeFrame_sanitizers INTERFACE
            /fsanitize=address
        )
        message(STATUS "JadeFrame sanitizers enabled: address")
        return()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "JADEFRAME_ENABLE_SANITIZERS requires Clang, GCC, or MSVC")
    endif()

    list(JOIN sanitizer_list "," sanitizer_arg)

    set(sanitizer_compile_options
        "-fsanitize=${sanitizer_arg}"
        -fno-omit-frame-pointer
    )

    if(JADEFRAME_SANITIZER_HALT_ON_ERROR)
        list(APPEND sanitizer_compile_options
            -fno-sanitize-recover=all
        )
    endif()

    if(JADEFRAME_SANITIZER_USE_AFTER_SCOPE AND "address" IN_LIST sanitizer_list)
        list(APPEND sanitizer_compile_options
            -fsanitize-address-use-after-scope
        )
    endif()

    target_compile_options(JadeFrame_sanitizers INTERFACE
        ${sanitizer_compile_options}
    )
    target_link_options(JadeFrame_sanitizers INTERFACE
        "-fsanitize=${sanitizer_arg}"
    )

    message(STATUS "JadeFrame sanitizers enabled: ${sanitizer_arg}")
endfunction()

function(jadeframe_enable_sanitizers target)
    if(NOT JADEFRAME_ENABLE_SANITIZERS)
        return()
    endif()

    if(NOT TARGET ${target})
        message(FATAL_ERROR "Cannot enable sanitizers for unknown target '${target}'")
    endif()

    target_link_libraries(${target} PRIVATE JadeFrame::sanitizers)
endfunction()

function(jadeframe_enable_sanitizers_public target)
    if(NOT JADEFRAME_ENABLE_SANITIZERS)
        return()
    endif()

    if(NOT TARGET ${target})
        message(FATAL_ERROR "Cannot enable sanitizers for unknown target '${target}'")
    endif()

    target_link_libraries(${target} PUBLIC JadeFrame::sanitizers)
endfunction()
