#pragma once

#include "JadeFrame/utils/assert.h"
#include "JadeFrame/utils/option.h"

#include <concepts>
#include <exception>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace JadeFrame {
namespace result {

template<typename T, typename E>
class Result;

namespace details {

struct unit {};

template<typename T>
struct stored_type {
    using type = std::conditional_t<
        std::is_lvalue_reference_v<T>,
        std::reference_wrapper<std::remove_reference_t<T>>,
        std::remove_cv_t<T>>;
};

template<>
struct stored_type<void> {
    using type = unit;
};

template<typename T>
using stored_type_t = typename stored_type<T>::type;

template<typename T>
struct const_reference_type {
    using type = const std::remove_reference_t<T>&;
};

template<>
struct const_reference_type<void> {
    using type = unit;
};

template<typename T>
using const_reference_t = typename const_reference_type<T>::type;

template<typename T>
struct mutable_reference_type {
    using type = std::remove_reference_t<T>&;
};

template<>
struct mutable_reference_type<void> {
    using type = unit;
};

template<typename T>
using mutable_reference_t = typename mutable_reference_type<T>::type;

template<typename T>
struct result_traits;

template<typename T, typename E>
struct result_traits<Result<T, E>> {
    using value_type = T;
    using error_type = E;
};

template<typename T>
concept result_type = requires {
    typename result_traits<std::remove_cvref_t<T>>::value_type;
    typename result_traits<std::remove_cvref_t<T>>::error_type;
};

template<typename F, typename Arg, bool = std::invocable<F, Arg>>
struct safe_invoke_result {
    using type = void;
};

template<typename F, typename Arg>
struct safe_invoke_result<F, Arg, true> {
    using type = std::invoke_result_t<F, Arg>;
};

template<typename F, typename T, bool = std::is_void_v<T>>
struct result_invoke;

template<typename F, typename T>
struct result_invoke<F, T, false> {
    using result_type = typename safe_invoke_result<F, const T&>::type;
    static constexpr bool invocable = std::invocable<F, const T&>;
};

template<typename F, typename T>
struct result_invoke<F, T, true> {
    using result_type = typename safe_invoke_result<F, void>::type;
    static constexpr bool invocable = std::invocable<F>;
};

template<typename F, typename T>
using result_invoke_result_t = typename result_invoke<F, T>::result_type;

template<typename F, typename T>
concept result_invocable = result_invoke<F, T>::invocable;

template<typename F, typename T, bool = std::is_void_v<T>>
struct result_rvalue_invoke;

template<typename F, typename T>
struct result_rvalue_invoke<F, T, false> {
    using result_type = typename safe_invoke_result<F, T&&>::type;
    static constexpr bool invocable = std::invocable<F, T&&>;
};

template<typename F, typename T>
struct result_rvalue_invoke<F, T, true> {
    using result_type = typename safe_invoke_result<F, void>::type;
    static constexpr bool invocable = std::invocable<F>;
};

template<typename F, typename T>
using result_rvalue_invoke_result_t = typename result_rvalue_invoke<F, T>::result_type;

template<typename F, typename T>
concept result_rvalue_invocable = result_rvalue_invoke<F, T>::invocable;

template<typename T>
concept equality_comparable = requires(const T& left, const T& right) { left == right; };

template<typename T>
struct is_failure : std::false_type {};

template<typename T>
struct is_success : std::false_type {};

template<typename T>
inline constexpr bool is_failure_v = is_failure<std::remove_cvref_t<T>>::value;

template<typename T>
inline constexpr bool is_success_v = is_success<std::remove_cvref_t<T>>::value;

enum class state : unsigned char {
    vacant,
    ok,
    err
};

struct ok_tag_t {
    explicit constexpr ok_tag_t() noexcept = default;
};

struct err_tag_t {
    explicit constexpr err_tag_t() noexcept = default;
};

inline constexpr ok_tag_t  ok_tag{};
inline constexpr err_tag_t err_tag{};

template<typename T, typename E>
class Storage {
public:
    using stored_value_type = stored_type_t<T>;
    using stored_error_type = stored_type_t<E>;

    constexpr Storage() = delete;

    template<typename... Args>
        requires std::constructible_from<stored_value_type, Args&&...>
    constexpr explicit Storage(ok_tag_t, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<stored_value_type, Args&&...>
    )
        : m_state(state::vacant) {
        std::construct_at(std::addressof(m_value), std::forward<Args>(args)...);
        m_state = state::ok;
    }

    template<typename... Args>
        requires std::constructible_from<stored_error_type, Args&&...>
    constexpr explicit Storage(err_tag_t, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<stored_error_type, Args&&...>
    )
        : m_state(state::vacant) {
        std::construct_at(std::addressof(m_error), std::forward<Args>(args)...);
        m_state = state::err;
    }

