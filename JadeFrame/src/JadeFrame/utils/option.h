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
namespace option {
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


namespace details {
    // NOTE: In "Option_Base" member variables and member functions are located which are specific to whether T is a
    // lvalue reference or not.
    template<typename T>
    class Option_Base;

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

        // constexpr Option_Base(T&& v) = delete;

        constexpr auto operator=(const Option_Base& o) -> Option_Base& = delete;
        constexpr auto operator=(Option_Base&& o) -> Option_Base& = delete;

        constexpr auto unwrap() & -> T& {
            JF_ASSERT(m_has_value, "Option has no value");
            return *m_pointer;
        }
        constexpr auto unwrap() const& -> const T& {
            JF_ASSERT(m_has_value, "Option has no value");
            return *m_pointer;
        }
        constexpr auto unwrap() && -> T&& {
            JF_ASSERT(m_has_value, "Option has no value");
            return std::move(*m_pointer);
        }

    protected:
        std::remove_reference_t<T>* m_pointer = nullptr;
        bool                        m_has_value = false;
    };
    template<typename T>
    requires(!std::is_lvalue_reference_v<T>) class Option_Base<T> {
    public:
        static_assert(!std::is_lvalue_reference_v<T> && !std::is_rvalue_reference_v<T>);
        // Default ctors
        constexpr Option_Base() noexcept
            : m_has_value(false) {}

        // Copy ctors
        // constexpr Option_Base(const Option_Base& o) requires(!std::copy_constructible<T>) = delete;
        constexpr Option_Base(const Option_Base& o)
            : m_has_value(o.m_has_value) {
            if (o.m_has_value) { new (&m_storage) T(o.unwrap_unchecked()); }
        }
        // Move ctors
        // constexpr Option_Base(Option_Base&& o) requires(!std::move_constructible<T>) = delete;
        constexpr Option_Base(Option_Base&& o)
            : m_has_value(o.m_has_value) {
            if (o.m_has_value) { new (&m_storage) T(o.release_unchecked()); }
        }
        // Ctors
        constexpr explicit Option_Base(const T& v)
            : m_has_value(true) {
            new (&m_storage) T(v);
            // new (&m_storage) T(std::forward<const T&>(v));
        }
        // constexpr explicit Option_Base(const T& v) = delete;
        constexpr explicit Option_Base(T&& v)
            : m_has_value(true) {
            new (&m_storage) T(std::forward<T>(v));
        }

        // Dtor
        // constexpr ~Option_Base() requires(!std::destructible<T>) = delete;
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

    protected:
        alignas(T) u8 m_storage[sizeof(T)];
        bool m_has_value;
    };

    template<typename T>
    class Storage {
    public:
        bool m_has_value;
    };
    template<typename T>
    requires std::is_lvalue_reference_v<T>
    class Storage<T> {
    public:
        constexpr Storage()
            : m_has_value(false) {}
        constexpr Storage(const Storage& o)
            : m_has_value(o.m_has_value)
            , m_pointer(o.m_pointer) {}
        constexpr Storage(Storage&& o)
            : m_has_value(o.m_has_value)
            , m_pointer(o.m_pointer) {
            o.m_has_value = false;
            o.m_pointer = nullptr;
        }
        constexpr Storage(const T& v)
            : m_has_value(true)
            , m_pointer(&v) {}
        constexpr Storage(T&& v)
            : m_has_value(true)
            , m_pointer(&v) {}


        constexpr auto get() const& -> const T& { return *m_pointer; }

    public:
        std::remove_reference_t<T>* m_pointer = nullptr;
        bool                        m_has_value = false;
    };

    template<typename T>
    requires(!std::is_lvalue_reference_v<T>) class Storage<T> {
    public:
        constexpr Storage()
            : m_has_value(false) {}
        constexpr Storage(const Storage& o)
            : m_has_value(o.m_has_value) {
            if (o.m_has_value) { new (&m_storage) T(reinterpret_cast<const T&>(o.m_storage)); }
        }

        constexpr Storage(Storage&& o)
            : m_has_value(o.m_has_value) {
            if (o.m_has_value) {
                T rv = std::move(reinterpret_cast<T&>(o.m_storage));
                reinterpret_cast<T&>(o.m_storage).~T();
                m_has_value = false;
                new (&m_storage) T(rv);
            }
        }

        constexpr Storage(const T& v)
            : m_has_value(true) {
            new (&m_storage) T(v);
        }

        constexpr Storage(T&& v)
            : m_has_value(true) {
            new (&m_storage) T(std::forward<T>(v));
        }
        constexpr auto get() const& -> const T& { return reinterpret_cast<const T&>(m_storage); }
        constexpr auto has_value() const -> bool { return m_has_value; }

    private:
        alignas(T) u8 m_storage[sizeof(T)];
        bool m_has_value;
    };
} // namespace details

