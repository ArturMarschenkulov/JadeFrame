#pragma once

#include <cassert>
#include <cstdlib>
#include <memory>
#include <type_traits>
#include <utility>

namespace JadeFrame {

struct NoneTag {
    explicit constexpr NoneTag() noexcept = default;
};

struct InPlaceTag {
    explicit constexpr InPlaceTag() noexcept = default;
};

// X11 defines an object-like None macro. Preserve it for platform code while
// allowing this header to be included after Xlib/GLX/Vulkan headers.
#pragma push_macro("None")
#undef None
inline constexpr NoneTag None{};
#pragma pop_macro("None")

inline constexpr InPlaceTag InPlace{};

namespace option {
namespace details {

// Independent of assertions and exceptions: checked access is fatal in every build.
[[noreturn]] inline auto fatal_bad_option_access() noexcept -> void { std::abort(); }

template<class T>
union Storage {
    unsigned char dummy;
    T             value;

    constexpr Storage() noexcept
        : dummy(0) {}

    constexpr ~Storage()
        requires std::is_trivially_destructible_v<T>
    = default;

    constexpr ~Storage()
        requires(!std::is_trivially_destructible_v<T>)
    {}

    Storage(const Storage&) = default;
    Storage(Storage&&) = default;
    auto operator=(const Storage&) -> Storage& = default;
    auto operator=(Storage&&) -> Storage& = default;
};

// Pointer convertibility excludes user conversions that might bind a temporary.
// On C++20 this deliberately accepts only compatible object lvalues.
template<class T, class U>
inline constexpr bool safe_reference_binding =
    std::is_lvalue_reference_v<U&&> &&
    std::is_convertible_v<std::remove_reference_t<U>*, T*>
#if defined(__cpp_lib_reference_from_temporary) &&                                       \
    __cpp_lib_reference_from_temporary >= 202202L
    && !std::reference_constructs_from_temporary_v<T&, U&&>
#endif
    ;

} // namespace details

template<class T>
class Option {
    static_assert(
        std::is_object_v<T> && !std::is_array_v<T>,
        "Option<T> requires a non-array object; rvalue references are unsupported"
    );
    static_assert(std::is_destructible_v<T>, "Option<T> requires a destructible payload");
    static_assert(
        !std::is_same_v<std::remove_cv_t<T>, NoneTag> &&
            !std::is_same_v<std::remove_cv_t<T>, InPlaceTag>,
        "Option tag types cannot be payloads"
    );

    // Store an unqualified object so construct_at also supports Option<const T>
    // on C++20 libraries. Public access and operation constraints retain const.
    details::Storage<std::remove_const_t<T>> storage_;
    bool                                     engaged_ = false;

    static constexpr bool trivial_copy_assignment =
        std::is_trivially_copy_constructible_v<T> &&
        std::is_trivially_copy_assignable_v<T> && std::is_trivially_destructible_v<T>;
    static constexpr bool trivial_move_assignment =
        std::is_trivially_move_constructible_v<T> &&
        std::is_trivially_move_assignable_v<T> && std::is_trivially_destructible_v<T>;

    template<class... Args>
    constexpr auto
    construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args&&...>)
        -> void {
        std::construct_at(std::addressof(storage_.value), std::forward<Args>(args)...);
        engaged_ = true;
    }

    struct TakeTag {};

    // A prvalue return avoids a second move even when NRVO is disabled. Delegation
    // ensures this completed object is destroyed if source.reset() throws.
    constexpr Option(TakeTag, Option& source) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>
    )
        : Option(InPlace, std::move(source).value()) {
        source.reset();
    }

public:
    using value_type = T;

    constexpr Option() noexcept = default;

    explicit constexpr Option(NoneTag) noexcept {}

