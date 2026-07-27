#pragma once
#include <cassert>

namespace JadeFrame {

// Keep assertion handling independent from the logging implementation. This
// header is included by public value types, so pulling spdlog into it would
// make the logger a transitive dependency of every JadeFrame consumer.
auto report_assert_failure(
    const char* expression,
    const char* message,
    const char* file,
    int         line
) noexcept -> void;

#define JF_ASSERT(expr, msg)                                                             \
    do {                                                                                 \
        const bool jf_assert_result = static_cast<bool>(expr);                           \
        if (!jf_assert_result) {                                                         \
            ::JadeFrame::report_assert_failure(#expr, (msg), __FILE__, __LINE__);        \
            assert(jf_assert_result);                                                    \
        }                                                                                \
    } while (0)

// template<typename T>
// constexpr void JF_ASSERT(T expr, const char* msg) {
//     if (expr) {
//     } else {
//         Logger::err("{}", msg);
//         assert(false);
//     }
// }

#define JF_UNIMPLEMENTED(text) JF_ASSERT(false, "unimplemented: " text)
#define JF_UNREACHABLE(text)   JF_ASSERT(false, "unreachable: " text)
#define JF_PANIC(text)         JF_ASSERT(false, text)

} // namespace JadeFrame