template<typename T>
class Option2 {
public:
    constexpr Option2()
        : m_storage() {}
    constexpr Option2(const Option2& o)
        : m_storage(o.m_storage) {}
    constexpr Option2(Option2&& o)
        : m_storage(std::move(o.m_storage)) {}
    constexpr Option2(const T& v)
        : m_storage(v) {}
    constexpr Option2(T&& v)
        : m_storage(std::move(v)) {}

    constexpr auto operator==(const Option2& o) const noexcept -> bool {
        if (m_storage.has_value() && o.m_storage.has_value()) {
            return this->unwrap() == o.unwrap();
        } else if (!m_storage.has_value() && !o.m_storage.has_value()) {
            return true;
        } else {
            return false;
        }
    }
    constexpr auto operator==(const T& v) const noexcept -> bool {
        if (m_storage.has_value()) {
            return this->unwrap() == v;
        } else {
            return false;
        }
    }

    constexpr auto is_some() const -> bool { return m_storage.has_value(); }
    constexpr auto is_none() const -> bool { return !this->is_some(); }

    constexpr auto unwrap() & -> T& {
        if (m_storage.has_value()) { return m_storage.get(); }
        JF_PANIC("called `Option::unwrap() & -> T&` on a `None` value");
        std::terminate();
    }
    constexpr auto unwrap() const& -> const T& {
        if (m_storage.has_value()) { return m_storage.get(); }
        JF_PANIC("called `Option::unwrap() const& -> const T&` on a `None` value");
        std::terminate();
    }
    constexpr auto unwrap() && -> T { return this->release(); }

    constexpr auto release() -> T {
        if (m_storage.m_has_value) {
            T released_value = std::move(this->unwrap());
            this->unwrap().~T();
            m_storage.m_has_value = false;
            return released_value;
        }
        JF_PANIC("called `Option::release() -> T` on a `None` value");
        std::terminate();
    }


    template<typename U = T>
    constexpr auto and_(const Option2<U>& o) const& -> Option2<U> {
        if (this->is_some()) {
            return o;
        } else {
            return Option2<U>();
        }
    }
    constexpr auto or_(const Option2<T>& o) const& -> Option2<T> {
        if (this->is_some()) {
            return *this;
        } else {
            return o;
        }
    }
    constexpr auto xor_(const Option2<T>& o) const& -> Option2<T> {
        if (this->is_some() && !o.is_some()) {
            return *this;
        } else if (!this->is_some() && o.is_some()) {
            return o;
        } else {
            return Option2<T>();
        }
    }

    template<typename U = T, typename F>
    requires std::invocable<F, T>
    constexpr auto and_then(F&& func) const& -> Option2<U> {
        if (this->is_some()) {
            return func(this->unwrap());
        } else {
            return Option2<U>();
        }
    }

    template<typename F>
    // requires std::convertible_to<Option<T>>
    requires std::invocable<F, int>
    constexpr auto or_else(F&& func) const& -> Option2<T> {
        if (this->is_some()) {
            return *this;
        } else {
            return func();
        }
    }

private:
    details::Storage<T> m_storage;
};

template<typename T>
class Option : public details::Option_Base<T> {
public:
    // NOTE: Ctors, Dtor as well as assignment operators are in "Option_Base<T>"
    // Ctors
    using details::Option_Base<T>::Option_Base;

    // Option()
    //     : Option_Base<T>() {}
    // Option(const Option& o)
    //     : Option_Base<T>(o) {}
    // Option(Option&& o)
    //     : Option_Base<T>(std::move(o)) {}
    // Option(const T& v)
    //     : Option_Base<T>(v) {}
    // ~Option()
    // //: ~Option_Base<T>()
    // {}

public:
    constexpr auto is_some() const -> bool { return details::Option_Base<T>::m_has_value; }
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

public: // C++-like wrappers
    auto value() -> T& { return this->unwrap(); }
    auto value() const -> const T& { return this->unwrap(); }
    auto has_value() const -> bool { return this->is_some(); }

private:
    // NOTE: The member variables are found in "Option_Base<T>".
    // bool m_has_value = false;
};


