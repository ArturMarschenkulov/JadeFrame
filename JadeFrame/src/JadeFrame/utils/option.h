#pragma once

#include "JadeFrame/types.h"
#include "JadeFrame/utils/assert.h"

#include <concepts>
#include <exception>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace JadeFrame {

// A value that can be used to explicitly construct or assign an empty Option.
struct none_t {
    explicit constexpr none_t() noexcept = default;
};

inline constexpr none_t None{};

namespace option {

template<typename T>
class Option;

namespace details {

template<typename T>
class Storage;

// References are represented by a nullable pointer. The pointed-to object is
// never owned and is never destroyed by Storage.
template<typename T>
    requires std::is_lvalue_reference_v<T>
class Storage<T> {
public:
    using value_type = T;

    constexpr Storage() noexcept = default;
    constexpr ~Storage() noexcept = default;
    constexpr Storage(const Storage&) noexcept = default;
    constexpr Storage(Storage&&) noexcept = default;
    constexpr auto operator=(const Storage&) noexcept -> Storage& = default;
    constexpr auto operator=(Storage&&) noexcept -> Storage& = default;

    template<typename U>
        requires std::is_lvalue_reference_v<U&&> && std::convertible_to<U&&, T>
    constexpr explicit Storage(U&& value) noexcept
        : m_pointer(std::addressof(value)) {}

    template<typename U>
        requires std::is_lvalue_reference_v<U&&> && std::convertible_to<U&&, T>
    constexpr auto emplace(U&& value) noexcept -> void {
        m_pointer = std::addressof(value);
    }

    constexpr auto reset() noexcept -> void { m_pointer = nullptr; }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool {
        return m_pointer != nullptr;
    }

    [[nodiscard]] constexpr auto get() & noexcept -> T { return *m_pointer; }

    [[nodiscard]] constexpr auto get() const& noexcept -> T { return *m_pointer; }

    [[nodiscard]] constexpr auto get() && noexcept -> T { return *m_pointer; }

    [[nodiscard]] constexpr auto get() const&& noexcept -> T { return *m_pointer; }

private:
    using referred_type = std::remove_reference_t<T>;
    referred_type* m_pointer = nullptr;
};

// Values are stored in a union so that an empty Option does not require a
// default-constructible payload and does not construct a payload at all.
template<typename T>
    requires(!std::is_lvalue_reference_v<T>)
class Storage<T> {
public:
    using value_type = T;
    using stored_type = std::remove_cv_t<T>;

    static_assert(
        std::is_object_v<T>,
        "Option requires an object or lvalue-reference value type"
    );
    static_assert(
        std::is_destructible_v<stored_type>,
        "Option's value type must be destructible"
    );

    constexpr Storage() noexcept
        : m_empty(0)
        , m_has_value(false) {}

    constexpr ~Storage() noexcept(std::is_nothrow_destructible_v<stored_type>)
        requires std::is_trivially_destructible_v<stored_type>
    = default;

    constexpr ~Storage() noexcept(std::is_nothrow_destructible_v<stored_type>)
        requires(!std::is_trivially_destructible_v<stored_type>)
    {
        reset();
    }

    constexpr Storage(
        const Storage& other
    ) noexcept(std::is_nothrow_copy_constructible_v<stored_type>)
        requires std::is_copy_constructible_v<stored_type>
        : m_empty(0)
        , m_has_value(false) {
        if (other.m_has_value) {
            std::construct_at(std::addressof(m_value), other.m_value);
            m_has_value = true;
        }
    }

    constexpr Storage(const Storage&)
        requires(!std::is_copy_constructible_v<stored_type>)
    = delete;

    constexpr Storage(
        Storage&& other
    ) noexcept(std::is_nothrow_move_constructible_v<stored_type>)
        requires std::is_move_constructible_v<stored_type>
        : m_empty(0)
        , m_has_value(false) {
        if (other.m_has_value) {
            std::construct_at(std::addressof(m_value), std::move(other.m_value));
            m_has_value = true;
        }
    }

    constexpr Storage(Storage&&)
        requires(!std::is_move_constructible_v<stored_type>)
    = delete;

