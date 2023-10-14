#include <memory>
#include <concepts>
#include <type_traits>
#include "JadeFrame/utils/option.h"

namespace JadeFrame {
namespace ptr {

template<typename T>
class Scope {

    // static_assert(!std::copy_constructible<Scope<T>>, "Implementation error: Scope MUST
    // NOT be copy constructible");
    // // static_assert(!std::is_copy_assignable_v<Scope<T>>, "Implementation error: Scope
    // MUST not be copy assignable"); static_assert(std::move_constructible<Scope<T>>,
    // "Implementation error: Scope MUST be move constructible");
    // // static_assert(std::is_move_assignable_v<Scope<T>>, "Implementation error: Scope
    // MUST be move assignable");

public:
    Scope() noexcept
        : m_pointer(nullptr) {

        static_assert(
            std::move_constructible<Scope>,
            "Implementation error: Scope MUST be move constructible"
        );
        static_assert(
            !std::copy_constructible<Scope>,
            "Implementation error: Scope MUST NOT be copy constructible"
        );

        static_assert(
            !std::is_copy_constructible_v<Scope>,
            "Implementation error: Scope MUST NOT be copy constructible"
        );
        static_assert(
            !std::is_copy_assignable_v<Scope>,
            "Implementation error: Scope MUST not be copy assignable"
        );
        static_assert(
            std::is_move_constructible_v<Scope>,
            "Implementation error: Scope MUST be move constructible"
        );
        static_assert(
            std::is_move_assignable_v<Scope>,
            "Implementation error: Scope MUST be move assignable"
        );

        static_assert(
            std::is_nothrow_move_constructible_v<Scope>,
            "Implementation error: Scope MUST be noexcept move constructible"
        );
        static_assert(
            std::is_nothrow_move_assignable_v<Scope>,
            "Implementation error: Scope MUST be noexcept move assignable"
        );
    }

    Scope(const Scope&) = delete;

    Scope(Scope&& rhs) noexcept
        : m_pointer(rhs.m_pointer) {
        rhs.m_pointer = nullptr;
    }

    ~Scope() noexcept {
        if (m_pointer) { delete m_pointer; }
    }

    explicit Scope(T* p)
        : m_pointer(p) {}

    auto operator=(const Scope&) -> Scope& = delete;

    auto operator=(Scope&& rhs) noexcept -> Scope& {
        m_pointer = rhs.m_pointer;
        rhs.m_pointer = nullptr;
        return *this;
    }

    constexpr auto get() const noexcept -> T* { return m_pointer; }

    constexpr auto operator*() const noexcept -> T& { return *this->get(); }

    constexpr auto operator->() const noexcept -> T* { return this->get(); }

private:
    T* m_pointer;
};

template<typename T, typename... Args>
constexpr auto make_scope(Args&&... args) -> Scope<T> {
    return Scope<T>(new T(std::forward<Args>(args)...));
}

template<typename T, typename... Args>
constexpr auto make_scope_noexcept(Args&&... args) noexcept -> Option<Scope<T>> {
    T* t = new (std::nothrow) T(std::forward<Args>(args)...);
    if (t != nullptr) {
        return Scope<T>(t);
    } else {
        return Option<Scope<T>>();
    }
}

} // namespace ptr
} // namespace JadeFrame