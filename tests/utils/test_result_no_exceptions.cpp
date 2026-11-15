#include "JadeFrame/utils/result.h"

#include <type_traits>
#include <utility>

using namespace JadeFrame;

namespace {
struct Stable {
    int value;

    explicit constexpr Stable(int n) noexcept
        : value(n) {}

    constexpr Stable(const Stable& other) noexcept(false)
        : value(other.value) {}

    constexpr Stable(Stable&& other) noexcept
        : value(other.value) {
        other.value = -1;
    }

    constexpr ~Stable() noexcept {}

    constexpr auto operator=(const Stable&) -> Stable& = default;
    constexpr auto operator=(Stable&&) -> Stable& = default;
};

struct Fragile {
    int value;

    explicit constexpr Fragile(int n) noexcept
        : value(n) {}

    constexpr Fragile(const Fragile& other) noexcept(false)
        : value(other.value) {}

    constexpr Fragile(Fragile&& other) noexcept(false)
        : value(other.value) {
        other.value = -1;
    }

    constexpr ~Fragile() noexcept {}

    constexpr auto operator=(const Fragile&) -> Fragile& = default;
    constexpr auto operator=(Fragile&&) -> Fragile& = default;
};

// Exercise all three transition strategies, including the armed rollback guard,
// in constant evaluation and with exception syntax disabled by the compiler.
constexpr auto exercise() -> bool {
    Result<Fragile, Stable> a{Ok, 1}, b{Err, 2};
    b = a; // Backup the old Stable, then construct Fragile.
    a.emplace_err(3);
    a = b;
    a.emplace_err(4);
    a = std::move(b);
    Result<Stable, Fragile> c{Ok, 5}, d{Err, 6};
    d = c; // Prepare a Stable temporary before destroying Fragile.
    c.emplace_err(7);
    c = d;
    c.emplace_err(8);
    c = std::move(d); // Direct nonthrowing move construction.
    int                value = 10;
    Result<int&, void> ref{Ok, value};
    auto               moved = std::move(ref);
    moved.value() = 11;
    Result<void, void> unit{Err};
    unit.emplace_ok();
    return a.value().value == 1 && c.value().value == 5 && value == 11 && ref.is_ok() &&
           unit.ok().has_value();
}

static_assert(exercise());
} // namespace

int main() { return exercise() ? 0 : 1; }
