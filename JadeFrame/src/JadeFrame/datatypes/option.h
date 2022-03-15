#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/utils/assert.h"

#include <utility>
#include <array>
#include <concepts>
#include <optional>

/*
    NOTE: This is class is mainly modelled after Rust's Option<T>.
    NOTE: Think about maybe renaming this class (Rust, Ocaml) to Maybe (Haskell) or Optional (C++, Swift)
*/

namespace JadeFrame {

/*
    This class is mainly modelled afer Rust's Option<T>.
*/

template<typename T>
class Option {
public:
    using ValueType = T;

    // constexpr Option() = default;

    constexpr Option() noexcept
        : m_has_value(false) {}

    constexpr explicit Option(const T& v)
        : m_has_value(true) {
        new (&m_storage) T(std::forward<const T&>(v));
    }

    constexpr Option(const Option& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.value()); }
    }
    constexpr Option(Option&& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.release_value()); }
    }



    constexpr ~Option() { this->clear(); }


    constexpr auto operator=(const Option& o) -> Option& {
        if (m_has_value) { this->value().~T(); }
        m_has_value = o.m_has_value;
        if (o.m_has_value) { new (&m_storage) T(o.value()); }
        return *this;
    }

    constexpr auto operator==(const Option& o) const noexcept -> bool {
        if (m_has_value && o.m_has_value) { return this->value() == o.value(); }
        if (!m_has_value && !o.m_has_value) { return true; }
        return false;
    }

public:
    auto has_value() const -> bool { return m_has_value; }
    // auto explicit operator bool() const { return this->has_value(); }


    constexpr auto value() & -> T& {
        if (m_has_value) { return reinterpret_cast<T&>(m_storage); }
        JF_PANIC("called `Option::value()` on a `None` value");
        std::terminate();
    }
    constexpr auto value() const& -> const T& {
        if (m_has_value) { return reinterpret_cast<const T&>(m_storage); }
        JF_PANIC("called `Option::value()` on a `None` value");
        std::terminate();
    }
    constexpr auto value() && -> T { return this->release_value(); }
    // constexpr auto unwrap() const -> T { return this->value(); }

    template<typename U>
    constexpr auto and_(const Option<U>& o) const& -> Option<U> {
        if (this->has_value()) {
            return o;
        } else {
            return Option<T>();
        }
    }
    constexpr auto or_(const Option<T>& o) const& -> Option<T> {
        if (this->has_value()) {
            return *this;
        } else {
            return o;
        }
    }
    constexpr auto xor_(const Option<T>& o) const& -> Option<T> {
        if (this->has_value() && !o.has_value()) {
            return *this;
        } else if (!this->has_value() && o.has_value()) {
            return o;
        } else {
            return Option<T>();
        }
    }

    // NOTE: in other languages it may be called "flat_map"
    template<typename U, typename F>
    requires std::invocable<F, T>
    constexpr auto and_then(F&& f) const& -> Option<U> {
        if (this->has_value()) {
            return f(this->value());
        } else {
            return Option<U>();
        }
    }
    template<typename U, typename F>
    constexpr auto flat_map(F&& f) const& -> Option<U> {
        return this->and_then(std::forward<F>(f));
    }



    template<typename F>
    // requires std::convertible_to<Option<T>>
    // requires std::invocable<F, int>
    constexpr auto or_else(F&& f) const& -> Option<T> {
        if (this->has_value()) {
            return *this;
        } else {
            return f();
        }
    }

    template<typename U>
    requires std::convertible_to<U, T> && std::copy_constructible<T>
    constexpr auto value_or(const U& d) const& -> T {
        if (m_has_value) { return this->value(); }
        return (T)d;
    }

    template<typename U>
    requires std::convertible_to<U, T> && std::move_constructible<T>
    constexpr auto value_or(U&& d) && -> T {
        if (m_has_value) { return std::move(this->value()); }
        return std::move(d);
    }

    constexpr auto operator*() const -> const T& { return this->value(); }
    constexpr auto operator*() -> T& { return this->value(); }
    constexpr auto operator->() const -> const T* { return &this->value(); }
    constexpr auto operator->() -> T* { return &this->value(); }




private:
    constexpr auto release_value() -> T {
        if (m_has_value) {
            T released_value = std::move(this->value());
            this->value().~T();
            m_has_value = false;
            return released_value;
        }
        JF_PANIC("called `Option::release_value()` on a `None` value");
        std::terminate();
    }
    constexpr auto clear() -> void {
        if (m_has_value) {
            this->value().~T();
            m_has_value = false;
        }
    }

public: // Rust-like wrapper
    constexpr auto is_some() const -> bool { return this->has_value(); }
    constexpr auto is_none() const -> bool { return !this->is_some(); }
    auto           unwrap() -> T& { return this->value(); }
    auto           unwrap() const -> const T& { return this->value(); }



private:
    alignas(T) u8 m_storage[sizeof(T)];
    // alignas(T) std::array<u8, sizeof(T)> m_storage;
    bool m_has_value = false;
};
} // namespace JadeFrame