#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/utils/assert.h"

#include <utility>

#include <concepts>
#include <type_traits>

namespace JadeFrame {


template<typename T, typename E>
class Result {
public:
    using ErrorType = E;
    using ValueType = T;

    Result(const ValueType& res)
        : m_has_result(true) {}
    Result(ValueType&& res)
        : m_has_result(true) {}
    Result(const ErrorType& err)
        : m_has_result(false) {}
    Result(ErrorType&& err)
        : m_has_result(false) {}

    bool m_has_result = false;
    union {
        alignas(T) u8 m_ok[sizeof(T)];
        alignas(E) u8 m_err[sizeof(E)];
    }
};

} // namespace JadeFrame