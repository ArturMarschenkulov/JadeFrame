#pragma once
#include "logger.h"

namespace JadeFrame {

#define JF_ASSERT(expr, msg)                                                                                           \
    do {                                                                                                               \
        if (expr) {                                                                                                    \
        } else {                                                                                                       \
            /*std::string t = fmt::v8::format("{}: {}: {}", __FILE__,                                                  \
             * __LINE__, std::string(text));*/                                                                         \
                                                                                                                       \
            Logger::err("{}", msg);                                                                                          \
            assert(false);                                                                                             \
        }                                                                                                              \
    } while (0)

#define JF_UNIMPLEMENTED() JF_ASSERT(false, "unimplemented")
#define JF_UNREACHABLE()   JF_ASSERT(false, "unreachable")
#define JF_PANIC(text)     JF_ASSERT(false, text)

} // namespace JadeFrame