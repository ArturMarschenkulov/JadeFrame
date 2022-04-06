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
concept option_concept_functions = requires(T v) {
    { v.is_none() } -> std::same_as<bool>;
    { v.is_some() } -> std::same_as<bool>;
    { v.unwrap() } -> std::same_as<T>;
};


// NOTE: In "Option_Base" member variables and member functions are located which are specific to whether T is a lvalue
// reference or not.
template<typename T>
struct Option_Base;

template<typename T>
requires std::is_lvalue_reference_v<T>
class Option_Base<T> {
public:
    // Default ctors
    constexpr Option_Base() noexcept
        : m_has_value(false)
        , m_pointer(nullptr) {}

    // Copy ctors
    constexpr Option_Base(const Option_Base& o)
        : m_has_value(o.m_has_value)
        , m_pointer(o.m_pointer) {}
    // Move ctors
    constexpr Option_Base(Option_Base&& o)
        : m_has_value(o.m_has_value)
        , m_pointer(o.m_pointer) {
        o.m_has_value = false;
        o.m_pointer = nullptr;
    }
    // Ctors
    constexpr Option_Base(const T& v)
        : m_has_value(true)
        , m_pointer(&v) {}

    // constexpr Option_Base(T&& v)
    //     : m_has_value(true)
    //     , m_pointer(nullptr) {}

    constexpr auto unwrap() const -> T {
        JF_ASSERT(m_has_value, "Option has no value");
        return *m_pointer;
    }

    constexpr auto is_some() const -> bool { return m_has_value; }

    std::remove_reference<T>::type* m_pointer = nullptr;
    bool                            m_has_value = false;
};
template<typename T>
requires(!std::is_lvalue_reference_v<T>) struct Option_Base<T> {
    // Default ctors
    constexpr Option_Base() noexcept
        : m_has_value(false) {}

    // Copy ctors
    constexpr Option_Base(const Option_Base& o) requires(!std::copy_constructible<T>) = delete;
    constexpr Option_Base(const Option_Base& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.unwrap()); }
    }
    // Move ctors
    constexpr Option_Base(Option_Base&& o) requires(!std::move_constructible<T>) = delete;
    constexpr Option_Base(Option_Base&& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.release()); }
    }
    // Ctors
    constexpr explicit Option_Base(const T& v)
        : m_has_value(true) {
        new (&m_storage) T(std::forward<const T&>(v));
    }

    // Dtor
    constexpr ~Option_Base() requires(!std::destructible<T>) = delete;
    constexpr ~Option_Base() {
        if (m_has_value) {
            this->unwrap().~T();
            m_has_value = false;
        }
    }

    // Copy assignment

    constexpr auto operator=(const Option_Base& o)
        -> Option_Base& requires(!std::copy_constructible<T> || std::destructible<T>) = delete;
    constexpr auto operator=(const Option_Base& o) -> Option_Base& {
        if (m_has_value) { this->unwrap().~T(); }
        m_has_value = o.m_has_value;
        if (o.m_has_value) { new (&m_storage) T(o.unwrap()); }
        return *this;
    }
    // Move assignment
    constexpr auto operator=(const Option_Base& o)
        -> Option_Base& requires(!std::move_constructible<T> || std::destructible<T>) = delete;
    constexpr auto operator=(Option_Base&& o) -> Option_Base& {
        if (m_has_value) { this->unwrap().~T(); }
        m_has_value = o.m_has_value;
        if (o.m_has_value) { new (&m_storage) T(o.release()); }
        return *this;
    }

    constexpr auto operator==(const Option_Base& o) const noexcept -> bool {
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

    constexpr auto unwrap_unchecked() & -> T& { return reinterpret_cast<T&>(m_storage); }
    constexpr auto unwrap_unchecked() const& -> const T& { return reinterpret_cast<const T&>(m_storage); }
    constexpr auto unwrap_unchecked() && -> T { return std::move(this->release_unchecked()); }

    constexpr auto operator*() const -> const T& { return this->unwrap(); }
    constexpr auto operator*() -> T& { return this->unwrap(); }
    constexpr auto operator->() const -> const T* { return &this->unwrap(); }
    constexpr auto operator->() -> T* { return &this->unwrap(); }

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
    constexpr auto is_some() const -> bool { return m_has_value; }

protected:
    alignas(T) u8 m_storage[sizeof(T)];
    bool m_has_value;
};

template<typename T>
class Option : public Option_Base<T> {
public:
    // NOTE: Ctors, Dtor as well as assignment operators are in "Option_Base<T>"
    // Ctors
    using Option_Base<T>::Option_Base;
    // constexpr Option() noexcept
    //     : Option_Base<T>() {}
    // constexpr Option(const T& v)
    //     : Option_Base<T>(v) {}
    // constexpr Option(T&& v)
    //     : Option_Base<T>(v) {}

public:
    // constexpr auto is_some() const -> bool { return m_has_value; }
    constexpr auto is_none() const -> bool { return !this->is_some(); }

