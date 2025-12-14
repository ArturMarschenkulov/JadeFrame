#pragma once

#include <utility>
#include <concepts>
#include <type_traits>
#include <new>

#include "JadeFrame/types.h"
#include "JadeFrame/utils/assert.h"

namespace JadeFrame {

/*
    NOTE: This is class is mainly modelled after Rust's Option<T>.
    NOTE: Thus member function should have the same naming convention, though provide
   C++-like wrapper member functions. NOTE: Think about maybe renaming this class (Rust,
   Ocaml) to Maybe (Haskell) or Optional (C++, Swift)

    TODO: Consider whether the ref-qualified "const&&"" overloads are needed. Most likely
   not.
*/

// https://www.foonathan.net/2018/07/optional-reference/
namespace option {

namespace details {
template<typename T>
class Storage;

template<typename T>
    requires std::is_lvalue_reference_v<T>
class Storage<T> {
public:
    constexpr Storage() = default;
    constexpr ~Storage() = default;
    constexpr Storage(const Storage&) = default;
    constexpr auto operator=(const Storage&) -> Storage& = default;
    constexpr Storage(Storage&&) = default;
    constexpr auto operator=(Storage&&) -> Storage& = default;

    constexpr explicit Storage(const T& v)
        : m_pointer(&v) {}

    template<typename U>
    constexpr explicit Storage(U&& v)
        requires std::same_as<std::remove_reference_t<U>, std::remove_reference_t<T>> &&
                 std::is_lvalue_reference_v<U&&>
        : m_pointer(std::addressof(std::forward<U>(v))) {}

    [[nodiscard]] constexpr auto get() const& -> const T& { return *m_pointer; }

public:
    [[nodiscard]] auto has_value() const -> bool { return m_pointer != nullptr; }

    auto reset() -> void { m_pointer = nullptr; }

public:
    std::remove_reference_t<T>* m_pointer = nullptr;
};

template<typename T>
    requires(!std::is_lvalue_reference_v<T>)
class Storage<T> {
    using U = std::remove_cv_t<T>;

public:
    constexpr Storage()
        requires(std::is_trivially_destructible_v<T>)
    = default;

    constexpr ~Storage() {
        if (m_has_value) { this->reset(); }
    }

    constexpr Storage(const Storage&)
        requires(std::is_trivially_copy_constructible_v<T>)
    = default;
    constexpr auto operator=(const Storage& o) -> Storage&
        requires(std::is_trivially_copy_assignable_v<T>)
    = default;
    constexpr Storage(Storage&&)
        requires(std::is_trivially_move_constructible_v<T>)
    = default;
    constexpr auto operator=(Storage&&) -> Storage&
        requires(std::is_trivially_move_assignable_v<T>)
    = default;

    constexpr Storage(const Storage& o) noexcept(std::is_nothrow_copy_constructible_v<U>)
        requires(!std::is_trivially_copy_constructible_v<T>)
    {
        if (o.m_has_value) {
            ::new (this->raw_ptr()) U(*o.raw_ptr());
            m_has_value = true;
        }
    }

    constexpr auto operator=(const Storage& o) -> Storage&
        requires(
            !std::is_trivially_copy_assignable_v<T> ||
            !std::is_trivially_copy_constructible_v<T>
        )
    {
        if (this == &o) { return *this; }
        if (m_has_value) { this->reset(); }
        m_has_value = o.m_has_value;
        if (m_has_value) { new (this->raw_ptr()) U(*o.raw_ptr()); }
        return *this;
    }

    constexpr Storage(Storage&& o) noexcept(std::is_nothrow_move_constructible_v<U>)
        requires(!std::is_trivially_move_constructible_v<T>)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) {
            ::new (this->raw_ptr()) U(std::move(*o.raw_ptr()));
            o.reset();
        }
    }

    constexpr auto operator=(Storage&& o) noexcept(
        std::is_nothrow_move_constructible_v<U> && std::is_nothrow_move_assignable_v<U>
    ) -> Storage&
        requires(
            !std::is_trivially_move_assignable_v<T> ||
            !std::is_trivially_move_constructible_v<T>
        )
    {
        if (this == &o) { return *this; }

        if (m_has_value && o.m_has_value) {
            *this->raw_ptr() = std::move(*o.raw_ptr());
            o.reset();
        } else if (m_has_value && !o.m_has_value) {
            this->reset();
        } else if (!m_has_value && o.m_has_value) {
            ::new (this->raw_ptr()) U(std::move(*o.raw_ptr()));
            m_has_value = true;
            o.reset();
        }
        return *this;
    }

    constexpr explicit Storage(const T& v)
        : m_has_value(true) {
        ::new (this->raw_ptr()) U(v);
    }

    constexpr explicit Storage(T&& v)
        : m_has_value(true) {
        ::new (this->raw_ptr()) U(std::move(v));
    }

    [[nodiscard]] constexpr auto get() & -> T& { return *this->raw_ptr(); }

    [[nodiscard]] constexpr auto get() const& -> const T& { return *this->raw_ptr(); }

    [[nodiscard]] constexpr auto get() && -> T { return std::move(*this->raw_ptr()); }

    template<typename... Args>
    constexpr auto emplace(Args&&... args) -> void {
        this->reset();
        ::new (this->raw_ptr()) U(std::forward<Args>(args)...);
        m_has_value = true;
    }

