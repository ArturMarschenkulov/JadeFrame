#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/utils/assert.h"

#include <utility>

#include <concepts>
#include <type_traits>

namespace JadeFrame {
namespace result {


namespace details {
struct ResultTag {
    explicit ResultTag() = default;
};
struct ErrorTag {
    explicit ErrorTag() = default;
};
template<typename T, typename E>
class Storage {
public:
    bool m_has_value;
};
template<typename T, typename E>
requires std::is_lvalue_reference_v<T>
class Storage<T, E> {
public:
    // constexpr Storage(const T& v)
    //     : m_has_value(true)
    //     , m_pointer(&v) {}
    // constexpr Storage(T&& v) requires(!std::is_lvalue_reference_v<T>)
    //     : m_has_value(true)
    //     , m_pointer(&v) {}

private:
    union {
        T* m_value;
        E  m_error;
    };
    bool m_has_value;
};

template<typename T, typename E>
requires(!std::is_lvalue_reference_v<T>) class Storage<T, E> {
public:
    constexpr Storage(const T& v)
        : m_has_value(true) {
        new (&m_value) T(v);
    }
    constexpr Storage(T&& v)
        : m_has_value(true) {
        new (&m_value) T(std::forward<T>(v));
    }
    constexpr Storage(details::ErrorTag, E& v)
        : m_has_value(false) {
        new (&m_error) E(v);
    }
    constexpr Storage(details::ErrorTag, E&& v)
        : m_has_value(false) {
        new (&m_error) E(std::forward<E>(v));
    }

private:
    union {
        T m_value;
        E m_error;
    };
    bool m_has_value;
};
} // namespace details

enum ResultType {
    OK,
    ERR
};

template<typename E>
class Failure {
public:
    Failure(E error)
        : m_error(error) {}

    E m_error;
};

template<typename T>
Failure(std::reference_wrapper<T>) -> Failure<T&>;

template<typename T>
Failure(T&&) -> Failure<typename std::decay_t<T>>;

template<typename T, typename E>
class Result {
    static_assert(!std::is_abstract_v<T>, "It is ill-formed for T to be abstract type");
    static_assert(
        !std::is_rvalue_reference_v<T>,
        "It is ill-formed for T to be rvalue reference. Only lvalue references are valid");
    static_assert(
        !std::is_reference_v<E>, "It is ill-formed for E to be a reference. Only T types may be lvalue referneces");


public:
    constexpr Result() requires(std::is_default_constructible_v<T>)
        : m_storage() {}
    constexpr Result(const Result& o)
        : m_storage(o.m_storage) {}
    constexpr Result(Result&& o) requires(!std::is_lvalue_reference_v<T>)
        : m_storage(std::forward<Result>(o.m_storage)) {}
    constexpr Result(const T& v)
        : m_storage(v) {}

    constexpr Result(const Failure<E>& f)
        : m_storage(details::ErrorTag{}, f.m_error) {}
    constexpr Result(Failure<E>&& f)
        : m_storage(details::ErrorTag{}, std::forward<E>(f.m_error)) {}

    constexpr Result(T&& v) requires(!std::is_lvalue_reference_v<T>)
        : m_storage(std::forward<T>(std::move(v))) {}

private:
    details::Storage<T, E> m_storage;
};
namespace to_be_removed {
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
} // namespace to_be_removed

namespace tests {
enum class Version {
    VER1,
    VER2
};
auto parse_version(const int& version) -> Result<Version, const char*> {
    if (version == 1) {
        return Result<Version, const char*>(Version::VER2);
    } else if (version == 2) {
        return Result<Version, const char*>(Version::VER2);
    } else {
        auto f = Failure("invalid version");
        return f;
    }
}
auto general_usage() {}
auto test() -> void {}
} // namespace tests

static auto test() -> void { tests::test(); }
} // namespace result
using result::Failure;
using result::Result;
} // namespace JadeFrame