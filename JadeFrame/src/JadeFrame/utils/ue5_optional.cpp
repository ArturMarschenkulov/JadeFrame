#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace modern {

struct nullopt_t {
    explicit constexpr nullopt_t(int /*unused*/) {}
};

inline constexpr nullopt_t nullopt{0};

struct intrusive_unset_state {};

namespace detail {

struct empty {};

template<class T>
concept intrusive_optional_value =
    std::constructible_from<T, intrusive_unset_state> && requires(const T& value) {
        { value == intrusive_unset_state{} } -> std::convertible_to<bool>;
    };

template<class T, bool Trivial = std::is_trivially_destructible_v<T>>
union optional_storage {
    char dummy;
    T    value;

    constexpr optional_storage() noexcept
        : dummy{} {}

    template<class... Args>
    constexpr explicit optional_storage(std::in_place_t, Args&&... args)
        : value(std::forward<Args>(args)...) {}

    constexpr ~optional_storage() = default;
};

template<class T>
union optional_storage<T, false> {
    char dummy;
    T    value;

    constexpr optional_storage() noexcept
        : dummy{} {}

    template<class... Args>
    constexpr explicit optional_storage(std::in_place_t, Args&&... args)
        : value(std::forward<Args>(args)...) {}

    constexpr ~optional_storage() {}
};

template<class T>
concept hashable = requires(const T& value) {
    { std::hash<T>{}(value) } -> std::convertible_to<std::size_t>;
};

} // namespace detail

template<class T>
class Optional {
    static_assert(!std::is_reference_v<T>, "optional<T&> is not supported.");
    static_assert(!std::is_void_v<T>, "optional<void> is not supported.");
    static_assert(std::destructible<T>, "T must be destructible.");

    static constexpr bool intrusive = detail::intrusive_optional_value<T>;

public:
    using value_type = T;

    constexpr Optional() noexcept
        requires(!intrusive)
    = default;

    constexpr Optional() noexcept(
        std::is_nothrow_constructible_v<T, intrusive_unset_state>
    )
        requires(intrusive)
        : storage_(std::in_place, intrusive_unset_state{}) {}

    constexpr explicit Optional(
        nullopt_t
        /*unused*/
    ) noexcept(std::is_nothrow_default_constructible_v<Optional>)
        : Optional() {}

    constexpr explicit Optional(const T& value)
        requires std::copy_constructible<T>
        : Optional(std::in_place, value) {}

    constexpr explicit Optional(T&& value)
        requires std::move_constructible<T>
        : Optional(std::in_place, std::move(value)) {}

    template<class... Args>
        requires std::constructible_from<T, Args...>
    explicit constexpr Optional(std::in_place_t /*unused*/, Args&&... args)
        : storage_(std::in_place, std::forward<Args>(args)...) {
        if constexpr (!intrusive) {
            is_set_ = true;
        } else {
            assert(
                this->is_set() &&
                "optional cannot be emplaced with an intrusive unset value"
            );
        }
    }

    constexpr Optional(const Optional& other)
        requires std::copy_constructible<T>
        : Optional() {
        if (other.is_set()) { this->emplace(other.value()); }
    }

    constexpr Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::move_constructible<T>
        : Optional() {
        if (other.is_set()) { this->emplace(std::move(other.value())); }
    }

    constexpr ~Optional()
        requires std::is_trivially_destructible_v<T>
    = default;

    constexpr ~Optional()
        requires(!std::is_trivially_destructible_v<T>)
    {
        if (this->is_set()) { this->destroy_value(); }
    }

    constexpr auto
    operator=(nullopt_t /*unused*/) noexcept(std::is_nothrow_destructible_v<T>)
        -> Optional& {
        this->reset();
        return *this;
    }

    constexpr auto operator=(const Optional& other) -> Optional&
        requires std::copy_constructible<T>
    {
        if (this == std::addressof(other)) { return *this; }

        if (other.is_set()) {
            this->emplace(other.value());
        } else {
            this->reset();
        }

        return *this;
    }

