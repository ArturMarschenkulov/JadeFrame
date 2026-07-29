#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/utils/assert.h"

#include <utility>

#include <concepts>
#include <type_traits>
#include <variant>

namespace JadeFrame {
namespace result {

namespace details {
template<typename T>
using StoredValue = std::conditional_t<
    std::is_lvalue_reference_v<T>,
    std::reference_wrapper<std::remove_reference_t<T>>,
    T
>;
} // namespace details

template<typename E>
class Failure {
public:
    Failure(E error)
        : m_error(std::move(error)) {}

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
        !std::is_reference_v<E>,
        "It is ill-formed for E to be a reference. Only T types may be lvalue referneces"
    );

public:
    constexpr Result()
        requires(std::is_default_constructible_v<T>)
        : m_storage(std::in_place_index<0>) {}

    constexpr Result(const Result&) = default;
    constexpr Result(Result&&) = default;
    constexpr auto operator=(const Result&) -> Result& = default;
    constexpr auto operator=(Result&&) -> Result& = default;

    template<typename U>
        requires(
            !std::is_lvalue_reference_v<T> &&
            !std::same_as<std::remove_cvref_t<U>, Failure<E>> &&
            std::constructible_from<T, U&&>
        )
    constexpr Result(U&& v)
        : m_storage(std::in_place_index<0>, std::forward<U>(v)) {}

    constexpr Result(T v)
        requires(std::is_lvalue_reference_v<T>)
        : m_storage(std::in_place_index<0>, std::ref(v)) {}

    constexpr Result(const Failure<E>& f)
        : m_storage(std::in_place_index<1>, f.m_error) {}

    constexpr Result(Failure<E>&& f)
        : m_storage(std::in_place_index<1>, std::move(f.m_error)) {}

    constexpr ~Result() = default;

public:
    [[nodiscard]] auto is_ok() const -> bool {
        return m_storage.index() == 0;
    }

    [[nodiscard]] auto is_err() const -> bool { return !this->is_ok(); }

    [[nodiscard]] auto value() & -> decltype(auto) {
        return this->value_storage();
    }

    [[nodiscard]] auto value() const& -> decltype(auto) {
        return this->value_storage();
    }

    [[nodiscard]] auto value() && -> T {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return std::get<0>(m_storage).get();
        } else {
            return std::move(std::get<0>(m_storage));
        }
    }

    [[nodiscard]] auto error() & -> E& { return std::get<1>(m_storage); }

    [[nodiscard]] auto error() const& -> const E& { return std::get<1>(m_storage); }

    [[nodiscard]] auto error() && -> E { return std::move(std::get<1>(m_storage)); }

private:
    using StoredT = details::StoredValue<T>;
    std::variant<StoredT, E> m_storage;

    auto value_storage() -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return std::get<0>(m_storage).get();
        } else {
            return (std::get<0>(m_storage));
        }
    }

    auto value_storage() const -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<T>) {
            return std::get<0>(m_storage).get();
        } else {
            return (std::get<0>(m_storage));
        }
    }
};

} // namespace result

using result::Failure;
using result::Result;
} // namespace JadeFrame