    constexpr Storage(const Storage& other) noexcept(
        std::is_nothrow_copy_constructible_v<stored_value_type> &&
        std::is_nothrow_copy_constructible_v<stored_error_type>
    )
        requires std::is_copy_constructible_v<stored_value_type> &&
                 std::is_copy_constructible_v<stored_error_type>
        : m_state(state::vacant) {
        if (other.is_ok()) {
            std::construct_at(std::addressof(m_value), other.m_value);
            m_state = state::ok;
        } else if (other.is_err()) {
            std::construct_at(std::addressof(m_error), other.m_error);
            m_state = state::err;
        }
    }

    constexpr Storage(const Storage&)
        requires(!std::is_copy_constructible_v<stored_value_type> ||
                 !std::is_copy_constructible_v<stored_error_type>)
    = delete;

    constexpr Storage(Storage&& other) noexcept(
        std::is_nothrow_move_constructible_v<stored_value_type> &&
        std::is_nothrow_move_constructible_v<stored_error_type>
    )
        requires std::is_move_constructible_v<stored_value_type> &&
                 std::is_move_constructible_v<stored_error_type>
        : m_state(state::vacant) {
        if (other.is_ok()) {
            std::construct_at(std::addressof(m_value), std::move(other.m_value));
            m_state = state::ok;
        } else if (other.is_err()) {
            std::construct_at(std::addressof(m_error), std::move(other.m_error));
            m_state = state::err;
        }
    }

    constexpr Storage(Storage&&)
        requires(!std::is_move_constructible_v<stored_value_type> ||
                 !std::is_move_constructible_v<stored_error_type>)
    = delete;

    constexpr ~Storage() noexcept(
        std::is_nothrow_destructible_v<stored_value_type> &&
        std::is_nothrow_destructible_v<stored_error_type>
    ) {
        destroy_active();
    }

    constexpr auto operator=(const Storage& other) noexcept(
        std::is_nothrow_copy_constructible_v<stored_value_type> &&
        std::is_nothrow_copy_constructible_v<stored_error_type> &&
        std::is_nothrow_move_constructible_v<stored_value_type> &&
        std::is_nothrow_move_constructible_v<stored_error_type>
    ) -> Storage&
        requires std::is_copy_constructible_v<stored_value_type> &&
                 std::is_copy_constructible_v<stored_error_type> &&
                 std::is_move_constructible_v<stored_value_type> &&
                 std::is_move_constructible_v<stored_error_type>
    {
        if (this != std::addressof(other)) {
            Storage replacement(other);
            swap(*this, replacement);
        }
        return *this;
    }

    constexpr auto operator=(const Storage&) -> Storage&
        requires(!std::is_copy_constructible_v<stored_value_type> ||
                 !std::is_copy_constructible_v<stored_error_type> ||
                 !std::is_move_constructible_v<stored_value_type> ||
                 !std::is_move_constructible_v<stored_error_type>)
    = delete;

    constexpr auto operator=(Storage&& other) noexcept(
        std::is_nothrow_move_constructible_v<stored_value_type> &&
        std::is_nothrow_move_constructible_v<stored_error_type>
    ) -> Storage&
        requires std::is_move_constructible_v<stored_value_type> &&
                 std::is_move_constructible_v<stored_error_type>
    {
        if (this != std::addressof(other)) {
            Storage replacement(std::move(other));
            swap(*this, replacement);
        }
        return *this;
    }

    constexpr auto operator=(Storage&&) -> Storage&
        requires(!std::is_move_constructible_v<stored_value_type> ||
                 !std::is_move_constructible_v<stored_error_type>)
    = delete;

    [[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
        return m_state == state::ok;
    }

    [[nodiscard]] constexpr auto is_err() const noexcept -> bool {
        return m_state == state::err;
    }

    [[nodiscard]] constexpr auto is_valid() const noexcept -> bool {
        return is_ok() || is_err();
    }

    constexpr auto destroy_active() noexcept(
        std::is_nothrow_destructible_v<stored_value_type> &&
        std::is_nothrow_destructible_v<stored_error_type>
    ) -> void {
        if (is_ok()) {
            m_state = state::vacant;
            std::destroy_at(std::addressof(m_value));
        } else if (is_err()) {
            m_state = state::vacant;
            std::destroy_at(std::addressof(m_error));
        }
    }

    template<typename... Args>
        requires std::constructible_from<stored_value_type, Args&&...>
    constexpr auto emplace_ok(
        Args&&... args
    ) noexcept(std::is_nothrow_constructible_v<stored_value_type, Args&&...>) -> void {
        destroy_active();
        std::construct_at(std::addressof(m_value), std::forward<Args>(args)...);
        m_state = state::ok;
    }

    template<typename... Args>
        requires std::constructible_from<stored_error_type, Args&&...>
    constexpr auto emplace_err(
        Args&&... args
    ) noexcept(std::is_nothrow_constructible_v<stored_error_type, Args&&...>) -> void {
        destroy_active();
        std::construct_at(std::addressof(m_error), std::forward<Args>(args)...);
        m_state = state::err;
    }

    constexpr auto value() & -> decltype(auto) {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_value.get();
        } else {
            return (m_value);
        }
    }