namespace tests {
static auto test() -> void {


    // Testing functions
    {
        Option<u32> x0 = Option<u32>(2);
        assert(x0.is_some() == true);

        Option<u32> x1 = Option<u32>();
        assert(x1.is_some() == false);
    }
    {
        Option<u32> x0 = Option<u32>(2);
        assert(x0.is_none() == false);

        Option<u32> x1 = Option<u32>();
        assert(x1.is_none() == true);
    }

    // Testing "and"
    auto s = Option<i32>(2_i32);
    s.and_(Option<i32>(3_i32));
    {
        Option<i32>         x0 = Option<i32>(2_i32);
        Option<const char*> y0 = Option<const char*>();
        assert(x0.and_(y0) == Option<const char*>());

        Option<i32>         x1 = Option<i32>();
        Option<const char*> y1 = Option<const char*>("foo");
        assert(x1.and_(y1) == Option<const char*>());

        Option<i32>         x2 = Option<i32>(2_i32);
        Option<const char*> y2 = Option<const char*>("foo");
        assert(x2.and_(y2) == Option<const char*>("foo"));

        Option<i32>         x3 = Option<i32>();
        Option<const char*> y3 = Option<const char*>();
        assert(x3.and_(x3) == Option<i32>());


        assert(Option<int>{1}.and_(Option<int>{2}) == Option<int>{2});
        assert(Option<int>{1}.and_(Option<int>{}) == Option<int>{});
        assert(Option<int>{}.and_(Option<int>{2}) == Option<int>{});
        assert(Option<int>{}.and_(Option<int>{}) == Option<int>{});
    }

    {
        assert(Option<int>{1}.or_(Option<int>{2}) == Option<int>{1});
        assert(Option<int>{1}.or_(Option<int>{}) == Option<int>{1});
        assert(Option<int>{}.or_(Option<int>{2}) == Option<int>{2});
        assert(Option<int>{}.or_(Option<int>{}) == Option<int>{});
    }

    {
        assert(Option<int>{1}.xor_(Option<int>{2}) == Option<int>{});
        assert(Option<int>{1}.xor_(Option<int>{}) == Option<int>{1});
        assert(Option<int>{}.xor_(Option<int>{2}) == Option<int>{2});
        assert(Option<int>{}.xor_(Option<int>{}) == Option<int>{});
    }
}
static auto test2() -> void {
    Option2<int> a = Option2<int>(333);

    // Testing functions
    {
        Option2<u32> x0 = Option2<u32>(2);
        assert(x0.is_some() == true);

        Option2<u32> x1 = Option2<u32>();
        assert(x1.is_some() == false);
    }
    {
        Option2<u32> x0 = Option2<u32>(2);
        assert(x0.is_none() == false);

        Option2<u32> x1 = Option2<u32>();
        assert(x1.is_none() == true);
    }
    // Testing "and"
    auto s = Option2<i32>(2_i32);
    s.and_(Option2<i32>(3_i32));


    {
        Option2<i32>         x0 = Option2<i32>(2_i32);
        Option2<const char*> y0 = Option2<const char*>();
        assert(x0.and_(y0) == Option2<const char*>());

        Option2<i32>         x1 = Option2<i32>();
        Option2<const char*> y1 = Option2<const char*>("foo");
        assert(x1.and_(y1) == Option2<const char*>());

        Option2<i32>         x2 = Option2<i32>(2_i32);
        Option2<const char*> y2 = Option2<const char*>("foo");
        assert(x2.and_(y2) == Option2<const char*>("foo"));

        Option2<i32>         x3 = Option2<i32>();
        Option2<const char*> y3 = Option2<const char*>();
        assert(x3.and_(x3) == Option2<i32>());


        assert(Option2<int>{1}.and_(Option2<int>{2}) == Option2<int>{2});
        assert(Option2<int>{1}.and_(Option2<int>{}) == Option2<int>{});
        assert(Option2<int>{}.and_(Option2<int>{2}) == Option2<int>{});
        assert(Option2<int>{}.and_(Option2<int>{}) == Option2<int>{});
    }

    {
        assert(Option2<int>{1}.or_(Option2<int>{2}) == Option2<int>{1});
        assert(Option2<int>{1}.or_(Option2<int>{}) == Option2<int>{1});
        assert(Option2<int>{}.or_(Option2<int>{2}) == Option2<int>{2});
        assert(Option2<int>{}.or_(Option2<int>{}) == Option2<int>{});
    }

    {
        assert(Option2<int>{1}.xor_(Option2<int>{2}) == Option2<int>{});
        assert(Option2<int>{1}.xor_(Option2<int>{}) == Option2<int>{1});
        assert(Option2<int>{}.xor_(Option2<int>{2}) == Option2<int>{2});
        assert(Option2<int>{}.xor_(Option2<int>{}) == Option2<int>{});
    }
}
static auto lvalue_ref() -> void {
    struct Student {
        int         age;
        std::string name;

        auto get_name() const -> Option<const std::string&> {
            if (age >= 18) {
                return Option<const std::string&>(name);
            } else {
                return Option<const std::string&>();
            }
        }
    };

    Student s = {16, "John"};
    auto    x = s.get_name();
    assert(x.is_some() == false);
}
} // namespace tests
static auto test() -> void {
    tests::test();
    tests::test2();
    tests::lvalue_ref();
}
} // namespace option
using option::Option;
} // namespace JadeFrame