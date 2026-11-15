#pragma once

#include "JadeFrame/utils/option.h"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

namespace JadeFrame {

struct OkTag {
    explicit constexpr OkTag() noexcept = default;
};

struct ErrTag {
    explicit constexpr ErrTag() noexcept = default;
};

inline constexpr OkTag  Ok{};
inline constexpr ErrTag Err{};

namespace result {
namespace details {

[[noreturn]] inline auto fatal_bad_result_access() noexcept -> void { std::abort(); }

struct UnitStorage {};

template<class T>
inline constexpr bool is_tag = std::is_same_v<std::remove_cvref_t<T>, NoneTag> ||
                               std::is_same_v<std::remove_cvref_t<T>, InPlaceTag> ||
                               std::is_same_v<std::remove_cvref_t<T>, OkTag> ||
                               std::is_same_v<std::remove_cvref_t<T>, ErrTag>;

// Logical capabilities are separate from the mutable physical representation.
template<class T, class S>
struct PayloadCapabilities {
    using storage_type = S;
    static constexpr bool copy_constructible = std::is_copy_constructible_v<T>;
    static constexpr bool move_constructible = std::is_move_constructible_v<T>;
    static constexpr bool copy_assignable = std::is_copy_assignable_v<T>;
    static constexpr bool move_assignable = std::is_move_assignable_v<T>;
    static constexpr bool nothrow_copy = std::is_nothrow_copy_constructible_v<T>;
    static constexpr bool nothrow_move = std::is_nothrow_move_constructible_v<T>;
    static constexpr bool nothrow_copy_assignment = std::is_nothrow_copy_assignable_v<T>;
    static constexpr bool nothrow_move_assignment = std::is_nothrow_move_assignable_v<T>;
    static constexpr bool trivial_copy = std::is_trivially_copy_constructible_v<T> &&
                                         std::is_trivially_copy_constructible_v<S>;
    static constexpr bool trivial_move = std::is_trivially_move_constructible_v<T> &&
                                         std::is_trivially_move_constructible_v<S>;
    static constexpr bool trivial_destruction = std::is_trivially_destructible_v<S>;
    static constexpr bool trivial_copy_assignment =
        trivial_copy && std::is_trivially_copy_assignable_v<T> &&
        std::is_trivially_copy_assignable_v<S> && trivial_destruction;
    static constexpr bool trivial_move_assignment =
        trivial_move && std::is_trivially_move_assignable_v<T> &&
        std::is_trivially_move_assignable_v<S> && trivial_destruction;
};

template<class T>
struct PayloadTraits : PayloadCapabilities<T, std::remove_const_t<T>> {
    static_assert(
        std::is_object_v<T> && !std::is_array_v<T>,
        "Result payloads must be non-array objects, lvalue references, or void"
    );
    static_assert(
        std::is_nothrow_destructible_v<T>,
        "Owned Result payloads require nonthrowing destruction"
    );
    static_assert(!is_tag<T>, "Result tag types cannot be payloads");
    using storage_type = std::remove_const_t<T>;

    template<class... Args>
    static constexpr bool constructible = std::is_constructible_v<T, Args&&...>;
    template<class... Args>
    static constexpr bool nothrow_constructible =
        std::is_nothrow_constructible_v<T, Args&&...>;

    template<class... Args>
    static constexpr auto construct(storage_type* target, Args&&... args) noexcept(
        nothrow_constructible<Args...>
    ) -> void {
        std::construct_at(target, std::forward<Args>(args)...);
    }

    static constexpr auto copy(const storage_type& source) noexcept -> const T& {
        return source;
    }

    static constexpr auto move(storage_type& source) noexcept -> T&& {
        return static_cast<T&&>(source);
    }