    explicit constexpr Option(
        const T& value
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires std::is_copy_constructible_v<T>
    {
        this->construct(value);
    }

    explicit constexpr Option(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::is_move_constructible_v<T>
    {
        this->construct(std::move(value));
    }

    template<class... Args>
        requires std::is_constructible_v<T, Args&&...>
    explicit constexpr Option(InPlaceTag, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args&&...>
    ) {
        this->construct(std::forward<Args>(args)...);
    }

    constexpr Option(const Option&)
        requires std::is_trivially_copy_constructible_v<T>
    = default;

    constexpr Option(
        const Option& other
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires(
            std::is_copy_constructible_v<T> && !std::is_trivially_copy_constructible_v<T>
        )
    {
        if (other.engaged_) { this->construct(other.storage_.value); }
    }

    Option(const Option&)
        requires(!std::is_copy_constructible_v<T>)
    = delete;

    constexpr Option(Option&&)
        requires std::is_trivially_move_constructible_v<T>
    = default;

    constexpr Option(Option&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires(
            std::is_move_constructible_v<T> && !std::is_trivially_move_constructible_v<T>
        )
    {
        if (other.engaged_) { this->construct(std::move(other).value()); }
    }

    Option(Option&&)
        requires(!std::is_move_constructible_v<T>)
    = delete;

    constexpr ~Option()
        requires std::is_trivially_destructible_v<T>
    = default;

    constexpr ~Option() noexcept(std::is_nothrow_destructible_v<T>)
        requires(!std::is_trivially_destructible_v<T>)
    {
        this->reset();
    }

    constexpr auto operator=(const Option&) -> Option&
        requires trivial_copy_assignment
    = default;

    constexpr auto operator=(const Option& other) noexcept(
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T> &&
        std::is_nothrow_destructible_v<T>
    ) -> Option&
        requires(
            std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T> &&
            !trivial_copy_assignment
        )
    {
        if (this == std::addressof(other)) { return *this; }
        if (!other.engaged_) {
            this->reset();
        } else if (engaged_) {
            storage_.value = other.storage_.value;
        } else {
            this->construct(other.storage_.value);
        }
        return *this;
    }

    auto operator=(const Option&) -> Option&
        requires(!std::is_copy_constructible_v<T> || !std::is_copy_assignable_v<T>)
    = delete;

    constexpr auto operator=(Option&&) -> Option&
        requires trivial_move_assignment
    = default;

    constexpr auto operator=(Option&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T> &&
        std::is_nothrow_destructible_v<T>
    ) -> Option&
        requires(
            std::is_move_constructible_v<T> && std::is_move_assignable_v<T> &&
            !trivial_move_assignment
        )
    {
        if (this == std::addressof(other)) { return *this; }
        if (!other.engaged_) {
            this->reset();
        } else if (engaged_) {
            storage_.value = std::move(other.storage_.value);
        } else {
            this->construct(std::move(other).value());
        }
        return *this;
    }

    auto operator=(Option&&) -> Option&
        requires(!std::is_move_constructible_v<T> || !std::is_move_assignable_v<T>)
    = delete;

    constexpr auto operator=(NoneTag) noexcept(std::is_nothrow_destructible_v<T>)
        -> Option& {
        this->reset();
        return *this;
    }

    constexpr auto reset() noexcept(std::is_nothrow_destructible_v<T>) -> void {
        if (engaged_) {
            // Clear first: even a throwing destructor ends the payload lifetime.
            engaged_ = false;
            std::destroy_at(std::addressof(storage_.value));
        }
    }

    template<class... Args>
        requires std::is_constructible_v<T, Args&&...>
    constexpr auto emplace(Args&&... args) noexcept(
        std::is_nothrow_destructible_v<T> && std::is_nothrow_constructible_v<T, Args&&...>
    ) -> T& {
        this->reset();
        this->construct(std::forward<Args>(args)...);
        return storage_.value;
    }

    [[nodiscard]] constexpr auto take() & noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>
    ) -> Option
        requires std::is_move_constructible_v<T>
    {
        if (!engaged_) { return Option{}; }
        return Option(TakeTag{}, *this);
    }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool { return engaged_; }

    constexpr auto value() & noexcept -> T& {
        if (!engaged_) { details::fatal_bad_option_access(); }
        return storage_.value;
    }

    constexpr auto value() const& noexcept -> const T& {
        if (!engaged_) { details::fatal_bad_option_access(); }
        return storage_.value;
    }

    constexpr auto value() && noexcept -> T&& {
        if (!engaged_) { details::fatal_bad_option_access(); }
        return std::move(storage_.value);
    }

    constexpr auto value() const&& noexcept -> const T&& {
        if (!engaged_) { details::fatal_bad_option_access(); }
        return std::move(storage_.value);
    }

    constexpr auto operator*() & noexcept -> T& {
        assert(engaged_);
        return storage_.value;
    }

    constexpr auto operator*() const& noexcept -> const T& {
        assert(engaged_);
        return storage_.value;
    }

    constexpr auto operator*() && noexcept -> T&& {
        assert(engaged_);
        return std::move(storage_.value);
    }

    constexpr auto operator*() const&& noexcept -> const T&& {
        assert(engaged_);
        return std::move(storage_.value);
    }

    constexpr auto operator->() noexcept -> T* {
        assert(engaged_);
        return std::addressof(storage_.value);
    }

    constexpr auto operator->() const noexcept -> const T* {
        assert(engaged_);
        return std::addressof(storage_.value);
    }
};

template<class T>
class Option<T&> {
    static_assert(std::is_object_v<T>, "Option<T&> requires an object referent");
    T* pointer_ = nullptr;

public:
    using value_type = T&;
    constexpr Option() noexcept = default;

    explicit constexpr Option(NoneTag) noexcept {}

    template<class U>
        requires details::safe_reference_binding<T, U>
    explicit constexpr Option(U&& value) noexcept
        : pointer_(std::addressof(value)) {}

    template<class U>
        requires details::safe_reference_binding<T, U>
    explicit constexpr Option(InPlaceTag, U&& value) noexcept
        : pointer_(std::addressof(value)) {}

    constexpr auto operator=(NoneTag) noexcept -> Option& {
        this->reset();
        return *this;
    }

    template<class U>
        requires details::safe_reference_binding<T, U>
    constexpr auto rebind(U&& value) noexcept -> void {
        pointer_ = std::addressof(value);
    }

    constexpr auto reset() noexcept -> void { pointer_ = nullptr; }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool {
        return pointer_ != nullptr;
    }

    constexpr auto value() const noexcept -> T& {
        if (!pointer_) { details::fatal_bad_option_access(); }
        return *pointer_;
    }

    constexpr auto operator*() const noexcept -> T& {
        assert(pointer_);
        return *pointer_;
    }

    constexpr auto operator->() const noexcept -> T* {
        assert(pointer_);
        return pointer_;
    }

    [[nodiscard]] constexpr auto take() & noexcept -> Option {
        Option previous(*this);
        this->reset();
        return previous;
    }
};

template<>
class Option<void> {
    bool engaged_ = false;

public:
    using value_type = void;
    constexpr Option() noexcept = default;

    explicit constexpr Option(NoneTag) noexcept {}

    explicit constexpr Option(InPlaceTag) noexcept
        : engaged_(true) {}

    constexpr auto operator=(NoneTag) noexcept -> Option& {
        this->reset();
        return *this;
    }

    constexpr auto reset() noexcept -> void { engaged_ = false; }

    constexpr auto emplace() noexcept -> void { engaged_ = true; }

    [[nodiscard]] constexpr auto has_value() const noexcept -> bool { return engaged_; }

    constexpr auto value() const noexcept -> void {
        if (!engaged_) { details::fatal_bad_option_access(); }
    }

    [[nodiscard]] constexpr auto take() & noexcept -> Option {
        Option previous(*this);
        this->reset();
        return previous;
    }
};

} // namespace option

using option::Option;

template<class T, class... Args>
    requires requires(Args&&... args) { Option<T>{InPlace, std::forward<Args>(args)...}; }
[[nodiscard]] constexpr auto
some(Args&&... args) noexcept(noexcept(Option<T>{InPlace, std::forward<Args>(args)...}))
    -> Option<T> {
    return Option<T>{InPlace, std::forward<Args>(args)...};
}

template<class T>
[[nodiscard]] constexpr auto none() noexcept -> Option<T> {
    return Option<T>{};
}

} // namespace JadeFrame