    constexpr auto reset() -> void {
        if (m_has_value) {
            this->raw_ptr()->~U();
            m_has_value = false;
        }
    }

    // constexpr auto has_value() const -> bool { return m_has_value; }

public:
    [[nodiscard]] auto has_value() const -> bool { return m_has_value; }

public:
    alignas(U) std::byte m_storage[sizeof(U)]{};
    bool m_has_value = false;

private:
    constexpr auto ptr() -> T* { return this->raw_ptr(); }

    constexpr auto ptr() const -> const T* { return this->raw_ptr(); }

    constexpr auto raw_ptr() -> U* {
        // std::launder for strict aliasing safety
        return std::launder(reinterpret_cast<U*>(&m_storage[0]));
    }

    constexpr auto raw_ptr() const -> const U* {
        return std::launder(reinterpret_cast<const U*>(&m_storage[0]));
    }
};
} // namespace details

template<typename T>
class Option {
    static_assert(
        !std::is_rvalue_reference_v<T>,
        "`Option<T&&> is not supported, use Option<T&> or Option<const T&."
    );

public:
    constexpr Option()
        : m_storage() {}

    constexpr ~Option()
        requires(!std::is_destructible_v<T>)
    = delete;

    constexpr ~Option() = default;

    constexpr Option(const Option& o)
        : m_storage(o.m_storage) {}

    constexpr auto operator=(const Option& o) -> Option& {
        if (this == &o) { return *this; }
        m_storage = o.m_storage;
        return *this;
    }

    constexpr Option(Option&& o) noexcept
        : m_storage(std::move(o.m_storage)) {}

    constexpr auto operator=(Option&& o) noexcept -> Option& {
        if (this == &o) { return *this; }
        m_storage = std::move(o.m_storage);
        return *this;
    }

    constexpr explicit Option(const T& v)
        : m_storage(v) {}

    constexpr explicit Option(T&& v)
        requires(!std::is_lvalue_reference_v<T>)
        : m_storage(std::move(v)) {}

    constexpr auto operator==(const Option& o) const noexcept -> bool {
        if (this->is_some() && o.is_some()) {
            return this->unwrap_unchecked() == o.unwrap_unchecked();
        } else if (!this->is_some() && !o.is_some()) {
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] constexpr auto is_some() const -> bool { return m_storage.has_value(); }

    [[nodiscard]] constexpr auto is_none() const -> bool { return !this->is_some(); }

    constexpr auto unwrap() & -> T& {
        if (!this->is_some()) {
            JF_PANIC("called `Option::unwrap() & -> T&` on a `None` value");
            std::terminate();
        }
        return m_storage.get();
    }

    [[nodiscard]] constexpr auto unwrap() const& -> const T& {
        if (!this->is_some()) {
            JF_PANIC("called `Option::unwrap() const& -> const T&` on a `None` value");
            std::terminate();
        }
        return m_storage.get();
    }

    constexpr auto unwrap() && -> T
        requires(!std::is_reference_v<T>)
    {
        if (!this->is_some()) {
            JF_PANIC("called `Option::unwrap() -> T` on a `None` value");
            std::terminate();
        }
        T value = std::move(m_storage.get());
        m_storage.reset();
        return value;
    }

    constexpr auto unwrap_unchecked() & -> T& { return const_cast<T&>(m_storage.get()); }

    [[nodiscard]] constexpr auto unwrap_unchecked() const& -> const T& {
        return m_storage.get();
    }

    constexpr auto unwrap_unchecked() && -> T
        requires(!std::is_reference_v<T>)
    {
        T value = std::move(m_storage.get());
        m_storage.reset();
        return value;
    }

    constexpr auto operator*() const -> const T& { return this->unwrap(); }

    constexpr auto operator*() -> T& { return this->unwrap(); }

    template<typename U = T>
    [[nodiscard]] constexpr auto and_(const Option<U>& o) const& -> Option<U> {
        if (this->is_some()) {
            return o;
        } else {
            return Option<U>();
        }
    }

    [[nodiscard]] constexpr auto or_(const Option<T>& o) const& -> Option<T> {
        if (this->is_some()) {
            return *this;
        } else {
            return o;
        }
    }

    [[nodiscard]] constexpr auto xor_(const Option<T>& o) const& -> Option<T> {
        if (this->is_some() && !o.is_some()) {
            return *this;
        } else if (!this->is_some() && o.is_some()) {
            return o;
        } else {
            return Option<T>();
        }
    }

    template<typename U = T, typename F>
        requires std::invocable<F, T>
    constexpr auto and_then(F&& func) const& -> Option<U> {
        if (this->is_some()) {
            return func(this->unwrap());
        } else {
            return Option<U>();
        }
    }

    template<typename F>
        requires std::invocable<F> &&
                 std::convertible_to<std::invoke_result_t<F>, Option<T>>
    constexpr auto or_else(F&& func) const& -> Option<T> {
        if (this->is_some()) {
            return *this;
        } else {
            return func();
        }
    }

public: // C++-like wrappers
    auto value() -> T& { return this->unwrap(); }

    [[nodiscard]] auto value() const -> const T& { return this->unwrap(); }

    [[nodiscard]] auto has_value() const -> bool { return this->is_some(); }

private:
    details::Storage<T> m_storage;
};

} // namespace option

using option::Option;
} // namespace JadeFrame