    template<class S>
    static constexpr auto get(S&& source) noexcept -> decltype(auto) {
        using qualified_type = std::conditional_t<
            std::is_const_v<std::remove_reference_t<S>>,
            std::add_const_t<T>,
            T>;
        if constexpr (std::is_lvalue_reference_v<S&&>) {
            return static_cast<qualified_type&>(source);
        } else {
            return static_cast<qualified_type&&>(source);
        }
    }
};

template<class T>
struct PayloadTraits<T&> : PayloadCapabilities<T*, T*> {
    static_assert(std::is_object_v<T>, "Result references require object referents");
    static_assert(!is_tag<T>, "Result tag types cannot be payloads");
    using storage_type = T*;

    template<class... Args>
    static constexpr bool constructible = false;
    template<class U>
    static constexpr bool constructible<U> =
        option::details::safe_reference_binding<T, U>;
    template<class... Args>
    static constexpr bool nothrow_constructible = constructible<Args...>;

    template<class U>
    static constexpr auto construct(T** target, U&& source) noexcept -> void {
        std::construct_at(target, std::addressof(source));
    }

    static constexpr auto copy(T* const& source) noexcept -> T* { return source; }

    static constexpr auto move(T*& source) noexcept -> T* { return source; }

    template<class S>
    static constexpr auto get(S&& source) noexcept -> T& {
        return *source;
    }
};

template<>
struct PayloadTraits<void> : PayloadCapabilities<UnitStorage, UnitStorage> {
    template<class... Args>
    static constexpr bool constructible = sizeof...(Args) == 0;
    template<class... Args>
    static constexpr bool nothrow_constructible = constructible<Args...>;

    static constexpr auto construct(UnitStorage* target) noexcept -> void {
        std::construct_at(target);
    }

    static constexpr auto copy(const UnitStorage& source) noexcept -> const UnitStorage& {
        return source;
    }

    static constexpr auto move(UnitStorage& source) noexcept -> UnitStorage&& {
        return std::move(source);
    }

    template<class S>
    static constexpr auto get(S&& /*unused*/) noexcept -> void {}
};

template<class T, class E>
union Storage {
    unsigned char dummy;
    T             ok;
    E             err;

    constexpr Storage() noexcept
        : dummy(0) {}

    constexpr ~Storage()
        requires(std::is_trivially_destructible_v<T> &&
                 std::is_trivially_destructible_v<E>)
    = default;

    constexpr ~Storage()
        requires(
            !std::is_trivially_destructible_v<T> || !std::is_trivially_destructible_v<E>
        )
    {}

    Storage(const Storage&) = default;
    Storage(Storage&&) = default;
    auto operator=(const Storage&) -> Storage& = default;
    auto operator=(Storage&&) -> Storage& = default;
};

// Restores the old union member on unwinding; no exception syntax is needed.
template<class Traits>
struct RollbackGuard {
    using S = Traits::storage_type;
    S*   target;
    S    backup;
    bool armed = true;

    explicit constexpr RollbackGuard(S& old) noexcept
        : target(std::addressof(old))
        , backup(Traits::move(old)) {
        std::destroy_at(target);
    }

    RollbackGuard(const RollbackGuard&) = delete;

    constexpr ~RollbackGuard() noexcept {
        if (armed) { std::construct_at(target, Traits::move(backup)); }
    }
};

template<class New, class Old, class Arg>
constexpr auto transition(
    typename New::storage_type& target,
    typename Old::storage_type& old,
    Arg&&                       argument
) noexcept(std::is_nothrow_constructible_v<typename New::storage_type, Arg&&>) -> void {
    using N = New::storage_type;
    if constexpr (std::is_nothrow_constructible_v<N, Arg&&>) {
        std::destroy_at(std::addressof(old));
        std::construct_at(std::addressof(target), std::forward<Arg>(argument));
    } else if constexpr (New::nothrow_move) {
        N temporary(std::forward<Arg>(argument));
        std::destroy_at(std::addressof(old));
        std::construct_at(std::addressof(target), New::move(temporary));
    } else {
        static_assert(Old::nothrow_move);
        RollbackGuard<Old> guard(old);
        std::construct_at(std::addressof(target), std::forward<Arg>(argument));
        guard.armed = false;
    }
}

} // namespace details

template<class T, class E>
class [[nodiscard("Result may contain an unhandled error")]] Result {
    using O = details::PayloadTraits<T>;
    using F = details::PayloadTraits<E>;
    enum class State : unsigned char {
        Ok,
        Err,
    };

