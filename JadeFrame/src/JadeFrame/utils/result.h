#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/utils/assert.h"

#include <utility>

#include <concepts>
#include <type_traits>

namespace JadeFrame {

enum ResultType {
    OK,
    ERR
};
template<typename T, typename E>
class Result {


public:
    using ErrorType = E;
    using ValueType = T;

    enum ResultType {
        OK,
        ERR
    };

    // Default ctors
    // Copy ctors
    // Move ctors

    // Custom copy ctors
    Result(const ValueType& res)
        : m_has_result(true) {
        new (&m_ok) ValueType(std::forward<const ValueType&>(res));
    }
    Result(const ErrorType& err)
        : m_has_result(false) {
        new (&m_err) ErrorType(std::forward<const ErrorType&>(err));
    }
    // Custom move ctors
    Result(ValueType&& res)
        : m_has_result(true) {
        new (&m_ok) ValueType(std::forward<const ValueType&>(res));
    }
    Result(ErrorType&& err)
        : m_has_result(false) {
        new (&m_err) ErrorType(std::forward<const ErrorType&>(err));
    }

    // Dtor
    ~Result() {
        if (m_has_result) {
            this->unwrap().~ValueType();
            // m_ok.~ValueType();
        } else {
            this->unwrap_err().~ErrorType();
            // m_err.~ErrorType();
        }
    }


    constexpr auto unwrap() & -> T& {
        if (m_has_result) { return reinterpret_cast<T&>(m_ok); }
        JF_PANIC("called `Result::unwrap() & -> T&` on a `Err` value");
        std::terminate();
    }
    constexpr auto unwrap() const& -> const T& {
        if (m_has_result) { return reinterpret_cast<const T&>(m_ok); }
        JF_PANIC("called `Result::unwrap() const & -> const T&` on a `Err` value");
        std::terminate();
    }
    constexpr auto unwrap() && -> T { return this->release(); }

    constexpr auto unwrap_unchecked() & -> T& { return reinterpret_cast<T&>(m_ok); }
    constexpr auto unwrap_unchecked() const& -> const T& { return reinterpret_cast<const T&>(m_ok); }
    constexpr auto unwrap_unchecked() && -> T { return std::move(this->release_unchecked()); }

    constexpr auto unwrap_err() & -> E& {
        if (!m_has_result) { return reinterpret_cast<E&>(m_err); }
        JF_PANIC("called `Result::unwrap_err() & -> T&` on a `Err` value");
        std::terminate();
    }
    constexpr auto unwrap_err() const& -> const E& {
        if (!m_has_result) { return reinterpret_cast<const E&>(m_err); }
        JF_PANIC("called `Result::unwrap_err() const & -> const T&` on a `Err` value");
        std::terminate();
    }
    constexpr auto unwrap_err() && -> E { return this->release_err(); }

    constexpr auto unwrap_err_unchecked() & -> E& { return reinterpret_cast<E&>(m_err); }
    constexpr auto unwrap_err_unchecked() const& -> const E& { return reinterpret_cast<const E&>(m_err); }
    constexpr auto unwrap_err_unchecked() && -> E { return std::move(this->release_err_unchecked()); }

private:
    constexpr auto release() -> T {
        if (m_has_result) {
            T released_value = std::move(this->unwrap());
            this->unwrap().~T();
            m_has_result = false;
            return released_value;
        }
    }
    constexpr auto release_unchecked() -> T {
        T released_value = std::move(this->unwrap_unchecked());
        this->unwrap_unchecked().~T();
        m_has_result = false;
        return released_value;
    }
    constexpr auto release_err() -> E {
        if (!m_has_result) {
            E released_value = std::move(this->unwrap_err());
            this->unwrap().~T();
            m_has_result = false;
            return released_value;
        }
    }
    constexpr auto release_err_unchecked() -> E {
        E released_value = std::move(this->unwrap_err_unchecked());
        this->unwrap_unchecked().~T();
        m_has_result = false;
        return released_value;
    }

public:
    constexpr auto is_ok() const noexcept { return m_has_result; }
    constexpr auto is_err() const noexcept { return !this->is_ok(); }

    bool m_has_result = false;
    union {
        alignas(T) u8 m_ok[sizeof(T)];
        alignas(E) u8 m_err[sizeof(E)];
    };
};

} // namespace JadeFrame