    template<typename U = T>
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
    // requires std::convertible_to<Option<T>>
    requires std::invocable<F, int>
    constexpr auto or_else(F&& func) const& -> Option<T> {
        if (this->is_some()) {
            return *this;
        } else {
            return func();
        }
    }



private:
    // NOTE: The member variables are found in "Option_Base<T>".
    // bool m_has_value = false;
};

// namespace L {
// template<typename T>
// requires(!std::is_lvalue_reference_v<T>) class Option<T> {
// public:
//     using ValueType = T;

//     // Default ctors
//     constexpr Option() noexcept
//         : m_has_value(false) {}

//     // Copy ctors
//     constexpr Option(const Option& o) requires(!std::copy_constructible<T>) = delete;
//     constexpr Option(const Option& o)
//         : m_has_value(o.m_has_value) {
//         if (o.m_has_value) { new (&m_storage) T(o.unwrap()); }
//     }
//     // Move ctors
//     constexpr Option(Option&& o) requires(!std::move_constructible<T>) = delete;
//     constexpr Option(Option&& o)
//         : m_has_value(o.m_has_value) {
//         if (o.m_has_value) { new (&m_storage) T(o.release()); }
//     }
//     // Ctors
//     constexpr explicit Option(const T& v)
//         : m_has_value(true) {
//         new (&m_storage) T(std::forward<const T&>(v));
//     }

//     // Dtor
//     constexpr ~Option() requires(!std::destructible<T>) = delete;
//     constexpr ~Option() {
//         if (m_has_value) {
//             this->unwrap().~T();
//             m_has_value = false;
//         }
//     }

//     // Copy assignment

//     constexpr auto operator=(const Option& o)
//         -> Option& requires(!std::copy_constructible<T> || std::destructible<T>) = delete;
//     constexpr auto operator=(const Option& o) -> Option& {
//         if (m_has_value) { this->unwrap().~T(); }
//         m_has_value = o.m_has_value;
//         if (o.m_has_value) { new (&m_storage) T(o.unwrap()); }
//         return *this;
//     }
//     // Move assignment
//     constexpr auto operator=(const Option& o)
//         -> Option& requires(!std::move_constructible<T> || std::destructible<T>) = delete;
//     constexpr auto operator=(Option&& o) -> Option& {
//         if (m_has_value) { this->unwrap().~T(); }
//         m_has_value = o.m_has_value;
//         if (o.m_has_value) { new (&m_storage) T(o.release()); }
//         return *this;
//     }

//     constexpr auto operator==(const Option& o) const noexcept -> bool {
//         if (m_has_value && o.m_has_value) {
//             return this->unwrap() == o.unwrap();
//         } else if (!m_has_value && !o.m_has_value) {
//             return true;
//         } else {
//             return false;
//         }
//     }
//     constexpr auto operator==(const T& v) const noexcept -> bool {
//         if (m_has_value) {
//             return this->unwrap() == v;
//         } else {
//             return false;
//         }
//     }

// public:
//     // auto explicit operator bool() const { return this->has_value(); }


//     constexpr auto unwrap() & -> T& {
//         if (m_has_value) { return reinterpret_cast<T&>(m_storage); }
//         JF_PANIC("called `Option::unwrap() & -> T&` on a `None` value");
//         std::terminate();
//     }
//     constexpr auto unwrap() const& -> const T& {
//         if (m_has_value) { return reinterpret_cast<const T&>(m_storage); }
//         JF_PANIC("called `Option::unwrap() const& -> const T&` on a `None` value");
//         std::terminate();
//     }
//     constexpr auto unwrap() && -> T { return this->release(); }

//     constexpr auto unwrap_unchecked() & -> T& { return reinterpret_cast<T&>(m_storage); }
//     constexpr auto unwrap_unchecked() const& -> const T& { return reinterpret_cast<const T&>(m_storage); }
//     constexpr auto unwrap_unchecked() && -> T { return std::move(this->release_unchecked()); }



//     template<typename U>
//     requires std::convertible_to<U, T> && std::copy_constructible<T>
//     constexpr auto unwrap_or(const U& d) const& -> T {
//         if (m_has_value) {
//             return this->unwrap();
//         } else {
//             return (T)d;
//         }
//     }

//     template<typename U>
//     requires std::convertible_to<U, T> && std::move_constructible<T>
//     constexpr auto unwrap_or(U&& d) && -> T {
//         if (m_has_value) {
//             return std::move(this->unwrap());
//         } else {
//             return std::move(d);
//         }
//     }




// private:
//     constexpr auto release() -> T {
//         if (m_has_value) {
//             T released_value = std::move(this->unwrap());
//             this->unwrap().~T();
//             m_has_value = false;
//             return released_value;
//         }
//         JF_PANIC("called `Option::release() -> T` on a `None` value");
//         std::terminate();
//     }
//     constexpr auto release_unchecked() -> T {
//         T released_value = std::move(this->unwrap_unchecked());
//         this->unwrap_unchecked().~T();
//         m_has_value = false;
//         return released_value;
//     }