    details::Storage<typename O::storage_type, typename F::storage_type> storage_;
    State                                                                state_;

    static constexpr bool copy_constructible =
        O::copy_constructible && F::copy_constructible;
    static constexpr bool move_constructible =
        O::move_constructible && F::move_constructible;
    static constexpr bool trivial_copy = O::trivial_copy && F::trivial_copy;
    static constexpr bool trivial_move = O::trivial_move && F::trivial_move;
    static constexpr bool trivial_destruction =
        O::trivial_destruction && F::trivial_destruction;
    static constexpr bool copy_assignable =
        copy_constructible && O::copy_assignable && F::copy_assignable &&
        (O::nothrow_copy || O::nothrow_move || F::nothrow_move) &&
        (F::nothrow_copy || F::nothrow_move || O::nothrow_move);
    static constexpr bool move_assignable = move_constructible && O::move_assignable &&
                                            F::move_assignable &&
                                            (O::nothrow_move || F::nothrow_move);
    static constexpr bool trivial_copy_assignment =
        copy_assignable && O::trivial_copy_assignment && F::trivial_copy_assignment;
    static constexpr bool trivial_move_assignment =
        move_assignable && O::trivial_move_assignment && F::trivial_move_assignment;

    constexpr auto destroy_active() noexcept -> void {
        if (is_ok()) {
            std::destroy_at(std::addressof(storage_.ok));
        } else {
            std::destroy_at(std::addressof(storage_.err));
        }
    }

public:
    using value_type = T;
    using error_type = E;

    Result() = delete;

    template<class... Args>
        requires(O::template constructible<Args...>)
    explicit constexpr Result(OkTag /*unused*/, Args&&... args) noexcept(
        O::template nothrow_constructible<Args...>
    )
        : state_(State::Ok) {
        O::construct(std::addressof(storage_.ok), std::forward<Args>(args)...);
    }

    template<class... Args>
        requires(F::template constructible<Args...>)
    explicit constexpr Result(ErrTag /*unused*/, Args&&... args) noexcept(
        F::template nothrow_constructible<Args...>
    )
        : state_(State::Err) {
        F::construct(std::addressof(storage_.err), std::forward<Args>(args)...);
    }

    constexpr Result(const Result&)
        requires trivial_copy
    = default;

    constexpr Result(const Result& other) noexcept(O::nothrow_copy && F::nothrow_copy)
        requires(copy_constructible && !trivial_copy)
        : state_(other.state_) {
        if (is_ok()) {
            std::construct_at(std::addressof(storage_.ok), O::copy(other.storage_.ok));
        } else {
            std::construct_at(std::addressof(storage_.err), F::copy(other.storage_.err));
        }
    }

    Result(const Result&)
        requires(!copy_constructible)
    = delete;

    constexpr Result(Result&&)
        requires trivial_move
    = default;

    constexpr Result(Result&& other) noexcept(O::nothrow_move && F::nothrow_move)
        requires(move_constructible && !trivial_move)
        : state_(other.state_) {
        if (is_ok()) {
            std::construct_at(std::addressof(storage_.ok), O::move(other.storage_.ok));
        } else {
            std::construct_at(std::addressof(storage_.err), F::move(other.storage_.err));
        }
    }

    Result(Result&&)
        requires(!move_constructible)
    = delete;

    constexpr ~Result()
        requires trivial_destruction
    = default;

    constexpr ~Result() noexcept
        requires(!trivial_destruction)
    {
        this->destroy_active();
    }

    constexpr auto operator=(const Result&) -> Result&
        requires trivial_copy_assignment
    = default;