    constexpr auto value() const& -> decltype(auto) {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_value.get();
        } else {
            return (m_value);
        }
    }

    constexpr auto value() && -> decltype(auto) {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_value.get();
        } else {
            return std::move(m_value);
        }
    }

    constexpr auto value() const&& -> decltype(auto) {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_value.get();
        } else {
            return std::move(m_value);
        }
    }

    constexpr auto error() & -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return m_error.get();
        } else {
            return (m_error);
        }
    }

    constexpr auto error() const& -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return m_error.get();
        } else {
            return (m_error);
        }
    }

    constexpr auto error() && -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return m_error.get();
        } else {
            return std::move(m_error);
        }
    }

    constexpr auto error() const&& -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return m_error.get();
        } else {
            return std::move(m_error);
        }
    }

    friend constexpr auto swap(Storage& lhs, Storage& rhs) noexcept(
        std::is_nothrow_move_constructible_v<stored_value_type> &&
        std::is_nothrow_move_constructible_v<stored_error_type>
    ) -> void
        requires std::is_move_constructible_v<stored_value_type> &&
                 std::is_move_constructible_v<stored_error_type>
    {
        if (std::addressof(lhs) == std::addressof(rhs)) { return; }

        if (lhs.m_state == rhs.m_state) {
            if (lhs.is_ok()) {
                exchange(lhs.m_value, rhs.m_value);
            } else if (lhs.is_err()) {
                exchange(lhs.m_error, rhs.m_error);
            }
            return;
        }

        Storage temporary(std::move(lhs));
        lhs.destroy_active();
        if (rhs.is_ok()) {
            std::construct_at(std::addressof(lhs.m_value), std::move(rhs.m_value));
            lhs.m_state = state::ok;
        } else if (rhs.is_err()) {
            std::construct_at(std::addressof(lhs.m_error), std::move(rhs.m_error));
            lhs.m_state = state::err;
        }

        rhs.destroy_active();
        if (temporary.is_ok()) {
            std::construct_at(std::addressof(rhs.m_value), std::move(temporary.m_value));
            rhs.m_state = state::ok;
        } else if (temporary.is_err()) {
            std::construct_at(std::addressof(rhs.m_error), std::move(temporary.m_error));
            rhs.m_state = state::err;
        }
    }

private:
    template<typename V>
    static constexpr auto exchange(V& lhs, V& rhs) -> void
        requires std::is_move_constructible_v<V>
    {
        if (std::addressof(lhs) == std::addressof(rhs)) { return; }
        V temporary(std::move(lhs));
        std::destroy_at(std::addressof(lhs));
        std::construct_at(std::addressof(lhs), std::move(rhs));
        std::destroy_at(std::addressof(rhs));
        std::construct_at(std::addressof(rhs), std::move(temporary));
    }

    union {
        stored_value_type m_value;
        stored_error_type m_error;
    };

    state m_state = state::vacant;
};

} // namespace details

template<typename E>
class Failure {
public:
    using value_type = E;
    using stored_type = details::stored_type_t<E>;

    template<typename U>
        requires std::constructible_from<stored_type, U&&>
    constexpr explicit Failure(
        U&& error
    ) noexcept(std::is_nothrow_constructible_v<stored_type, U&&>)
        : m_error(std::forward<U>(error)) {}

    constexpr Failure(const Failure&) = default;
    constexpr Failure(Failure&&) = default;
    constexpr auto operator=(const Failure&) -> Failure& = default;
    constexpr auto operator=(Failure&&) -> Failure& = default;

    constexpr auto get() & -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return m_error.get();
        } else {
            return (m_error);
        }
    }

    constexpr auto get() const& -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return m_error.get();
        } else {
            return (m_error);
        }
    }

    constexpr auto get() && -> decltype(auto) {
        if constexpr (std::is_lvalue_reference_v<E>) {
            return m_error.get();
        } else {
            return std::move(m_error);
        }
    }

    stored_type m_error;
};

template<typename T>
Failure(T&&) -> Failure<std::decay_t<T>>;

template<typename T>
Failure(std::reference_wrapper<T>) -> Failure<T&>;

namespace details {

template<typename E>
struct is_failure<Failure<E>> : std::true_type {};

} // namespace details

template<typename T = void>
class Success {
public:
    using value_type = T;

    constexpr Success()
        requires std::is_void_v<T>
    = default;

    template<typename U>
        requires(!std::is_void_v<T>) &&
                std::constructible_from<details::stored_type_t<T>, U&&>
    constexpr explicit Success(U&& value)
        : m_value(std::forward<U>(value)) {}

