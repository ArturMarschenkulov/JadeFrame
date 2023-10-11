#pragma once
#include "JadeFrame/prelude.h"
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
    constexpr Storage(const Storage& o)
        : m_has_value(o.m_has_value)
        , m_value(o.m_value) {}

    constexpr Storage(Storage&& o)
        : m_has_value(o.m_has_value)
        , m_value(o.m_value) {
        o.m_has_value = false;
        o.m_value = nullptr;
    }

    constexpr Storage(const T& v)
        : m_has_value(true)
        , m_value(&v) {}

    constexpr Storage(T&& v)
        requires(!std::is_lvalue_reference_v<T>)
        : m_has_value(true)
        , m_value(&v) {}

    constexpr Storage(details::ErrorTag, E& v)
        : m_has_value(false)
        , m_error(v) {}

    constexpr Storage(details::ErrorTag, E&& v)
        requires(!std::is_lvalue_reference_v<E>)
        : m_has_value(false)
        , m_error(v) {}

    constexpr ~Storage() {
        if (m_has_value) {
            m_value = nullptr;
        } else {
            m_error.~E();
        }
    }

private:
    union {
        std::remove_reference_t<T>* m_value;
        E                           m_error;
    };

    bool m_has_value;
};

template<typename T, typename E>
    requires(!std::is_lvalue_reference_v<T>)
class Storage<T, E> {
public:
    constexpr Storage(const T& v)
        : m_has_value(true) {
        new (&m_value) T(v);
    }

    constexpr Storage(T&& v)
        requires(!std::is_lvalue_reference_v<T>)
        : m_has_value(true) {
        new (&m_value) T(std::forward<T>(v));
    }

    constexpr Storage(details::ErrorTag, E& v)
        : m_has_value(false) {
        new (&m_error) E(v);
    }

    constexpr Storage(details::ErrorTag, E&& v)
        requires(!std::is_lvalue_reference_v<T>)
        : m_has_value(false) {
        new (&m_error) E(std::forward<E>(v));
    }

    ~Storage() {
        if (m_has_value) {
            m_value.~T();
        } else {
            m_error.~E();
        }
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
        "It is ill-formed for T to be rvalue reference. Only lvalue references are valid"
    );
    static_assert(
        !std::is_reference_v<E>, "It is ill-formed for E to be a reference. Only T types may be lvalue referneces"
    );

public:
    constexpr Result()
        requires(std::is_default_constructible_v<T>)
        : m_storage() {}

    constexpr Result(const Result& o)
        : m_storage(o.m_storage) {}

    constexpr Result(Result&& o)
        requires(!std::is_lvalue_reference_v<T>)
        : m_storage(std::forward<Result>(o.m_storage)) {}

    constexpr Result(const T& v)
        : m_storage(v) {}

    constexpr Result(const Failure<E>& f)
        : m_storage(details::ErrorTag{}, f.m_error) {}

    constexpr Result(Failure<E>&& f)
        : m_storage(details::ErrorTag{}, std::forward<E>(f.m_error)) {}

    constexpr Result(T&& v)
        requires(!std::is_lvalue_reference_v<T>)
        : m_storage(std::forward<T>(std::move(v))) {}

    constexpr ~Result() {}

public:
    auto is_ok() const -> bool { return m_storage.m_has_value; }

    auto is_err() const -> bool { return !this->is_ok(); }

private:
    details::Storage<T, E> m_storage;
};

namespace tests {

struct Student {
    std::string name;
    int         age;

    auto get_name() const -> Result<const std::string&, std::string> {
        if (age < 18) {
            return Failure(std::string("Too young"));
        } else {
            return Result<const std::string&, std::string>(name);
        }
    }
};

auto pp() -> void {}
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

auto general_usage() { auto res = parse_version(1); }

auto test() -> void {}
} // namespace tests

static auto test() -> void { tests::test(); }
} // namespace result

using result::Failure;
using result::Result;
} // namespace JadeFrame