    template<typename U>
        requires std::constructible_from<stored_type, U&&>
    constexpr explicit Storage(
        U&& value
    ) noexcept(std::is_nothrow_constructible_v<stored_type, U&&>)
        : m_empty(0)
        , m_has_value(false) {
        std::construct_at(std::addressof(m_value), std::forward<U>(value));
        m_has_value = true;
    }

    constexpr auto operator=(const Storage& other) noexcept(
        std::is_nothrow_copy_constructible_v<stored_type> &&
        std::is_nothrow_copy_assignable_v<stored_type> &&
        std::is_nothrow_destructible_v<stored_type>
    ) -> Storage&
        requires std::is_copy_constructible_v<stored_type> &&
                 std::is_copy_assignable_v<stored_type>
    {
        if (this == std::addressof(other)) { return *this; }

        if (other.m_has_value) {
            if (m_has_value) {
                m_value = other.m_value;
            } else {
                if constexpr (std::is_nothrow_copy_constructible_v<stored_type>) {
                    std::construct_at(std::addressof(m_value), other.m_value);
                } else {
                    try {
                        std::construct_at(std::addressof(m_value), other.m_value);
                    } catch (...) {
                        std::construct_at(std::addressof(m_empty), 0);
                        throw;
                    }
                }
                m_has_value = true;
            }
        } else {
            reset();
        }
        return *this;
    }

    constexpr auto operator=(const Storage&) -> Storage&
        requires(!std::is_copy_constructible_v<stored_type> ||
                 !std::is_copy_assignable_v<stored_type>)
    = delete;

    constexpr auto operator=(Storage&& other) noexcept(
        std::is_nothrow_move_constructible_v<stored_type> &&
        std::is_nothrow_move_assignable_v<stored_type> &&
        std::is_nothrow_destructible_v<stored_type>
    ) -> Storage&
        requires std::is_move_constructible_v<stored_type> &&
                 std::is_move_assignable_v<stored_type>
    {
        if (this == std::addressof(other)) { return *this; }

        if (other.m_has_value) {
            if (m_has_value) {
                m_value = std::move(other.m_value);
            } else {
                if constexpr (std::is_nothrow_move_constructible_v<stored_type>) {
                    std::construct_at(std::addressof(m_value), std::move(other.m_value));
                } else {
                    try {
                        std::construct_at(
                            std::addressof(m_value), std::move(other.m_value)
                        );
                    } catch (...) {
                        std::construct_at(std::addressof(m_empty), 0);
                        throw;
                    }
                }
                m_has_value = true;
            }
        } else {
            reset();
        }
        return *this;
    }

    constexpr auto operator=(Storage&&) -> Storage&
        requires(!std::is_move_constructible_v<stored_type> ||
                 !std::is_move_assignable_v<stored_type>)
    = delete;

    template<typename... Args>
        requires std::constructible_from<stored_type, Args&&...>
    constexpr auto emplace(
        Args&&... args
    ) noexcept(std::is_nothrow_constructible_v<stored_type, Args&&...>) -> T& {
        reset();
        if constexpr (std::is_nothrow_constructible_v<stored_type, Args&&...>) {
            std::construct_at(std::addressof(m_value), std::forward<Args>(args)...);
        } else {
            try {
                std::construct_at(std::addressof(m_value), std::forward<Args>(args)...);
            } catch (...) {
                std::construct_at(std::addressof(m_empty), 0);
                throw;
            }
        }
        m_has_value = true;
        return m_value;
    }