    constexpr auto operator=(const Result& other) noexcept(
        O::nothrow_copy && F::nothrow_copy && O::nothrow_copy_assignment &&
        F::nothrow_copy_assignment
    ) -> Result&
        requires(copy_assignable && !trivial_copy_assignment)
    {
        if (this == std::addressof(other)) { return *this; }
        if (is_ok() && other.is_ok()) {
            storage_.ok = O::copy(other.storage_.ok);
        } else if (is_err() && other.is_err()) {
            storage_.err = F::copy(other.storage_.err);
        } else if (other.is_ok()) {
            details::transition<O, F>(
                storage_.ok, storage_.err, O::copy(other.storage_.ok)
            );
            state_ = State::Ok;
        } else {
            details::transition<F, O>(
                storage_.err, storage_.ok, F::copy(other.storage_.err)
            );
            state_ = State::Err;
        }
        return *this;
    }

    auto operator=(const Result&) -> Result&
        requires(!copy_assignable)
    = delete;

    constexpr auto operator=(Result&&) -> Result&
        requires trivial_move_assignment
    = default;

    constexpr auto operator=(Result&& other) noexcept(
        O::nothrow_move && F::nothrow_move && O::nothrow_move_assignment &&
        F::nothrow_move_assignment
    ) -> Result&
        requires(move_assignable && !trivial_move_assignment)
    {
        if (this == std::addressof(other)) { return *this; }
        if (is_ok() && other.is_ok()) {
            storage_.ok = O::move(other.storage_.ok);
        } else if (is_err() && other.is_err()) {
            storage_.err = F::move(other.storage_.err);
        } else if (other.is_ok()) {
            details::transition<O, F>(
                storage_.ok, storage_.err, O::move(other.storage_.ok)
            );
            state_ = State::Ok;
        } else {
            details::transition<F, O>(
                storage_.err, storage_.ok, F::move(other.storage_.err)
            );
            state_ = State::Err;
        }
        return *this;
    }

    auto operator=(Result&&) -> Result&
        requires(!move_assignable)
    = delete;

    [[nodiscard]] constexpr auto is_ok() const noexcept -> bool {
        return state_ == State::Ok;
    }