    details::stored_type_t<T> m_value{};
};

namespace details {

template<typename T>
struct is_success<Success<T>> : std::true_type {};

} // namespace details

template<typename T>
[[nodiscard]] constexpr auto Ok(T&& value) -> Success<std::decay_t<T>> {
    return Success<std::decay_t<T>>(std::forward<T>(value));
}

[[nodiscard]] constexpr auto Ok() -> Success<void> { return Success<void>(); }

template<typename E>
[[nodiscard]] constexpr auto Err(E&& error) -> Failure<std::decay_t<E>> {
    return Failure<std::decay_t<E>>(std::forward<E>(error));
}

template<typename T, typename E>
class Result {
    static_assert(!std::is_rvalue_reference_v<T>, "Result<T&&, E> is not supported");
    static_assert(!std::is_rvalue_reference_v<E>, "Result<T, E&&> is not supported");
    static_assert(
        std::is_void_v<T> || std::is_lvalue_reference_v<T> || std::is_object_v<T>,
        "Result's value must be void, an object, or an lvalue reference"
    );
    static_assert(
        std::is_void_v<T> || std::is_lvalue_reference_v<T> || !std::is_abstract_v<T>,
        "Result's value type cannot be abstract"
    );
    static_assert(
        std::is_lvalue_reference_v<E> || std::is_object_v<E>,
        "Result's error must be an object or an lvalue reference"
    );
    static_assert(
        std::is_lvalue_reference_v<E> || !std::is_abstract_v<E>,
        "Result's error type cannot be abstract"
    );
    static_assert(std::is_void_v<E> == false, "Result's error type cannot be void");

    using storage_type = details::Storage<T, E>;
    using value_storage_type = typename storage_type::stored_value_type;
    using error_storage_type = typename storage_type::stored_error_type;

    template<typename U>
    static constexpr bool is_special_wrapper_v =
        details::is_failure_v<U> || details::is_success_v<U> ||
        std::same_as<std::remove_cvref_t<U>, Result>;

    [[noreturn]] static auto panic(const char* message) -> void {
        JF_PANIC(message);
        std::terminate();
    }

    storage_type m_storage;

    template<typename R>
    static constexpr auto make_error(R&& error) -> Failure<E> {
        return Failure<E>(std::forward<R>(error));
    }

    template<typename R>
    static constexpr auto make_success(R&& value) -> Success<T> {
        return Success<T>(std::forward<R>(value));
    }

public:
    using value_type = T;
    using error_type = E;

    constexpr Result()
        requires(std::is_void_v<T> || std::default_initializable<value_storage_type>)
        : m_storage(details::ok_tag) {}

    template<typename U>
        requires(!is_special_wrapper_v<U>) && (!std::is_void_v<T>) &&
                std::constructible_from<value_storage_type, U&&>
    constexpr Result(
        U&& value
    ) noexcept(std::is_nothrow_constructible_v<value_storage_type, U&&>)
        : m_storage(details::ok_tag, std::forward<U>(value)) {}

    template<typename U>
        requires details::is_success_v<U> && std::constructible_from<
                                                 value_storage_type,
                                                 decltype(std::declval<U&&>().m_value)>
    constexpr Result(U&& success)
        : m_storage(details::ok_tag, std::forward<U>(success).m_value) {}

    template<typename U>
        requires details::is_failure_v<U> && std::constructible_from<
                                                 error_storage_type,
                                                 decltype(std::declval<U&&>().get())>
    constexpr Result(U&& failure)
        : m_storage(details::err_tag, std::forward<U>(failure).get()) {}

    constexpr Result(
        const Result& other
    ) noexcept(std::is_nothrow_copy_constructible_v<storage_type>)
        requires std::is_copy_constructible_v<storage_type>
        : m_storage(other.m_storage) {}

    constexpr Result(const Result&)
        requires(!std::is_copy_constructible_v<storage_type>)
    = delete;

    constexpr Result(
        Result&& other
    ) noexcept(std::is_nothrow_move_constructible_v<storage_type>)
        requires std::is_move_constructible_v<storage_type>
        : m_storage(std::move(other.m_storage)) {}

    constexpr Result(Result&&)
        requires(!std::is_move_constructible_v<storage_type>)
    = delete;

    constexpr ~Result() = default;

    constexpr auto
    operator=(const Result& other) noexcept(noexcept(m_storage = other.m_storage))
        -> Result&
        requires requires(storage_type& lhs, const storage_type& rhs) { lhs = rhs; }
    {
        if (this != std::addressof(other)) { m_storage = other.m_storage; }
        return *this;
    }

    constexpr auto operator=(const Result&) -> Result&
        requires(!requires(storage_type& lhs, const storage_type& rhs) { lhs = rhs; })
    = delete;