    constexpr auto reset() noexcept(std::is_nothrow_destructible_v<stored_type>) -> void {
        if (m_has_value) {
            std::destroy_at(std::addressof(m_value));
            std::construct_at(std::addressof(m_empty), 0);
            m_has_value = false;
        }
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool {
        return m_has_value;
    }

    [[nodiscard]] constexpr auto get() & noexcept -> T& { return m_value; }

    [[nodiscard]] constexpr auto get() const& noexcept -> const T& { return m_value; }

    [[nodiscard]] constexpr auto get() && noexcept -> T&& { return std::move(m_value); }

    [[nodiscard]] constexpr auto get() const&& noexcept -> const T&& {
        return std::move(m_value);
    }

private:
    union {
        stored_type   m_value;
        unsigned char m_empty;
    };

    bool m_has_value = false;
};

template<typename T>
struct option_traits;

template<typename T>
struct option_traits<Option<T>> {
    using value_type = T;
};

template<typename T>
concept option_type =
    requires { typename option_traits<std::remove_cvref_t<T>>::value_type; };

template<typename T>
using option_value_t = typename option_traits<std::remove_cvref_t<T>>::value_type;

template<typename F, typename Arg>
concept option_invocable =
    std::invocable<F, Arg> && option_type<std::invoke_result_t<F, Arg>>;

template<typename T>
using option_object_t = std::remove_reference_t<T>;

} // namespace details

template<typename T>
class Option {
    static_assert(
        !std::is_rvalue_reference_v<T>,
        "Option<T&&> is not supported; use Option<T&> or Option<const T&>"
    );
    static_assert(
        std::is_lvalue_reference_v<T> || std::is_object_v<T>,
        "Option requires an object or lvalue-reference value type"
    );
    static_assert(
        std::is_lvalue_reference_v<T> || !std::is_abstract_v<T>,
        "Option's value type cannot be abstract"
    );

    using storage_type = details::Storage<T>;
    using object_type = std::remove_reference_t<T>;

    template<typename U>
    using remove_cvref_t = std::remove_cvref_t<U>;

    template<typename U>
    static constexpr bool is_option_or_none_v = std::same_as<remove_cvref_t<U>, Option> ||
                                                std::same_as<remove_cvref_t<U>, none_t>;

    template<typename U>
    static constexpr bool storage_constructible_v =
        std::constructible_from<storage_type, U&&>;

    storage_type m_storage;

    [[noreturn]] static auto panic(const char* message) -> void {
        JF_PANIC(message);
        std::terminate();
    }

    template<typename U>
    using map_value_t = std::remove_cvref_t<std::invoke_result_t<U, T&>>;

public:
    using value_type = T;

    constexpr Option() noexcept = default;

    constexpr Option(none_t) noexcept {}

    template<typename U>
        requires(!is_option_or_none_v<U> && storage_constructible_v<U>)
    constexpr explicit Option(
        U&& value
    ) noexcept(std::is_nothrow_constructible_v<storage_type, U&&>)
        : m_storage(std::forward<U>(value)) {}

    constexpr Option(
        const Option& other
    ) noexcept(std::is_nothrow_copy_constructible_v<storage_type>)
        requires std::is_copy_constructible_v<storage_type>
        : m_storage(other.m_storage) {}

    constexpr Option(const Option&)
        requires(!std::is_copy_constructible_v<storage_type>)
    = delete;

    constexpr Option(Option&& other) noexcept(
        std::is_nothrow_move_constructible_v<storage_type> &&
        noexcept(other.m_storage.reset())
    )
        requires std::is_move_constructible_v<storage_type>
        : m_storage(std::move(other.m_storage)) {
        other.m_storage.reset();
    }

    constexpr Option(Option&&)
        requires(!std::is_move_constructible_v<storage_type>)
    = delete;

    constexpr ~Option() = default;

    constexpr auto
    operator=(const Option& other) noexcept(noexcept(m_storage = other.m_storage))
        -> Option&
        requires requires { m_storage = other.m_storage; }
    {
        if (this != std::addressof(other)) { m_storage = other.m_storage; }
        return *this;
    }

    constexpr auto operator=(const Option&) -> Option&
        requires(!requires(const storage_type& lhs, const storage_type& rhs) {
                    lhs = rhs;
                })
    = delete;

    constexpr auto operator=(Option&& other) noexcept(
        noexcept(m_storage = std::move(other.m_storage)) &&
        noexcept(other.m_storage.reset())
    ) -> Option&
        requires requires { m_storage = std::move(other.m_storage); }
    {
        if (this != std::addressof(other)) {
            m_storage = std::move(other.m_storage);
            other.m_storage.reset();
        }
        return *this;
    }

