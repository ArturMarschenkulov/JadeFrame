#pragma once
#include "JadeFrame/defines.h"
#include "JadeFrame/utils/assert.h"
#include <array>

namespace JadeFrame {

/*
    This class is mainly modelled afer Rust's Option<T>.
*/

template<typename T>
class Option {
public:
    using ValueType = T;

    Option() = default;
    constexpr Option(const Option& o) requires(!std::is_trivially_copy_constructible<T>)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.value()); }
    }
    Option(Option&& o)
        : m_has_value(o.m_has_value) {
        if (o.m_has_value) { new (&m_storage) T(o.release_value()); }
    }

    ~Option() {
        if (m_has_value) {
            ~T();
            m_has_value = false;
        }
    }


    /*
        [[nodiscard]] ALWAYS_INLINE T const& value() const&
        {
            VERIFY(m_has_value);
            return *__builtin_launder(reinterpret_cast<T const*>(&m_storage));
        }
    */

    auto value() & -> T& {
        JF_ASSERT(m_has_value, "Option has no value");
        return std::launder(reinterpret_cast<T*>(&m_storage));
    }

    auto value() const& -> const T& {
        JF_ASSERT(m_has_value, "Option has no value");
        return std::launder(reinterpret_cast<const T*>(&m_storage));
    }

    auto value() && -> T&& { return this->release_value(); }

    auto release_value() -> T {
        JF_ASSERT(m_has_value, "Option has no value");
        T released_value = std::move(value());
        this->value().~T();
        m_has_value = false;
        return released_value;
    }

    auto value_or(const T& fallback) const& -> T {
        if (m_has_value) { return this->value(); }
        return fallback;
    }
    auto value_or(T&& fallback) && -> T {
        if (m_has_value) { return std::move(this->value()); }
        return std::move(fallback);
    }

    constexpr auto is_some() const -> bool { return m_has_value; }
    constexpr auto is_none() const -> bool { return !this->is_some(); }
    auto           unwrap() -> T& { return this->value(); }
    auto           unwrap_or(const T& fallback) const& -> T { return this->value_or(fallback); }



private:
    // alignas(T) u8 m_storage[sizeof(T)];
    alignas(T) std::array<u8, sizeof(T)> m_storage;
    bool m_has_value = false;
};
} // namespace JadeFrame