    [[nodiscard]] constexpr auto is_err() const noexcept -> bool {
        return state_ == State::Err;
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool { return is_ok(); }

    constexpr auto value() & noexcept -> decltype(auto) {
        if (!is_ok()) { details::fatal_bad_result_access(); }
        return O::get(storage_.ok);
    }

    constexpr auto value() const& noexcept -> decltype(auto) {
        if (!is_ok()) { details::fatal_bad_result_access(); }
        return O::get(storage_.ok);
    }

    constexpr auto value() && noexcept -> decltype(auto) {
        if (!is_ok()) { details::fatal_bad_result_access(); }
        return O::get(std::move(storage_.ok));
    }

    constexpr auto value() const&& noexcept -> decltype(auto) {
        if (!is_ok()) { details::fatal_bad_result_access(); }
        return O::get(std::move(storage_.ok));
    }

    constexpr auto error() & noexcept -> decltype(auto) {
        if (!is_err()) { details::fatal_bad_result_access(); }
        return F::get(storage_.err);
    }

    constexpr auto error() const& noexcept -> decltype(auto) {
        if (!is_err()) { details::fatal_bad_result_access(); }
        return F::get(storage_.err);
    }

    constexpr auto error() && noexcept -> decltype(auto) {
        if (!is_err()) { details::fatal_bad_result_access(); }
        return F::get(std::move(storage_.err));
    }

    constexpr auto error() const&& noexcept -> decltype(auto) {
        if (!is_err()) { details::fatal_bad_result_access(); }
        return F::get(std::move(storage_.err));
    }

    constexpr auto operator*() & noexcept -> decltype(auto)
        requires(!std::is_void_v<T>)
    {
        assert(is_ok());
        return O::get(storage_.ok);
    }

    constexpr auto operator*() const& noexcept -> decltype(auto)
        requires(!std::is_void_v<T>)
    {
        assert(is_ok());
        return O::get(storage_.ok);
    }

    constexpr auto operator*() && noexcept -> decltype(auto)
        requires(!std::is_void_v<T>)
    {
        assert(is_ok());
        return O::get(std::move(storage_.ok));
    }

    constexpr auto operator*() const&& noexcept -> decltype(auto)
        requires(!std::is_void_v<T>)
    {
        assert(is_ok());
        return O::get(std::move(storage_.ok));
    }

    constexpr auto operator->() noexcept
        requires(!std::is_void_v<T>)
    {
        assert(is_ok());
        return std::addressof(O::get(storage_.ok));
    }

    constexpr auto operator->() const noexcept
        requires(!std::is_void_v<T>)
    {
        assert(is_ok());
        return std::addressof(O::get(storage_.ok));
    }

    // Arguments must not depend on the lifetime of the payload being replaced.
    template<class... Args>
        requires(O::template nothrow_constructible<Args...>)
    constexpr auto emplace_ok(Args&&... args) noexcept -> decltype(auto) {
        this->destroy_active();
        O::construct(std::addressof(storage_.ok), std::forward<Args>(args)...);
        state_ = State::Ok;
        return O::get(storage_.ok);
    }

    template<class... Args>
        requires(F::template nothrow_constructible<Args...>)
    constexpr auto emplace_err(Args&&... args) noexcept -> decltype(auto) {
        this->destroy_active();
        F::construct(std::addressof(storage_.err), std::forward<Args>(args)...);
        state_ = State::Err;
        return F::get(storage_.err);
    }

    [[nodiscard]] constexpr auto ok() const& noexcept(O::nothrow_copy) -> Option<T>
        requires O::copy_constructible
    {
        if (!is_ok()) { return Option<T>{}; }
        if constexpr (std::is_void_v<T>) {
            return Option<T>{InPlace};
        } else {
            return Option<T>{InPlace, value()};
        }
    }

    [[nodiscard]] constexpr auto ok() && noexcept(O::nothrow_move) -> Option<T>
        requires O::move_constructible
    {
        if (!is_ok()) { return Option<T>{}; }
        if constexpr (std::is_void_v<T>) {
            return Option<T>{InPlace};
        } else {
            return Option<T>{InPlace, std::move(*this).value()};
        }
    }

    [[nodiscard]] constexpr auto err() const& noexcept(F::nothrow_copy) -> Option<E>
        requires F::copy_constructible
    {
        if (!is_err()) { return Option<E>{}; }
        if constexpr (std::is_void_v<E>) {
            return Option<E>{InPlace};
        } else {
            return Option<E>{InPlace, error()};
        }
    }

    [[nodiscard]] constexpr auto err() && noexcept(F::nothrow_move) -> Option<E>
        requires F::move_constructible
    {
        if (!is_err()) { return Option<E>{}; }
        if constexpr (std::is_void_v<E>) {
            return Option<E>{InPlace};
        } else {
            return Option<E>{InPlace, std::move(*this).error()};
        }
    }
};

} // namespace result

using result::Result;

template<class T, class E, class... Args>
    requires requires(Args&&... args) { Result<T, E>{Ok, std::forward<Args>(args)...}; }
[[nodiscard]] constexpr auto
ok(Args&&... args) noexcept(noexcept(Result<T, E>{Ok, std::forward<Args>(args)...}))
    -> Result<T, E> {
    return Result<T, E>{Ok, std::forward<Args>(args)...};
}

template<class T, class E, class... Args>
    requires requires(Args&&... args) { Result<T, E>{Err, std::forward<Args>(args)...}; }
[[nodiscard]] constexpr auto
err(Args&&... args) noexcept(noexcept(Result<T, E>{Err, std::forward<Args>(args)...}))
    -> Result<T, E> {
    return Result<T, E>{Err, std::forward<Args>(args)...};
}

} // namespace JadeFrame