    constexpr auto
    operator=(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        -> Optional&
        requires std::move_constructible<T>
    {
        if (this == std::addressof(other)) { return *this; }

        if (other.is_set()) {
            this->emplace(std::move(other.value()));
        } else {
            this->reset();
        }

        return *this;
    }

    constexpr auto operator=(const T& value) -> Optional&
        requires std::copy_constructible<T>
    {
        if (std::addressof(value) != ptr()) { this->emplace(value); }
        return *this;
    }

    constexpr auto operator=(T&& value) -> Optional&
        requires std::move_constructible<T>
    {
        if (std::addressof(value) != ptr()) { this->emplace(std::move(value)); }
        return *this;
    }

    template<class... Args>
        requires std::constructible_from<T, Args...>
    constexpr auto emplace(Args&&... args) -> T& {
        if constexpr (intrusive) {
            destroy_value();

            try {
                std::construct_at(ptr(), std::forward<Args>(args)...);
            } catch (...) {
                std::construct_at(ptr(), intrusive_unset_state{});
                throw;
            }

            assert(
                this->is_set() &&
                "optional cannot be emplaced with an intrusive unset value"
            );
        } else {
            this->reset();
            std::construct_at(this->ptr(), std::forward<Args>(args)...);
            is_set_ = true;
        }

        return this->value();
    }

    constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>) {
        if (!this->is_set()) { return; }

        destroy_value();

        if constexpr (intrusive) {
            std::construct_at(this->ptr(), intrusive_unset_state{});
        } else {
            is_set_ = false;
        }
    }

    [[nodiscard]] constexpr auto is_set() const noexcept -> bool {
        if constexpr (intrusive) {
            return !(storage_.value == intrusive_unset_state{});
        } else {
            return is_set_;
        }
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept { return is_set(); }

    [[nodiscard]] constexpr auto value() & -> T&;

    [[nodiscard]] constexpr auto value() const& -> const T& {
        assert(is_set() && "called value() on an empty optional");
        return storage_.value;
    }

    [[nodiscard]] constexpr auto value() && -> T&& {
        assert(is_set() && "called value() on an empty optional");
        return std::move(storage_.value);
    }

    [[nodiscard]] constexpr const T&& value() const&& {
        assert(is_set() && "called value() on an empty optional");
        return std::move(storage_.value);
    }

    [[nodiscard]] constexpr auto get_if() noexcept -> T* {
        return is_set() ? ptr() : nullptr;
    }

    [[nodiscard]] constexpr auto get_if() const noexcept -> const T* {
        return is_set() ? ptr() : nullptr;
    }

    [[nodiscard]] constexpr auto operator*() & -> T& { return value(); }

    [[nodiscard]] constexpr auto operator*() const& -> const T& { return value(); }

    [[nodiscard]] constexpr auto operator*() && -> T&& { return std::move(value()); }

    [[nodiscard]] constexpr auto operator*() const&& -> const T&& {
        return std::move(value());
    }

    [[nodiscard]] constexpr auto operator->() -> T* { return std::addressof(value()); }

    [[nodiscard]] constexpr auto operator->() const -> const T* {
        return std::addressof(value());
    }

    [[nodiscard]] constexpr auto value_or(const T& fallback) const& -> const T& {
        return is_set() ? value() : fallback;
    }

    friend constexpr auto operator==(const Optional& lhs, const Optional& rhs) -> bool
        requires std::equality_comparable<T>
    {
        if (lhs.is_set() != rhs.is_set()) { return false; }

        if (!lhs.is_set()) { return true; }

        return lhs.value() == rhs.value();
    }

private:
    [[nodiscard]] constexpr auto ptr() noexcept -> T* {
        return std::addressof(storage_.value);
    }

    [[nodiscard]] constexpr auto ptr() const noexcept -> const T* {
        return std::addressof(storage_.value);
    }

    constexpr void destroy_value() noexcept(std::is_nothrow_destructible_v<T>) {
        if constexpr (!std::is_trivially_destructible_v<T>) { std::destroy_at(ptr()); }
    }

private:
    detail::optional_storage<T> storage_;
    [[no_unique_address]]
    std::conditional_t<intrusive, detail::empty, bool> is_set_{false};
};

template<class T>
[[nodiscard]] constexpr auto Optional<T>::value() & -> T& {
    assert(is_set() && "called value() on an empty optional");
    return storage_.value;
}

template<class T>
inline constexpr bool is_optional_v = false;

template<class T>
inline constexpr bool is_optional_v<Optional<T>> = true;

template<class T>
inline constexpr bool is_optional_v<const Optional<T>> = true;

template<class T>
inline constexpr bool is_optional_v<volatile Optional<T>> = true;

template<class T>
inline constexpr bool is_optional_v<const volatile Optional<T>> = true;

} // namespace modern

template<class T>
    requires modern::detail::hashable<T>
struct std::hash<modern::Optional<T>> {
    auto operator()(const modern::Optional<T>& opt) const -> std::size_t {
        return opt ? std::hash<T>{}(*opt) : 0U;
    }
};