    constexpr auto
    operator=(Result&& other) noexcept(noexcept(m_storage = std::move(other.m_storage)))
        -> Result&
        requires requires(storage_type& lhs, storage_type&& rhs) { lhs = std::move(rhs); }
    {
        if (this != std::addressof(other)) { m_storage = std::move(other.m_storage); }
        return *this;
    }

    constexpr auto operator=(Result&&) -> Result&
        requires(!requires(storage_type& lhs, storage_type&& rhs) {
                    lhs = std::move(rhs);
                })
    = delete;

    [[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
        return m_storage.is_ok();
    }

    [[nodiscard]] constexpr auto is_err() const noexcept -> bool {
        return m_storage.is_err();
    }

    [[nodiscard]] constexpr auto is_valid() const noexcept -> bool {
        return m_storage.is_valid();
    }

    constexpr explicit operator bool() const noexcept { return is_ok(); }

    constexpr decltype(auto) value() & {
        if (is_err() || !is_valid()) {
            panic("called `Result::value()` on an `Err` value");
        }
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return static_cast<T&>(m_storage.value());
        }
    }

    [[nodiscard]] constexpr decltype(auto) value() const& {
        if (is_err() || !is_valid()) {
            panic("called `Result::value()` on an `Err` value");
        }
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return static_cast<const T&>(m_storage.value());
        }
    }

    constexpr decltype(auto) value() && {
        if (is_err() || !is_valid()) {
            panic("called `Result::value()` on an `Err` value");
        }
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_storage.value();
        } else {
            return static_cast<T>(std::move(m_storage).value());
        }
    }

    [[nodiscard]] constexpr decltype(auto) value() const&& {
        if (is_err() || !is_valid()) {
            panic("called `Result::value()` on an `Err` value");
        }
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_storage.value();
        } else {
            return static_cast<const T&&>(std::move(m_storage).value());
        }
    }

    constexpr decltype(auto) unwrap() & { return value(); }

    [[nodiscard]] constexpr decltype(auto) unwrap() const& { return value(); }

    constexpr decltype(auto) unwrap() && { return std::move(*this).value(); }

    constexpr decltype(auto) expect(const char* message) & {
        if (is_err() || !is_valid()) { panic(message); }
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return static_cast<T&>(m_storage.value());
        }
    }

    [[nodiscard]] constexpr decltype(auto) expect(const char* message) const& {
        if (is_err() || !is_valid()) { panic(message); }
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return static_cast<const T&>(m_storage.value());
        }
    }

    constexpr decltype(auto) expect(const char* message) && {
        if (is_err() || !is_valid()) { panic(message); }
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_storage.value();
        } else {
            return static_cast<T>(std::move(m_storage).value());
        }
    }

    constexpr decltype(auto) unwrap_unchecked() & {
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return static_cast<T&>(m_storage.value());
        }
    }

    [[nodiscard]] constexpr decltype(auto) unwrap_unchecked() const& {
        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return static_cast<const T&>(m_storage.value());
        }
    }

    constexpr decltype(auto) unwrap_unchecked() && {
        if constexpr (std::is_void_v<T>) {
            return;
        } else if constexpr (std::is_lvalue_reference_v<T>) {
            return m_storage.value();
        } else {
            return static_cast<T>(std::move(m_storage).value());
        }
    }

    constexpr auto error() & -> E& {
        if (is_ok() || !is_valid()) {
            panic("called `Result::error()` on an `Ok` value");
        }
        return m_storage.error();
    }

    [[nodiscard]] constexpr auto error() const& -> const E& {
        if (is_ok() || !is_valid()) {
            panic("called `Result::error()` on an `Ok` value");
        }
        return m_storage.error();
    }

    constexpr auto error() && -> E {
        if (is_ok() || !is_valid()) {
            panic("called `Result::error()` on an `Ok` value");
        }
        return std::move(m_storage).error();
    }

    constexpr auto unwrap_err() & -> E& { return error(); }

    [[nodiscard]] constexpr auto unwrap_err() const& -> const E& { return error(); }

    constexpr auto unwrap_err() && -> E { return std::move(*this).error(); }

    constexpr auto expect_err(const char* message) & -> E& {
        if (is_ok() || !is_valid()) { panic(message); }
        return m_storage.error();
    }

    [[nodiscard]] constexpr auto expect_err(const char* message) const& -> const E& {
        if (is_ok() || !is_valid()) { panic(message); }
        return m_storage.error();
    }

    constexpr auto expect_err(const char* message) && -> E {
        if (is_ok() || !is_valid()) { panic(message); }
        return std::move(m_storage).error();
    }

    template<typename U>
    [[nodiscard]] constexpr auto contains(const U& expected) const -> bool
        requires(!std::is_void_v<T>) &&
                std::same_as<std::remove_cvref_t<U>, std::remove_cvref_t<T>> &&
                requires(const T& left, const U& right) { left == right; }
    {
        return is_ok() && (value() == expected);
    }

    [[nodiscard]] constexpr auto contains_err(const E& expected) const -> bool
        requires requires(const E& left, const E& right) { left == right; }
    {
        return is_err() && (error() == expected);
    }

    [[nodiscard]] constexpr auto
    ok() const& -> Option<std::conditional_t<std::is_void_v<T>, details::unit, T>> {
        using U = std::conditional_t<std::is_void_v<T>, details::unit, T>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return Option<U>(details::unit{});
            } else {
                return Option<U>(value());
            }
        }
        return Option<U>();
    }

    [[nodiscard]] constexpr auto
    ok() && -> Option<std::conditional_t<std::is_void_v<T>, details::unit, T>> {
        using U = std::conditional_t<std::is_void_v<T>, details::unit, T>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return Option<U>(details::unit{});
            } else {
                return Option<U>(std::move(m_storage).value());
            }
        }
        return Option<U>();
    }

    [[nodiscard]] constexpr auto err() const& -> Option<E> {
        if (is_err()) { return Option<E>(error()); }
        return Option<E>();
    }

    [[nodiscard]] constexpr auto err() && -> Option<E> {
        if (is_err()) { return Option<E>(std::move(m_storage).error()); }
        return Option<E>();
    }

    [[nodiscard]] constexpr auto as_ref() const& -> Result<
        details::const_reference_t<T>,
        const std::remove_reference_t<E>&>
        requires(!std::is_void_v<T>)
    {
        using ValueReference = details::const_reference_t<T>;
        using ErrorReference = const std::remove_reference_t<E>&;
        if (is_ok()) { return Result<ValueReference, ErrorReference>(value()); }
        return Result<ValueReference, ErrorReference>(Failure<ErrorReference>(error()));
    }

    [[nodiscard]] constexpr auto
    as_mut() & -> Result<details::mutable_reference_t<T>, std::remove_reference_t<E>&>
        requires(!std::is_void_v<T>)
    {
        using ValueReference = details::mutable_reference_t<T>;
        using ErrorReference = std::remove_reference_t<E>&;
        if (is_ok()) { return Result<ValueReference, ErrorReference>(value()); }
        return Result<ValueReference, ErrorReference>(Failure<ErrorReference>(error()));
    }

    template<typename U = T>
        requires(!std::is_void_v<T>) && std::constructible_from<T, U&&>
    [[nodiscard]] constexpr auto unwrap_or(U&& fallback) const& -> T {
        if (is_ok()) { return value(); }
        return std::forward<U>(fallback);
    }

    template<typename U = T>
        requires(!std::is_void_v<T>) && std::constructible_from<T, U&&>
    constexpr auto unwrap_or(U&& fallback) && -> T {
        if (is_ok()) { return std::move(m_storage).value(); }
        return std::forward<U>(fallback);
    }

    template<typename F>
        requires(!std::is_void_v<T>) && std::invocable<F> &&
                std::constructible_from<T, std::invoke_result_t<F>>
    [[nodiscard]] constexpr auto unwrap_or_else(F&& function) const& -> T {
        if (is_ok()) { return value(); }
        return std::invoke(std::forward<F>(function));
    }

    template<typename F>
        requires(!std::is_void_v<T>) && std::invocable<F> &&
                std::constructible_from<T, std::invoke_result_t<F>>
    constexpr auto unwrap_or_else(F&& function) && -> T {
        if (is_ok()) { return std::move(m_storage).value(); }
        return std::invoke(std::forward<F>(function));
    }

    constexpr auto unwrap_or_default() const& -> T
        requires(!std::is_void_v<T>) && std::default_initializable<T>
    {
        return unwrap_or(T{});
    }

    constexpr auto unwrap_or_default() && -> T
        requires(!std::is_void_v<T>) && std::default_initializable<T>
    {
        return std::move(*this).unwrap_or(T{});
    }

    template<typename F>
        requires details::result_invocable<F&&, T>
    [[nodiscard]] constexpr auto map(
        F&& function
    ) const& -> Result<std::remove_cvref_t<details::result_invoke_result_t<F&&, T>>, E> {
        using U = std::remove_cvref_t<details::result_invoke_result_t<F&&, T>>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                if constexpr (std::is_void_v<U>) {
                    std::invoke(std::forward<F>(function));
                    return Result<U, E>();
                } else {
                    return Result<U, E>(std::invoke(std::forward<F>(function)));
                }
            } else {
                if constexpr (std::is_void_v<U>) {
                    std::invoke(std::forward<F>(function), value());
                    return Result<U, E>();
                } else {
                    return Result<U, E>(std::invoke(std::forward<F>(function), value()));
                }
            }
        }
        return Result<U, E>(Failure<E>(error()));
    }

    template<typename F>
        requires details::result_rvalue_invocable<F&&, T>
    [[nodiscard]] constexpr auto map(F&& function) && -> Result<
        std::remove_cvref_t<details::result_rvalue_invoke_result_t<F&&, T>>,
        E> {
        using U = std::remove_cvref_t<details::result_rvalue_invoke_result_t<F&&, T>>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                if constexpr (std::is_void_v<U>) {
                    std::invoke(std::forward<F>(function));
                    return Result<U, E>();
                } else {
                    return Result<U, E>(std::invoke(std::forward<F>(function)));
                }
            } else {
                if constexpr (std::is_void_v<U>) {
                    std::invoke(std::forward<F>(function), std::move(m_storage).value());
                    return Result<U, E>();
                } else {
                    return Result<U, E>(std::invoke(
                        std::forward<F>(function), std::move(m_storage).value()
                    ));
                }
            }
        }
        return Result<U, E>(Failure<E>(std::move(m_storage).error()));
    }

    template<typename F>
        requires details::result_invocable<F&&, E> &&
                 (!std::is_void_v<details::result_invoke_result_t<F &&, E>>)
    [[nodiscard]] constexpr auto map_err(
        F&& function
    ) const& -> Result<T, std::remove_cvref_t<details::result_invoke_result_t<F&&, E>>> {
        using U = std::remove_cvref_t<details::result_invoke_result_t<F&&, E>>;
        if (is_err()) {
            return Result<T, U>(
                Failure<U>(std::invoke(std::forward<F>(function), error()))
            );
        }
        if constexpr (std::is_void_v<T>) {
            return Result<T, U>();
        } else {
            return Result<T, U>(value());
        }
    }

    template<typename F>
        requires std::invocable<F&&, E&&> &&
                 (!std::is_void_v<std::invoke_result_t<F &&, E &&>>)
    [[nodiscard]] constexpr auto map_err(
        F&& function
    ) && -> Result<T, std::remove_cvref_t<std::invoke_result_t<F&&, E&&>>> {
        using U = std::remove_cvref_t<std::invoke_result_t<F&&, E&&>>;
        if (is_err()) {
            return Result<T, U>(Failure<U>(
                std::invoke(std::forward<F>(function), std::move(m_storage).error())
            ));
        }
        if constexpr (std::is_void_v<T>) {
            return Result<T, U>();
        } else {
            return Result<T, U>(std::move(m_storage).value());
        }
    }

    template<typename D, typename F>
        requires details::result_invocable<F&&, T> &&
                 (!std::is_void_v<details::result_invoke_result_t<F &&, T>>) &&
                 std::constructible_from<
                     std::remove_cvref_t<details::result_invoke_result_t<F&&, T>>,
                     D&&>
    [[nodiscard]] constexpr auto map_or(
        D&& default_value,
        F&& function
    ) const& -> std::remove_cvref_t<details::result_invoke_result_t<F&&, T>> {
        using R = std::remove_cvref_t<details::result_invoke_result_t<F&&, T>>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return R(std::invoke(std::forward<F>(function)));
            } else {
                return R(std::invoke(std::forward<F>(function), value()));
            }
        }
        return R(std::forward<D>(default_value));
    }

    template<typename DefaultF, typename F>
        requires std::invocable<DefaultF&&> && details::result_invocable<F&&, T> &&
                 (!std::is_void_v<details::result_invoke_result_t<F &&, T>>) &&
                 std::constructible_from<
                     std::remove_cvref_t<details::result_invoke_result_t<F&&, T>>,
                     std::invoke_result_t<DefaultF&&>>
    [[nodiscard]] constexpr auto map_or_else(
        DefaultF&& default_function,
        F&&        function
    ) const& -> std::remove_cvref_t<details::result_invoke_result_t<F&&, T>> {
        using R = std::remove_cvref_t<details::result_invoke_result_t<F&&, T>>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return R(std::invoke(std::forward<F>(function)));
            } else {
                return R(std::invoke(std::forward<F>(function), value()));
            }
        }
        return R(std::invoke(std::forward<DefaultF>(default_function)));
    }

    template<typename F>
        requires details::result_invocable<F&&, T> &&
                 details::result_type<details::result_invoke_result_t<F&&, T>>
    [[nodiscard]] constexpr auto and_then(
        F&& function
    ) const& -> std::remove_cvref_t<details::result_invoke_result_t<F&&, T>> {
        using R = std::remove_cvref_t<details::result_invoke_result_t<F&&, T>>;
        using output_error = typename details::result_traits<R>::error_type;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return std::invoke(std::forward<F>(function));
            } else {
                return std::invoke(std::forward<F>(function), value());
            }
        }
        return R(Failure<output_error>(error()));
    }

    template<typename F>
        requires details::result_rvalue_invocable<F&&, T> &&
                 details::result_type<details::result_rvalue_invoke_result_t<F&&, T>>
    constexpr auto and_then(
        F&& function
    ) && -> std::remove_cvref_t<details::result_rvalue_invoke_result_t<F&&, T>> {
        using R = std::remove_cvref_t<details::result_rvalue_invoke_result_t<F&&, T>>;
        using output_error = typename details::result_traits<R>::error_type;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return std::invoke(std::forward<F>(function));
            } else {
                return std::invoke(
                    std::forward<F>(function), std::move(m_storage).value()
                );
            }
        }
        return R(Failure<output_error>(std::move(m_storage).error()));
    }

    template<typename U>
        requires std::is_copy_constructible_v<storage_type> &&
                 std::is_copy_constructible_v<details::Storage<U, E>>
    [[nodiscard]] constexpr auto and_(const Result<U, E>& other) const& -> Result<U, E> {
        if (is_ok()) { return other; }
        return Result<U, E>(Failure<E>(error()));
    }

    template<typename U>
    [[nodiscard]] constexpr auto and_(Result<U, E>&& other) const& -> Result<U, E> {
        if (is_ok()) { return std::move(other); }
        return Result<U, E>(Failure<E>(error()));
    }

    template<typename U>
    [[nodiscard]] constexpr auto and_(Result<U, E>&& other) && -> Result<U, E> {
        if (is_ok()) { return std::move(other); }
        return Result<U, E>(Failure<E>(std::move(m_storage).error()));
    }

    template<typename F>
        requires std::invocable<F&&, const E&> &&
                 details::result_type<std::invoke_result_t<F&&, const E&>>
    [[nodiscard]] constexpr auto or_else(
        F&& function
    ) const& -> std::remove_cvref_t<std::invoke_result_t<F&&, const E&>> {
        using R = std::remove_cvref_t<std::invoke_result_t<F&&, const E&>>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return R(Ok());
            } else {
                return R(value());
            }
        }
        return std::invoke(std::forward<F>(function), error());
    }

    template<typename F>
        requires details::result_type<std::invoke_result_t<F&&, E&&>>
    constexpr auto
    or_else(F&& function) && -> std::remove_cvref_t<std::invoke_result_t<F&&, E&&>> {
        using R = std::remove_cvref_t<std::invoke_result_t<F&&, E&&>>;
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return R(Ok());
            } else {
                return R(std::move(m_storage).value());
            }
        }
        return std::invoke(std::forward<F>(function), std::move(m_storage).error());
    }

    template<typename U>
        requires std::is_copy_constructible_v<storage_type> &&
                 std::is_copy_constructible_v<details::Storage<T, U>>
    [[nodiscard]] constexpr auto or_(const Result<T, U>& other) const& -> Result<T, U> {
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return Result<T, U>();
            } else {
                return Result<T, U>(value());
            }
        }
        return other;
    }

    template<typename U>
    [[nodiscard]] constexpr auto or_(Result<T, U>&& other) const& -> Result<T, U> {
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return Result<T, U>();
            } else {
                return Result<T, U>(value());
            }
        }
        return std::move(other);
    }

    template<typename F>
        requires details::result_invocable<F&&, T>
    [[nodiscard]] constexpr auto inspect(F&& function) const& -> const Result& {
        if (is_ok()) {
            if constexpr (std::is_void_v<T>) {
                std::invoke(std::forward<F>(function));
            } else {
                std::invoke(std::forward<F>(function), value());
            }
        }
        return *this;
    }

    template<typename F>
        requires std::invocable<F&&, const E&>
    [[nodiscard]] constexpr auto inspect_err(F&& function) const& -> const Result& {
        if (is_err()) { std::invoke(std::forward<F>(function), error()); }
        return *this;
    }

    friend constexpr auto
    swap(Result& lhs, Result& rhs) noexcept(noexcept(swap(lhs.m_storage, rhs.m_storage)))
        -> void
        requires requires { swap(lhs.m_storage, rhs.m_storage); }
    {
        swap(lhs.m_storage, rhs.m_storage);
    }

    friend constexpr auto operator==(const Result& lhs, const Result& rhs) -> bool
        requires(
            (std::is_void_v<T> || details::equality_comparable<T>) &&
            details::equality_comparable<E>
        )
    {
        if (lhs.is_ok() != rhs.is_ok()) { return false; }
        if (!lhs.is_valid() || !rhs.is_valid()) {
            return lhs.is_valid() == rhs.is_valid();
        }
        if (lhs.is_ok()) {
            if constexpr (std::is_void_v<T>) {
                return true;
            } else {
                return lhs.value() == rhs.value();
            }
        }
        return lhs.error() == rhs.error();
    }

    friend constexpr auto operator!=(const Result& lhs, const Result& rhs)
        requires requires { lhs == rhs; }
    {
        return !(lhs == rhs);
    }
};

} // namespace result

using result::Err;
using result::Failure;
using result::Ok;
using result::Result;
using result::Success;

} // namespace JadeFrame