    constexpr auto operator=(Option&&) -> Option&
        requires(!requires(storage_type& lhs, storage_type&& rhs) {
                    lhs = std::move(rhs);
                })
    = delete;

    constexpr auto operator=(none_t) noexcept -> Option& {
        reset();
        return *this;
    }

    template<typename U>
        requires(
            !is_option_or_none_v<U> && storage_constructible_v<U> &&
            (std::is_lvalue_reference_v<T> || std::is_assignable_v<object_type&, U &&> ||
             std::is_move_constructible_v<object_type>)
        )
    constexpr auto operator=(U&& value) -> Option& {
        if constexpr (std::is_lvalue_reference_v<T>) {
            m_storage.emplace(std::forward<U>(value));
        } else if (this->has_value() && std::is_assignable_v<object_type&, U&&>) {
            m_storage.get() = std::forward<U>(value);
        } else {
            // Construct before replacing the current value. This gives value
            // assignment a strong guarantee when construction fails.
            Option replacement(std::forward<U>(value));
            swap(replacement);
        }
        return *this;
    }

    constexpr auto reset() noexcept(noexcept(m_storage.reset())) -> void {
        m_storage.reset();
    }

    template<typename... Args>
        requires(!std::is_lvalue_reference_v<T>) &&
                std::constructible_from<object_type, Args&&...>
    constexpr auto emplace(Args&&... args) noexcept(
        noexcept(m_storage.emplace(std::forward<Args>(args)...))
    ) -> T& {
        return m_storage.emplace(std::forward<Args>(args)...);
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool {
        return m_storage.has_value();
    }

    [[nodiscard]] constexpr auto value() & -> T& { return unwrap(); }

    [[nodiscard]] constexpr auto value() const& -> const T& { return unwrap(); }

    constexpr auto value() && -> T
        requires(!std::is_reference_v<T>)
    {
        return std::move(*this).unwrap();
    }

    [[nodiscard]] constexpr auto value() const&& -> const T&& {
        return std::move(*this).unwrap();
    }

    constexpr auto unwrap() & -> T& {
        if (!this->has_value()) { panic("called `Option::unwrap()` on a `None` value"); }
        return m_storage.get();
    }

    [[nodiscard]] constexpr auto unwrap() const& -> const T& {
        if (!this->has_value()) { panic("called `Option::unwrap()` on a `None` value"); }
        return m_storage.get();
    }

    constexpr auto unwrap() && -> T
        requires(!std::is_reference_v<T>)
    {
        if (!this->has_value()) { panic("called `Option::unwrap()` on a `None` value"); }
        T result = std::move(m_storage).get();
        m_storage.reset();
        return result;
    }

    [[nodiscard]] constexpr auto unwrap() const&& -> const T&& {
        if (!this->has_value()) { panic("called `Option::unwrap()` on a `None` value"); }
        return std::move(m_storage).get();
    }

    constexpr auto expect(const char* message) & -> T& {
        if (!this->has_value()) { panic(message); }
        return m_storage.get();
    }

    [[nodiscard]] constexpr auto expect(const char* message) const& -> const T& {
        if (!this->has_value()) { panic(message); }
        return m_storage.get();
    }

    constexpr auto expect(const char* message) && -> T
        requires(!std::is_reference_v<T>)
    {
        if (!this->has_value()) { panic(message); }
        T result = std::move(m_storage).get();
        m_storage.reset();
        return result;
    }

    constexpr auto unwrap_unchecked() & -> T& { return m_storage.get(); }

    [[nodiscard]] constexpr auto unwrap_unchecked() const& -> const T& {
        return m_storage.get();
    }

    constexpr auto unwrap_unchecked() && -> T
        requires(!std::is_reference_v<T>)
    {
        T result = std::move(m_storage).get();
        m_storage.reset();
        return result;
    }

    [[nodiscard]] constexpr auto unwrap_unchecked() const&& -> const T&& {
        return std::move(m_storage).get();
    }

    template<typename U>
        requires requires(const T& value, const U& expected) { value == expected; }
    [[nodiscard]] constexpr auto contains(const U& expected) const -> bool {
        return this->has_value() && (this->unwrap_unchecked() == expected);
    }

    template<typename U = T>
        requires std::is_copy_constructible_v<object_type> &&
                 std::constructible_from<T, U&&>
    [[nodiscard]] constexpr auto unwrap_or(U&& fallback) const& -> T {
        if (this->has_value()) { return this->unwrap_unchecked(); }
        return std::forward<U>(fallback);
    }

    template<typename U = T>
        requires std::constructible_from<T, U&&>
    constexpr auto unwrap_or(U&& fallback) && -> T
        requires(!std::is_reference_v<T>)
    {
        if (this->has_value()) {
            T result = std::move(m_storage).get();
            m_storage.reset();
            return result;
        }
        return std::forward<U>(fallback);
    }

    template<typename F>
        requires std::invocable<F> && std::constructible_from<T, std::invoke_result_t<F>>
    [[nodiscard]] constexpr auto unwrap_or_else(F&& fallback) const& -> T {
        if (this->has_value()) { return this->unwrap_unchecked(); }
        return std::invoke(std::forward<F>(fallback));
    }

    template<typename F>
        requires std::invocable<F> && std::constructible_from<T, std::invoke_result_t<F>>
                                  constexpr auto unwrap_or_else(F&& fallback) && -> T
                     requires(!std::is_reference_v<T>)
    {
        if (this->has_value()) {
            T result = std::move(m_storage).get();
            m_storage.reset();
            return result;
        }
        return std::invoke(std::forward<F>(fallback));
    }

    [[nodiscard]] constexpr auto unwrap_or_default() const& -> T
        requires std::default_initializable<T> &&
                 std::is_copy_constructible_v<object_type>
    {
        return unwrap_or(T{});
    }

    constexpr auto unwrap_or_default() && -> T
        requires(!std::is_reference_v<T>) && std::default_initializable<T>
    {
        return std::move(*this).unwrap_or(T{});
    }

    template<typename F>
        requires std::invocable<F&&, T&> &&
                 (!std::is_void_v<std::invoke_result_t<F &&, T&>>)
    constexpr auto
    map(F&& function) & -> Option<std::remove_cvref_t<std::invoke_result_t<F&&, T&>>> {
        using U = std::remove_cvref_t<std::invoke_result_t<F&&, T&>>;
        if (this->has_value()) {
            return Option<U>(std::invoke(std::forward<F>(function), this->unwrap()));
        }
        return Option<U>();
    }

    template<typename F>
        requires std::invocable<F&&, const T&> &&
                 (!std::is_void_v<std::invoke_result_t<F &&, const T&>>)
    [[nodiscard]] constexpr auto map(
        F&& function
    ) const& -> Option<std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>> {
        using U = std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>;
        if (this->has_value()) {
            return Option<U>(std::invoke(std::forward<F>(function), this->unwrap()));
        }
        return Option<U>();
    }

    template<typename F>
        requires(!std::is_reference_v<T>) && std::invocable<F&&, T&&> &&
                (!std::is_void_v<std::invoke_result_t<F &&, T &&>>)
    constexpr auto
    map(F&& function) && -> Option<std::remove_cvref_t<std::invoke_result_t<F&&, T&&>>> {
        using U = std::remove_cvref_t<std::invoke_result_t<F&&, T&&>>;
        if (this->has_value()) {
            auto result = Option<U>(
                std::invoke(std::forward<F>(function), std::move(m_storage).get())
            );
            m_storage.reset();
            return result;
        }
        return Option<U>();
    }

    template<typename D, typename F>
        requires(!std::invocable<D, const T&>) && std::invocable<F&&, const T&> &&
                std::constructible_from<D, std::invoke_result_t<F&&, const T&>>
    [[nodiscard]] constexpr auto map_or(
        D&& default_value,
        F&& function
    ) const& -> std::remove_cvref_t<std::invoke_result_t<F&&, const T&>> {
        using R = std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>;
        if (this->has_value()) {
            return std::invoke(std::forward<F>(function), this->unwrap());
        }
        return R(std::forward<D>(default_value));
    }

    template<typename DefaultF, typename F>
        requires std::invocable<DefaultF&&> && std::invocable<F&&, const T&> &&
                 std::constructible_from<
                     std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>,
                     std::invoke_result_t<DefaultF&&>>
    [[nodiscard]] constexpr auto map_or_else(
        DefaultF&& default_function,
        F&&        function
    ) const& -> std::remove_cvref_t<std::invoke_result_t<F&&, const T&>> {
        using R = std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>;
        if (this->has_value()) {
            return std::invoke(std::forward<F>(function), this->unwrap());
        }
        return R(std::invoke(std::forward<DefaultF>(default_function)));
    }

    template<typename F>
        requires details::option_invocable<F&&, T&>
    constexpr auto
    and_then(F&& function) & -> std::remove_cvref_t<std::invoke_result_t<F&&, T&>> {
        using R = std::remove_cvref_t<std::invoke_result_t<F&&, T&>>;
        if (this->has_value()) {
            return std::invoke(std::forward<F>(function), this->unwrap());
        }
        return R();
    }

    template<typename F>
        requires details::option_invocable<F&&, const T&>
    [[nodiscard]] constexpr auto and_then(
        F&& function
    ) const& -> std::remove_cvref_t<std::invoke_result_t<F&&, const T&>> {
        using R = std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>;
        if (this->has_value()) {
            return std::invoke(std::forward<F>(function), this->unwrap());
        }
        return R();
    }

    template<typename F>
        requires(!std::is_reference_v<T>) && details::option_invocable<F&&, T&&>
    constexpr auto
    and_then(F&& function) && -> std::remove_cvref_t<std::invoke_result_t<F&&, T&&>> {
        using R = std::remove_cvref_t<std::invoke_result_t<F&&, T&&>>;
        if (this->has_value()) {
            auto result =
                std::invoke(std::forward<F>(function), std::move(m_storage).get());
            m_storage.reset();
            return result;
        }
        return R();
    }

    // Compatibility overload for the original API, which allowed the output
    // value type to be supplied explicitly: option.and_then<U>(function).
    template<typename U, typename F>
        requires(!std::invocable<U, const T&>) && std::invocable<F&&, const T&> &&
                std::same_as<
                    std::remove_cvref_t<std::invoke_result_t<F&&, const T&>>,
                    Option<U>>
    [[nodiscard]] constexpr auto and_then(F&& function) const& -> Option<U> {
        if (this->has_value()) {
            return std::invoke(std::forward<F>(function), this->unwrap());
        }
        return Option<U>();
    }

    template<typename F>
        requires std::invocable<F&&> &&
                 std::same_as<std::remove_cvref_t<std::invoke_result_t<F&&>>, Option<T>>
    [[nodiscard]] constexpr auto or_else(F&& function) const& -> Option<T> {
        if (this->has_value()) { return *this; }
        return std::invoke(std::forward<F>(function));
    }

    template<typename F>
        requires(!std::is_reference_v<T>) && std::invocable<F&&> &&
                std::same_as<std::remove_cvref_t<std::invoke_result_t<F&&>>, Option<T>>
    constexpr auto or_else(F&& function) && -> Option<T> {
        if (this->has_value()) { return std::move(*this); }
        return std::invoke(std::forward<F>(function));
    }

    template<typename U>
        requires std::is_copy_constructible_v<details::Storage<U>>
    [[nodiscard]] constexpr auto and_(const Option<U>& other) const& -> Option<U> {
        return this->has_value() ? other : Option<U>();
    }

    template<typename U>
    [[nodiscard]] constexpr auto and_(Option<U>&& other) const& -> Option<U> {
        return this->has_value() ? std::move(other) : Option<U>();
    }

    template<typename U>
        requires std::is_copy_constructible_v<details::Storage<U>>
    [[nodiscard]] constexpr auto and_(const Option<U>& other) && -> Option<U> {
        const bool keep_other = this->has_value();
        this->reset();
        return keep_other ? other : Option<U>();
    }

    template<typename U>
    constexpr auto and_(Option<U>&& other) && -> Option<U> {
        const bool keep_other = this->has_value();
        this->reset();
        return keep_other ? std::move(other) : Option<U>();
    }

    template<typename U = T>
        requires std::is_copy_constructible_v<storage_type>
    [[nodiscard]] constexpr auto or_(const Option<T>& other) const& -> Option<T> {
        return this->has_value() ? *this : other;
    }

    [[nodiscard]] constexpr auto or_(Option<T>&& other) const& -> Option<T> {
        return this->has_value() ? *this : std::move(other);
    }

    constexpr auto or_(const Option<T>& other) && -> Option<T>
        requires std::is_copy_constructible_v<storage_type>
    {
        if (this->has_value()) { return std::move(*this); }
        this->reset();
        return other;
    }

    constexpr auto or_(Option<T>&& other) && -> Option<T> {
        if (this->has_value()) { return std::move(*this); }
        this->reset();
        return std::move(other);
    }

    template<typename U = T>
        requires std::is_copy_constructible_v<storage_type>
    [[nodiscard]] constexpr auto xor_(const Option<T>& other) const& -> Option<T> {
        if (this->has_value() != other.has_value()) {
            return this->has_value() ? *this : other;
        }
        return Option<T>();
    }

    [[nodiscard]] constexpr auto xor_(Option<T>&& other) const& -> Option<T> {
        if (this->has_value()) { return other.has_value() ? Option<T>() : *this; }
        return other.has_value() ? std::move(other) : Option<T>();
    }

    constexpr auto xor_(const Option<T>& other) && -> Option<T>
        requires std::is_copy_constructible_v<storage_type>
    {
        const bool result_has_value = this->has_value() != other.has_value();
        if (result_has_value && this->has_value()) { return std::move(*this); }
        this->reset();
        return result_has_value ? other : Option<T>();
    }

    constexpr auto xor_(Option<T>&& other) && -> Option<T> {
        const bool this_has_value = this->has_value();
        const bool other_has_value = other.has_value();
        if (this_has_value && !other_has_value) { return std::move(*this); }
        if (!this_has_value && other_has_value) { return std::move(other); }
        this->reset();
        return Option<T>();
    }

    template<typename F>
        requires std::invocable<F&&, const T&>
    [[nodiscard]] constexpr auto inspect(F&& function) const& -> Option<T>
        requires std::is_copy_constructible_v<storage_type>
    {
        if (this->has_value()) { std::invoke(std::forward<F>(function), this->unwrap()); }
        return *this;
    }

    template<typename F>
        requires(!std::is_reference_v<T>) && std::invocable<F&&, T&>
    constexpr auto inspect(F&& function) && -> Option<T> {
        if (this->has_value()) { std::invoke(std::forward<F>(function), this->unwrap()); }
        return std::move(*this);
    }

    [[nodiscard]] constexpr auto as_ref() const& -> Option<const object_type&>
        requires(!std::is_reference_v<T>) && std::is_copy_constructible_v<storage_type>
    {
        if (this->has_value()) { return Option<const object_type&>(this->unwrap()); }
        return Option<const object_type&>();
    }

    [[nodiscard]] constexpr auto as_mut() & -> Option<object_type&>
        requires(!std::is_reference_v<T>)
    {
        if (this->has_value()) { return Option<object_type&>(this->unwrap()); }
        return Option<object_type&>();
    }

    [[nodiscard]] constexpr auto take() -> Option {
        if (!this->has_value()) { return Option(); }
        if constexpr (std::is_reference_v<T>) {
            Option result(*this);
            this->reset();
            return result;
        } else {
            return Option(std::move(*this).unwrap());
        }
    }

    template<typename... Args>
        requires(!std::is_lvalue_reference_v<T>) &&
                std::constructible_from<object_type, Args&&...>
    constexpr auto get_or_insert(Args&&... args) -> T& {
        if (!this->has_value()) { emplace(std::forward<Args>(args)...); }
        return unwrap_unchecked();
    }

    template<typename U>
        requires std::is_lvalue_reference_v<T> && storage_constructible_v<U>
    constexpr auto get_or_insert(U&& value) -> T {
        if (!this->has_value()) { m_storage.emplace(std::forward<U>(value)); }
        return unwrap_unchecked();
    }

    template<typename F>
        requires std::invocable<F&&> &&
                 std::constructible_from<object_type, std::invoke_result_t<F&&>>
    constexpr auto get_or_insert_with(F&& function) -> T& {
        if (!this->has_value()) { emplace(std::invoke(std::forward<F>(function))); }
        return unwrap_unchecked();
    }

    template<typename F>
        requires std::is_lvalue_reference_v<T> && std::invocable<F&&> &&
                 storage_constructible_v<std::invoke_result_t<F&&>>
    constexpr auto get_or_insert_with(F&& function) -> T {
        if (!this->has_value()) {
            m_storage.emplace(std::invoke(std::forward<F>(function)));
        }
        return unwrap_unchecked();
    }

    constexpr auto get_or_insert_default() -> T&
        requires(!std::is_lvalue_reference_v<T>) &&
                std::default_initializable<object_type>
    {
        return get_or_insert();
    }

    template<typename U>
        requires std::constructible_from<storage_type, U&&>
    constexpr auto replace(U&& value) -> Option {
        Option replacement(std::forward<U>(value));
        Option previous = take();
        try {
            if constexpr (std::is_reference_v<T>) {
                m_storage.emplace(replacement.unwrap());
            } else {
                m_storage.emplace(std::move(replacement).unwrap());
            }
        } catch (...) {
            if (previous.has_value()) {
                if constexpr (std::is_reference_v<T>) {
                    m_storage.emplace(previous.unwrap());
                } else {
                    m_storage.emplace(std::move(previous).unwrap());
                }
            }
            throw;
        }
        return previous;
    }

    friend constexpr auto swap(Option& lhs, Option& rhs) noexcept(
        std::is_nothrow_move_constructible_v<storage_type> &&
        std::is_nothrow_move_assignable_v<storage_type>
    ) -> void
        requires std::is_move_constructible_v<storage_type> &&
                 std::is_move_assignable_v<storage_type>
    {
        if (std::addressof(lhs) == std::addressof(rhs)) { return; }
        Option temporary(std::move(lhs));
        lhs = std::move(rhs);
        rhs = std::move(temporary);
    }

    friend constexpr auto operator==(const Option& lhs, const Option& rhs)
        requires requires(const T& left, const T& right) { left == right; }
    {
        if (lhs.has_value() != rhs.has_value()) { return false; }
        return !lhs.has_value() || (lhs.unwrap_unchecked() == rhs.unwrap_unchecked());
    }

    friend constexpr auto operator!=(const Option& lhs, const Option& rhs)
        requires requires(const T& left, const T& right) { left == right; }
    {
        return !(lhs == rhs);
    }

    friend constexpr auto operator==(const Option& option, none_t) noexcept -> bool {
        return !option.has_value();
    }

    friend constexpr auto operator!=(const Option& option, none_t) noexcept -> bool {
        return option.has_value();
    }

    constexpr auto operator*() & -> T& { return unwrap_unchecked(); }

    [[nodiscard]] constexpr auto operator*() const& -> const T& {
        return unwrap_unchecked();
    }

    constexpr auto operator*() && -> T&&
        requires(!std::is_reference_v<T>)
    {
        return std::move(m_storage).get();
    }

    [[nodiscard]] constexpr auto operator*() const&& -> const T&& {
        return std::move(m_storage).get();
    }

    [[nodiscard]] constexpr auto operator->() -> std::add_pointer_t<T> {
        return std::addressof(unwrap());
    }

    [[nodiscard]] constexpr auto operator->() const -> std::add_pointer_t<const T> {
        return std::addressof(unwrap());
    }

    static constexpr auto none() noexcept -> Option { return Option(); }

    template<typename U>
        requires(!std::is_lvalue_reference_v<T>) && std::constructible_from<T, U&&>
    static constexpr auto some(U&& value) -> Option {
        return Option(std::forward<U>(value));
    }

    static constexpr auto None() noexcept -> Option { return Option(); }
};

template<typename T>
Option(T&&) -> Option<T>;

} // namespace option

using option::Option;

template<typename T>
[[nodiscard]] constexpr auto Some(T&& value) -> Option<std::decay_t<T>> {
    return Option<std::decay_t<T>>(std::forward<T>(value));
}

} // namespace JadeFrame
