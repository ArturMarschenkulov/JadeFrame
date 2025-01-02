#pragma once

#include <utility>
#include <concepts>
#include <type_traits>

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
class Storage {
public:
    bool m_has_value;
};

template<typename T>
    requires std::is_lvalue_reference_v<T>
class Storage<T> {
public:
    constexpr Storage() = default;

    constexpr ~Storage() {
        if (m_has_value) { m_pointer = nullptr; }
    }

    constexpr Storage(const Storage& o)
        : m_has_value(o.m_has_value)
        , m_pointer(o.m_pointer) {}

    constexpr auto operator=(const Storage& o) -> Storage& {
        if (this == &o) { return *this; }
        m_has_value = o.m_has_value;
        m_pointer = o.m_pointer;
        return *this;
    }

    constexpr Storage(Storage&& o) noexcept
        : m_has_value(o.m_has_value)
        , m_pointer(o.m_pointer) {
        o.m_has_value = false;
        o.m_pointer = nullptr;
    }

    constexpr auto operator=(Storage&& o) noexcept -> Storage& {
        m_has_value = o.m_has_value;
        m_pointer = o.m_pointer;
        o.m_has_value = false;
        o.m_pointer = nullptr;
        return *this;
    }

    constexpr explicit Storage(const T& v)
        : m_has_value(true)
        , m_pointer(&v) {}

    constexpr explicit Storage(T&& v)
        requires(std::is_lvalue_reference_v<T>)
        : m_has_value(true)
        , m_pointer(&v) {}

    [[nodiscard]] constexpr auto get() const& -> const T& { return *m_pointer; }

public:
    bool                        m_has_value = false;
    std::remove_reference_t<T>* m_pointer = nullptr;
};

template<typename T>
    requires(!std::is_lvalue_reference_v<T>)
class Storage<T> {
public:
    constexpr Storage()
        : m_has_value(false) {}

    constexpr ~Storage() {
        if (m_has_value) { reinterpret_cast<T&>(m_storage).~T(); }
    }

    constexpr Storage(const Storage& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) {
            new (&m_storage) T(reinterpret_cast<const T&>(o.m_storage));
        }
    }

    constexpr Storage(Storage&& o) noexcept
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) {
            T rv = std::move(reinterpret_cast<T&>(o.m_storage));
            reinterpret_cast<T&>(o.m_storage).~T();
            o.m_has_value = false;
            new (&m_storage) T(rv);
        }
    }

    constexpr auto operator=(Storage&& o) noexcept -> Storage& {
        if (m_has_value) { reinterpret_cast<T&>(m_storage).~T(); }
        m_has_value = o.m_has_value;
        if (o.m_has_value) {
            T rv = std::move(reinterpret_cast<T&>(o.m_storage));
            reinterpret_cast<T&>(o.m_storage).~T();
            o.m_has_value = false;
            new (&m_storage) T(rv);
        }
        return *this;
    }

    constexpr explicit Storage(const T& v)
        : m_has_value(true) {
        new (&m_storage) T(v);
    }

    constexpr explicit Storage(T&& v)
        : m_has_value(true) {
        new (&m_storage) T(std::forward<T>(v));
    }

    [[nodiscard]] constexpr auto get() const& -> const T& {
        return reinterpret_cast<const T&>(m_storage);
    }

    [[nodiscard]] constexpr auto get() & -> T& { return reinterpret_cast<T&>(m_storage); }

    [[nodiscard]] constexpr auto get() && -> T {
        return std::move(reinterpret_cast<T&>(m_storage));
    }

    [[nodiscard]] constexpr auto get() const&& -> const T {
        return std::move(reinterpret_cast<const T&>(m_storage));
    }

    // constexpr auto has_value() const -> bool { return m_has_value; }

public:
    alignas(T) u8 m_storage[sizeof(T)]{};
    bool m_has_value;
};
} // namespace details

template<typename T>
class Option {
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
        if (this->is_some()) { this->unwrap_unchecked().~T(); }
        m_storage = o.m_storage;
        return *this;
    }

    constexpr Option(Option&& o) noexcept
        : m_storage(std::forward<Option>(o).m_storage) {
        m_storage.m_has_value = true;
    }

    constexpr auto operator=(Option&& o) noexcept -> Option& {
        if (this->is_some()) { this->unwrap_unchecked().~T(); }
        m_storage = std::forward<details::Storage<T>>(o.m_storage);
        return *this;
    }

    constexpr explicit Option(const T& v)
        : m_storage(v) {}

    constexpr explicit Option(T&& v)
        requires(!std::is_lvalue_reference_v<T>)
        : m_storage(std::forward<T>(std::move(v))) {}

    constexpr auto operator==(const Option& o) const noexcept -> bool {
        if (this->is_some() && o.is_some()) {
            return this->unwrap() == o.unwrap();
        } else if (!this->is_some() && !o.is_some()) {
            return true;
        } else {
            return false;
        }
    }

    constexpr auto operator==(const T& v) const noexcept -> bool {
        if (this->is_some()) {
            return this->unwrap() == v;
        } else {
            return false;
        }
    }

    [[nodiscard]] constexpr auto is_some() const -> bool { return m_storage.m_has_value; }

    [[nodiscard]] constexpr auto is_none() const -> bool { return !this->is_some(); }

    constexpr auto unwrap() & -> T& {
        if (this->is_some()) { return m_storage.get(); }
        JF_PANIC("called `Option::unwrap() & -> T&` on a `None` value");
        std::terminate();
    }

    [[nodiscard]] constexpr auto unwrap() const& -> const T& {
        if (this->is_some()) { return m_storage.get(); }
        JF_PANIC("called `Option::unwrap() const& -> const T&` on a `None` value");
        std::terminate();
    }

    constexpr auto unwrap() && -> T { return this->release(); }

    constexpr auto unwrap_unchecked() & -> T& { return const_cast<T&>(m_storage.get()); }

    [[nodiscard]] constexpr auto unwrap_unchecked() const& -> const T& {
        return m_storage.get();
    }

    constexpr auto unwrap_unchecked() && -> T { return this->release_unchecked(); }

    constexpr auto release_unchecked() -> T {
        T released_value = std::move(this->unwrap_unchecked());
        this->unwrap_unchecked().~T();
        m_storage.m_has_value = false;
        return released_value;
    }

    constexpr auto release() -> T {
        if (this->is_some()) {
            T released_value = std::move(this->unwrap());
            this->unwrap().~T();
            m_storage.m_has_value = false;
            return released_value;
        }
        JF_PANIC("called `Option::release() -> T` on a `None` value");
        std::terminate();
    }

    constexpr auto operator*() const -> const T& { return this->unwrap(); }

    constexpr auto operator*() -> T& { return this->unwrap(); }

    // constexpr auto operator->() const -> const T* { return
    // &std::remove_reference_t<T>(this->unwrap()); } constexpr auto operator->() -> T* {
    // return &std::remove_reference_t<T>(this->unwrap()); }

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
