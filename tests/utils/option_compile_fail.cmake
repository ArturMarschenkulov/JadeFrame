# Compile each invalid public API use separately; success is a regression.
if(NOT DEFINED OPTION_INCLUDE_DIR OR NOT DEFINED OPTION_TEST_DIR OR NOT DEFINED OPTION_CXX)
    message(FATAL_ERROR "Missing Option compile-failure test configuration")
endif()
file(MAKE_DIRECTORY "${OPTION_TEST_DIR}")
set(cases
    "Option<int> x = 42$<SEMICOLON>"
    "Option<int> x = None$<SEMICOLON>"
    "Option<int> x$<SEMICOLON> x = 42$<SEMICOLON>"
    "Option<int> x$<SEMICOLON> if (x) {}"
    "Option<int&> x$<SEMICOLON> if (x) {}"
    "Option<void> x$<SEMICOLON> if (x) {}"
    "Option<int&&> x$<SEMICOLON>"
    "Option<int[2]> x$<SEMICOLON>"
    "Option<NoneTag> x$<SEMICOLON>"
    "Option<const std::string&> x{std::string{}}$<SEMICOLON>"
    "auto x = some<const std::string&>(std::string{})$<SEMICOLON>"
    "Option<const std::string&> x$<SEMICOLON> x.rebind(std::string{})$<SEMICOLON>"
    "Option<const std::string&> x{\"temporary\"}$<SEMICOLON>"
    "Option<int&> x$<SEMICOLON> int n = 0$<SEMICOLON> x = n$<SEMICOLON>"
    "auto a = some<std::unique_ptr<int>>()$<SEMICOLON> auto b = a$<SEMICOLON>"
    "struct X { X() = default$<SEMICOLON> X(X&&) = delete$<SEMICOLON> }$<SEMICOLON> Option<X> a$<SEMICOLON> auto b = std::move(a)$<SEMICOLON>"
    "struct X { X() = default$<SEMICOLON> X(const X&) = default$<SEMICOLON> X& operator=(const X&) = delete$<SEMICOLON> }$<SEMICOLON> Option<X> a, b$<SEMICOLON> a = b$<SEMICOLON>"
    "Option<void> x$<SEMICOLON> *x$<SEMICOLON>"
    "constexpr int n = none<int>().value()$<SEMICOLON>"
)
set(index 0)
# First require a valid control to compile so setup errors cannot count as passes.
list(PREPEND cases "auto x = some<int>(42)$<SEMICOLON> return x.value() == 42 ? 0 : 1$<SEMICOLON>")
foreach(body IN LISTS cases)
    string(REPLACE "$<SEMICOLON>" ";" body "${body}")
    set(source "${OPTION_TEST_DIR}/case_${index}.cpp")
    file(WRITE "${source}" "#include <JadeFrame/utils/option.h>\n#include <string>\n#include <memory>\nusing namespace JadeFrame;\nint main() { ${body} }\n")
    if(OPTION_CXX_ID STREQUAL "MSVC")
        execute_process(COMMAND "${OPTION_CXX}" /nologo /std:c++20 "/I${OPTION_INCLUDE_DIR}" /c "${source}" "/Fo${OPTION_TEST_DIR}/case.obj"
            RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
    else()
        execute_process(COMMAND "${OPTION_CXX}" -std=c++20 "-I${OPTION_INCLUDE_DIR}" -c "${source}" -o "${OPTION_TEST_DIR}/case.o"
            RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
    endif()
    if(index EQUAL 0)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Valid control did not compile: ${output}\n${errors}")
        endif()
    elseif(result EQUAL 0)
        message(FATAL_ERROR "Invalid Option operation compiled: ${body}")
    endif()
    math(EXPR index "${index} + 1")
endforeach()
message(STATUS "Option negative compilation tests passed")
