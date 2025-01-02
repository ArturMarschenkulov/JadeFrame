#pragma once
#include <cassert>
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {

#define JF_ASSERT(expr, msg)                                                             \
    do {                                                                                 \
        if (expr) {                                                                      \
        } else {                                                                         \
            /*std::string t = fmt::v8::format("{}: {}: {}", __FILE__,                    \
             * __LINE__, std::string(text));*/                                           \
                                                                                         \
            Logger::err("{}", msg);                                                      \
            assert(false);                                                               \
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