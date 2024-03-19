# utils file for projects came from visual studio solution with cmake-converter.

################################################################################
# Use props file for a target and configs
#     use_props(<target> <configs...> <props_file>)
# Inside <props_file> there are following variables:
#     PROPS_TARGET   - <target>
#     PROPS_CONFIG   - One of <configs...>
#     PROPS_CONFIG_U - Uppercase PROPS_CONFIG
# Input:
#     target      - Target to apply props file
#     configs     - Build configurations to apply props file
#     props_file  - CMake script
################################################################################
macro(use_props TARGET CONFIGS PROPS_FILE)
    set(PROPS_TARGET "${TARGET}")
    foreach(PROPS_CONFIG ${CONFIGS})
        string(TOUPPER "${PROPS_CONFIG}" PROPS_CONFIG_U)

        get_filename_component(ABSOLUTE_PROPS_FILE "${PROPS_FILE}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
        if(EXISTS "${ABSOLUTE_PROPS_FILE}")
            include("${ABSOLUTE_PROPS_FILE}")
        else()
            message(WARNING "Corresponding cmake file from props \"${ABSOLUTE_PROPS_FILE}\" doesn't exist")
        endif()
    endforeach()
endmacro()

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