// public:
//     constexpr auto is_some() const -> bool { return m_has_value; }
//     constexpr auto is_none() const -> bool { return !this->is_some(); }


//     template<typename U = T>
//     constexpr auto and_(const Option<U>& o) const& -> Option<U> {
//         if (this->is_some()) {
//             return o;
//         } else {
//             return Option<U>();
//         }
//     }
//     constexpr auto or_(const Option<T>& o) const& -> Option<T> {
//         if (this->is_some()) {
//             return *this;
//         } else {
//             return o;
//         }
//     }
//     constexpr auto xor_(const Option<T>& o) const& -> Option<T> {
//         if (this->is_some() && !o.is_some()) {
//             return *this;
//         } else if (!this->is_some() && o.is_some()) {
//             return o;
//         } else {
//             return Option<T>();
//         }
//     }

//     // NOTE: in other languages it may be called "flat_map"
//     template<typename U = T, typename F>
//     requires std::invocable<F, T>
//     constexpr auto and_then(F&& func) const& -> Option<U> {
//         if (this->is_some()) {
//             return func(this->unwrap());
//         } else {
//             return Option<U>();
//         }
//     }

//     template<typename F>
//     // requires std::convertible_to<Option<T>>
//     requires std::invocable<F, int>
//     constexpr auto or_else(F&& func) const& -> Option<T> {
//         if (this->is_some()) {
//             return *this;
//         } else {
//             return func();
//         }
//     }

// public: // C++-like wrappers
//     auto value() -> T& { return this->unwrap(); }
//     auto value() const -> const T& { return this->unwrap(); }
//     auto has_value() const -> bool { return this->is_some(); }



// private:
//     using Type = T;

//     alignas(T) u8 m_storage[sizeof(T)];

//     bool m_has_value = false;
// };

// template<typename T>
// requires(std::is_lvalue_reference_v<T>) class Option<T> {
// public:
//     // Default ctors
//     constexpr Option() noexcept
//         : m_has_value(false)
//         , m_pointer(nullptr) {}

//     // Copy ctors
//     constexpr Option(const Option& o)
//         : m_has_value(o.m_has_value)
//         , m_pointer(o.m_pointer) {}
//     // Move ctors
//     constexpr Option(Option&& o)
//         : m_has_value(o.m_has_value)
//         , m_pointer(o.m_pointer) {
//         o.m_pointer = nullptr;
//         // o.m_has_value = false;
//     }
//     // Ctors
//     constexpr explicit Option(const T& v)
//         : m_has_value(true)
//         , m_pointer(&v) {}


//     constexpr auto unwrap() -> T {
//         if (m_pointer != nullptr) { return *m_pointer; }
//     }

//     template<typename U = T>
//     constexpr auto and_(const Option<U>& o) const& -> Option<U> {
//         if (this->is_some()) {
//             return o;
//         } else {
//             return Option<U>();
//         }
//     }
//     constexpr auto or_(const Option<T>& o) const& -> Option<T> {
//         if (this->is_some()) {
//             return *this;
//         } else {
//             return o;
//         }
//     }
//     constexpr auto xor_(const Option<T>& o) const& -> Option<T> {
//         if (this->is_some() && !o.is_some()) {
//             return *this;
//         } else if (!this->is_some() && o.is_some()) {
//             return o;
//         } else {
//             return Option<T>();
//         }
//     }

//     // NOTE: in other languages it may be called "flat_map"
//     template<typename U = T, typename F>
//     requires std::invocable<F, T>
//     constexpr auto and_then(F&& func) const& -> Option<U> {
//         if (this->is_some()) {
//             return func(this->unwrap());
//         } else {
//             return Option<U>();
//         }
//     }

//     template<typename F>
//     // requires std::convertible_to<Option<T>>
//     requires std::invocable<F, int>
//     constexpr auto or_else(F&& func) const& -> Option<T> {
//         if (this->is_some()) {
//             return *this;
//         } else {
//             return func();
//         }
//     }


// private:
//     bool                            m_has_value = false;
//     std::remove_reference<T>::type* m_pointer = nullptr;
//     // std::remove_reference<T>::type* m_pointer = nullptr;
// };

// } // namespace L

static auto test_options() -> void {

    // Testing "and"
    auto s = Option<i32>(2_i32);
    s.and_(Option<i32>(3_i32));
    {
        assert(Option<int>{1}.and_(Option<int>{2}) == Option<int>{2});
        assert(Option<int>{1}.and_(Option<int>{}) == Option<int>{});
        assert(Option<int>{}.and_(Option<int>{2}) == Option<int>{});
        assert(Option<int>{}.and_(Option<int>{}) == Option<int>{});
    }
}
} // namespace JadeFrame