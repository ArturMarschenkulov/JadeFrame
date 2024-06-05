# utils file for projects came from visual studio solution with cmake-converter.

################################################################################
# Add compile options to source file
#     source_file_compile_options(<source_file> [compile_options...])
# Input:
#     source_file     - Source file
#     compile_options - Options to add to COMPILE_FLAGS property
################################################################################
function(source_file_compile_options SOURCE_FILE)
    if("${ARGC}" LESS_EQUAL "1")
        return()
    endif()

    get_source_file_property(COMPILE_OPTIONS "${SOURCE_FILE}" COMPILE_OPTIONS)

    if(COMPILE_OPTIONS)
        list(APPEND COMPILE_OPTIONS ${ARGN})
    else()
        set(COMPILE_OPTIONS "${ARGN}")
    endif()

    set_source_files_properties("${SOURCE_FILE}" PROPERTIES COMPILE_OPTIONS "${COMPILE_OPTIONS}")
endfunction()

################################################################################
# Default properties of visual studio projects
################################################################################
set(DEFAULT_CXX_PROPS "${CMAKE_CURRENT_LIST_DIR}/DefaultCXX.cmake")
set(DEFAULT_Fortran_PROPS "${CMAKE_CURRENT_LIST_DIR}/DefaultFortran.cmake")
