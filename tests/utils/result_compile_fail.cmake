# Require a valid control first, then reject each invalid payload/API use.
if(NOT DEFINED RESULT_INCLUDE_DIR OR NOT DEFINED RESULT_TEST_DIR OR NOT DEFINED RESULT_CXX)
    message(FATAL_ERROR "Missing Result compile-failure test configuration")
endif()
file(MAKE_DIRECTORY "${RESULT_TEST_DIR}")
set(cases
    "auto x = ok<int, int>(42)$<SEMICOLON> return x.value() == 42 ? 0 : 1$<SEMICOLON>"
    "Result<int, int> x$<SEMICOLON>"
    "Result<void, void> x$<SEMICOLON>"
    "Result<int, int> x = 42$<SEMICOLON>"
    "Result<int, int> x{Ok, 1}$<SEMICOLON> if (x) {}"
    "Result<int&&, int> x{Ok, 1}$<SEMICOLON>"
    "Result<int, int&&> x{Err, 1}$<SEMICOLON>"
    "Result<int[2], int> x{Ok}$<SEMICOLON>"
    "Result<int, int[2]> x{Err}$<SEMICOLON>"
    "Result<NoneTag, int> x{Ok}$<SEMICOLON>"
    "Result<int, InPlaceTag> x{Err}$<SEMICOLON>"
    "Result<OkTag, int> x{Ok}$<SEMICOLON>"
    "Result<int, ErrTag> x{Err}$<SEMICOLON>"
    "Result<const std::string&, int> x{Ok, std::string{}}$<SEMICOLON>"
    "auto x = err<int, const std::string&>(std::string{})$<SEMICOLON>"
    "Result<const std::string&, int> x{Ok, \"temporary\"}$<SEMICOLON>"
    "Result<void, int> x{Ok}$<SEMICOLON> *x$<SEMICOLON>"
    "Result<int, void> x{Err, 1}$<SEMICOLON>"
    "Result<void, int> x{Ok, 1}$<SEMICOLON>"
    "Result<const int, int> a{Ok, 1}, b{Err, 2}$<SEMICOLON> a = b$<SEMICOLON>"
    "Result<int, const int> a{Ok, 1}, b{Err, 2}$<SEMICOLON> a = std::move(b)$<SEMICOLON>"
    "struct X { ~X() noexcept(false) {} }$<SEMICOLON> Result<X, int> x{Ok}$<SEMICOLON>"
    "struct X { ~X() noexcept(false) {} }$<SEMICOLON> Result<int, X> x{Err}$<SEMICOLON>"
    "Result<std::string, int> x{Err, 1}$<SEMICOLON> x.emplace_ok(\"throwing\")$<SEMICOLON>"
    "constexpr int n = Result<int, int>{Err, 1}.value()$<SEMICOLON>"
)
set(index 0)
foreach(body IN LISTS cases)
    string(REPLACE "$<SEMICOLON>" ";" body "${body}")
    set(source "${RESULT_TEST_DIR}/case_${index}.cpp")
    file(WRITE "${source}" "#include <JadeFrame/utils/result.h>\n#include <string>\n#include <utility>\nusing namespace JadeFrame;\nint main() { ${body} }\n")
    if(RESULT_CXX_ID STREQUAL "MSVC")
        execute_process(COMMAND "${RESULT_CXX}" /nologo /std:c++20 "/I${RESULT_INCLUDE_DIR}" /c "${source}" "/Fo${RESULT_TEST_DIR}/case.obj"
            RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
    else()
        execute_process(COMMAND "${RESULT_CXX}" -std=c++20 "-I${RESULT_INCLUDE_DIR}" -c "${source}" -o "${RESULT_TEST_DIR}/case.o"
            RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors)
    endif()
    if(index EQUAL 0)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "Valid Result control did not compile: ${output}\n${errors}")
        endif()
    elseif(result EQUAL 0)
        message(FATAL_ERROR "Invalid Result operation compiled: ${body}")
    endif()
    math(EXPR index "${index} + 1")
endforeach()
message(STATUS "Result negative compilation tests passed")
