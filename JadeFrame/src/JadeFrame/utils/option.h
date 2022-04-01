#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/utils/assert.h"

#include <utility>

#include <concepts>
#include <type_traits>



namespace JadeFrame {

/*
    NOTE: This is class is mainly modelled after Rust's Option<T>.
    NOTE: Thus member function should have the same naming convention, though provide C++-like wrapper member functions.
    NOTE: Think about maybe renaming this class (Rust, Ocaml) to Maybe (Haskell) or Optional (C++, Swift)

    TODO: Consider whether the ref-qualified "const&&"" overloads are needed. Most likely not.
*/

template<typename T>
class Option;

template<typename T>
struct is_option : std::false_type {};
template<typename T>
struct is_option<Option<T>> : std::true_type {};

template<typename T>
static constexpr bool is_option_v = is_option<std::decay_t<T>>::value;



template<typename T>
class Option {
public:
    using ValueType = T;

    // Default ctors
    constexpr Option() noexcept
        : m_has_value(false) {}

    // Copy ctors
    constexpr Option(const Option& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.unwrap()); }
    }
    // Move ctors
    constexpr Option(Option&& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.release()); }
    }
    // Ctors
    constexpr explicit Option(const T& v)
        : m_has_value(true) {
        new (&m_storage) T(std::forward<const T&>(v));
    }

    // Dtor
    constexpr ~Option() {
        if (m_has_value) {
            this->unwrap().~T();
            m_has_value = false;
        }
    }

    // Copy assignment
    constexpr auto operator=(const Option& o) -> Option& {
        if (m_has_value) { this->unwrap().~T(); }
        m_has_value = o.m_has_value;
        if (o.m_has_value) { new (&m_storage) T(o.unwrap()); }
        return *this;
    }
    // Move assignment
    constexpr auto operator=(Option&& o) -> Option& {
        if (m_has_value) { this->unwrap().~T(); }
        m_has_value = o.m_has_value;
        if (o.m_has_value) { new (&m_storage) T(o.release()); }
        return *this;
    }

    constexpr auto operator==(const Option& o) const noexcept -> bool {
        if (m_has_value && o.m_has_value) {
            return this->unwrap() == o.unwrap();
        } else if (!m_has_value && !o.m_has_value) {
            return true;
        } else {
            return false;
        }
    }
    constexpr auto operator==(const T& v) const noexcept -> bool {
        if (m_has_value) {
            return this->unwrap() == v;
        } else {
            return false;
        }
    }

    constexpr auto operator*() const -> const T& { return this->unwrap(); }
    constexpr auto operator*() -> T& { return this->unwrap(); }
    constexpr auto operator->() const -> const T* { return &this->unwrap(); }
    constexpr auto operator->() -> T* { return &this->unwrap(); }

public:
    // auto explicit operator bool() const { return this->has_value(); }


    constexpr auto unwrap() & -> T& {
        if (m_has_value) { return reinterpret_cast<T&>(m_storage); }
        JF_PANIC("called `Option::unwrap() & -> T&` on a `None` value");
        std::terminate();
    }
    constexpr auto unwrap() const& -> const T& {
        if (m_has_value) { return reinterpret_cast<const T&>(m_storage); }
        JF_PANIC("called `Option::unwrap() const& -> const T&` on a `None` value");
        std::terminate();
    }
    constexpr auto unwrap() && -> T { return this->release(); }
    // constexpr auto unwrap() const&& -> const T { return this->release(); }

    constexpr auto unwrap_unchecked() & -> T& { return reinterpret_cast<T&>(m_storage); }
    constexpr auto unwrap_unchecked() const& -> const T& { return reinterpret_cast<const T&>(m_storage); }
    constexpr auto unwrap_unchecked() && -> T { return std::move(this->release_unchecked()); }
    // constexpr auto unwrap_unchecked() const&& -> const T { return std::move(this->release_unchecked()); }

    template<typename U>
    constexpr auto and_(const Option<U>& o) const& -> Option<U> {
        if (this->is_some()) {
            return o;
        } else {
            return Option<U>();
        }
    }
    constexpr auto or_(const Option<T>& o) const& -> Option<T> {
        if (this->is_some()) {
            return *this;
        } else {
            return o;
        }
    }
    constexpr auto xor_(const Option<T>& o) const& -> Option<T> {
        if (this->is_some() && !o.is_some()) {
            return *this;
        } else if (!this->is_some() && o.is_some()) {
            return o;
        } else {
            return Option<T>();
        }
    }

    // NOTE: in other languages it may be called "flat_map"
    template<typename U, typename F>
    requires std::invocable<F, T>
    constexpr auto and_then(F&& func) const& -> Option<U> {
        if (this->is_some()) {
            return func(this->unwrap());
        } else {
            return Option<U>();
        }
    }
    // template<typename F>
    // requires std::invocable<F, T>
    // constexpr auto and_then_0(F&& func) const& -> std::invoke_result_t<F, T> {
    //     if (this->has_value()) {
    //         return func(this->value());
    //     } else {
    //         return Option<T>();
    //     }
    // }

    template<typename U, typename F>
    constexpr auto flat_map(F&& func) const& -> Option<U> {
        return this->and_then(std::forward<F>(func));
    }



    template<typename F>
    // requires std::convertible_to<Option<T>>
    // requires std::invocable<F, int>
    constexpr auto or_else(F&& func) const& -> Option<T> {
        if (this->is_some()) {
            return *this;
        } else {
            return func();
        }
    }

    template<typename U>
    requires std::convertible_to<U, T> && std::copy_constructible<T>
    constexpr auto unwrap_or(const U& d) const& -> T {
        if (m_has_value) {
            return this->unwrap();
        } else {
            return (T)d;
        }
    }

    template<typename T, typename U>
    using ctnmc = std::convertible_to<U, T>&& std::move_constructible<T>;

    template<typename U>
    requires std::convertible_to<U, T> && std::move_constructible<T>
    constexpr auto unwrap_or(U&& d) && -> T {
        if (m_has_value) {
            return std::move(this->unwrap());
        } else {
            return std::move(d);
        }
    }




private:
    constexpr auto release() -> T {
        if (m_has_value) {
            T released_value = std::move(this->unwrap());
            this->unwrap().~T();
            m_has_value = false;
            return released_value;
        }
        JF_PANIC("called `Option::release() -> T` on a `None` value");
        std::terminate();
    }
    constexpr auto release_unchecked() -> T {
        T released_value = std::move(this->unwrap_unchecked());
        this->unwrap_unchecked().~T();
        m_has_value = false;
        return released_value;
    }
    constexpr auto clear() -> void {
        if (m_has_value) {
            this->unwrap().~T();
            m_has_value = false;
        }
    }

public:
    constexpr auto is_some() const -> bool { return m_has_value; }
    constexpr auto is_none() const -> bool { return !this->is_some(); }


public: // C++-like wrappers
    auto value() -> T& { return this->unwrap(); }
    auto value() const -> const T& { return this->unwrap(); }
    auto has_value() const -> bool { return this->is_some(); }



private:
    using Type = T;
    alignas(T) u8 m_storage[sizeof(T)];
    // alignas(T) std::array<u8, sizeof(T)> m_storage;
    bool m_has_value = false;
};
} // namespace JadeFrame