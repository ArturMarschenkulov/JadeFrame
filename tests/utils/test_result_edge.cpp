#include "JadeFrame/utils/result.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace JadeFrame;

namespace {

// Each constructor increments live only after all possible failure points.
template<bool NoThrowMove, bool NoThrowCopy = false>
struct Probe {
    inline static int                   live = 0;
    [[maybe_unused]] inline static int  destroyed = 0;
    inline static int                   copies = 0;
    inline static int                   moves = 0;
    [[maybe_unused]] inline static bool fail_copy = false;
    inline static bool                  fail_move = false;
    inline static bool                  fail_copy_assignment = false;
    inline static bool                  fail_move_assignment = false;
    inline static bool                  fail_construction = false;
    int                                 value;

    explicit Probe(int n)
        : value(n) {
        if (fail_construction) { throw std::runtime_error("construction"); }
        ++live;
    }

    Probe(const Probe& other) noexcept(NoThrowCopy)
        : value(other.value) {
        ++copies;
        if constexpr (!NoThrowCopy) {
            if (fail_copy) { throw std::runtime_error("copy"); }
        }
        ++live;
    }

    Probe(Probe&& other) noexcept(NoThrowMove)
        : value(other.value) {
        ++moves;
        other.value = -1;
        if constexpr (!NoThrowMove) {
            if (fail_move) { throw std::runtime_error("move"); }
        }
        ++live;
    }

    auto operator=(const Probe& other) -> Probe& {
        value = other.value;
        if (fail_copy_assignment) { throw std::runtime_error("copy assignment"); }
        return *this;
    }

    auto operator=(Probe&& other) -> Probe& {
        value = other.value;
        other.value = -1;
        if (fail_move_assignment) { throw std::runtime_error("move assignment"); }
        return *this;
    }

    ~Probe() noexcept {
        --live;
        ++destroyed;
    }

    static auto reset() -> void {
        EXPECT_EQ(live, 0);
        destroyed = copies = moves = 0;
        fail_copy = fail_move = fail_copy_assignment = fail_move_assignment =
            fail_construction = false;
    }
};

using Stable = Probe<true>;
using Fragile = Probe<false>;
using DirectCopy = Probe<false, true>;

static_assert(std::is_copy_constructible_v<Result<Fragile, Fragile>>);
static_assert(std::is_move_constructible_v<Result<Fragile, Fragile>>);
static_assert(!std::is_copy_assignable_v<Result<Fragile, Fragile>>);
static_assert(!std::is_move_assignable_v<Result<Fragile, Fragile>>);
static_assert(std::is_copy_assignable_v<Result<Stable, Fragile>>);
static_assert(std::is_move_assignable_v<Result<Stable, Fragile>>);
static_assert(std::is_copy_assignable_v<Result<DirectCopy, DirectCopy>>);
static_assert(!std::is_move_assignable_v<Result<DirectCopy, DirectCopy>>);
static_assert(!std::is_nothrow_copy_constructible_v<Result<Stable, int>>);
static_assert(std::is_nothrow_move_constructible_v<Result<Stable, int>>);
static_assert(!std::is_nothrow_move_constructible_v<Result<Fragile, int>>);
static_assert(!std::is_nothrow_copy_assignable_v<Result<Stable, int>>);
static_assert(!std::is_nothrow_move_assignable_v<Result<Stable, int>>);
static_assert(!noexcept(ok<Stable, int>(1)));
static_assert(!noexcept(err<int, Stable>(1)));
static_assert(!noexcept(std::declval<const Result<Stable, int>&>().ok()));
static_assert(noexcept(std::declval<Result<Stable, int>&&>().ok()));
static_assert(!noexcept(std::declval<Result<Fragile, int>&&>().ok()));
template<class R>
concept ThrowingEmplacement = requires(R& r) { r.emplace_ok(1); };
static_assert(!ThrowingEmplacement<Result<Stable, int>>);

// Mirroring the payload positions exercises the centralized transition in both
// directions.
template<bool NewOk, class New, class Old>
auto copy_transition_test() -> void {
    using R = std::conditional_t<NewOk, Result<New, Old>, Result<Old, New>>;
    New::reset();
    Old::reset();
    {
        R source = [] {
            if constexpr (NewOk) {
                return R{Ok, 20};
            } else {
                return R{Err, 20};
            }
        }();
        R destination = [] {
            if constexpr (NewOk) {
                return R{Err, 10};
            } else {
                return R{Ok, 10};
            }
        }();
        New::fail_copy = true;
        EXPECT_THROW(destination = source, std::runtime_error);
        New::fail_copy = false;
        EXPECT_EQ(destination.is_ok(), !NewOk);
        EXPECT_NE(destination.is_ok(), destination.is_err());
        EXPECT_EQ(source.is_ok(), NewOk);
        if constexpr (NewOk) {
            EXPECT_EQ(destination.error().value, 10);
        } else {
            EXPECT_EQ(destination.value().value, 10);
        }
        EXPECT_EQ(New::live, 1);
        EXPECT_EQ(Old::live, 1);
        destination = source;
        EXPECT_EQ(destination.is_ok(), NewOk);
        if constexpr (NewOk) {
            EXPECT_EQ(destination.value().value, 20);
        } else {
            EXPECT_EQ(destination.error().value, 20);
        }
        EXPECT_EQ(New::live, 2);
        EXPECT_EQ(Old::live, 0);
    }
    EXPECT_EQ(New::live, 0);
    EXPECT_EQ(Old::live, 0);
}

template<bool NewOk>
auto move_rollback_test() -> void {
    using R = std::conditional_t<NewOk, Result<Fragile, Stable>, Result<Stable, Fragile>>;
    Fragile::reset();
    Stable::reset();
    {
        R source = [] {
            if constexpr (NewOk) {
                return R{Ok, 20};
            } else {
                return R{Err, 20};
            }
        }();
        R destination = [] {
            if constexpr (NewOk) {
                return R{Err, 10};
            } else {
                return R{Ok, 10};
            }
        }();
        Fragile::fail_move = true;
        EXPECT_THROW(destination = std::move(source), std::runtime_error);
        Fragile::fail_move = false;
        EXPECT_EQ(destination.is_ok(), !NewOk);
        EXPECT_EQ(source.is_ok(), NewOk);
        EXPECT_EQ(Stable::moves, 2); // Backup and restoration.
        EXPECT_EQ(Stable::live, 1);
        EXPECT_EQ(Fragile::live, 1);
        if constexpr (NewOk) {
            EXPECT_EQ(destination.error().value, 10);
        } else {
            EXPECT_EQ(destination.value().value, 10);
        }
        destination = std::move(source);
        EXPECT_EQ(destination.is_ok(), NewOk);
        EXPECT_EQ(source.is_ok(), NewOk);
        EXPECT_EQ(Fragile::live, 2);
        EXPECT_EQ(Stable::live, 0);
    }
    EXPECT_EQ(Fragile::live, 0);
    EXPECT_EQ(Stable::live, 0);
}

} // namespace

TEST(ResultEdge, CrossStateCopyPreparesReplacementBeforeDestruction) {
    copy_transition_test<true, Stable, Fragile>();
    EXPECT_EQ(Fragile::moves, 0); // No backup necessary.
    copy_transition_test<false, Stable, Fragile>();
    EXPECT_EQ(Fragile::moves, 0);
}

TEST(ResultEdge, CrossStateCopyRollsBackOldAlternative) {
    copy_transition_test<true, Fragile, Stable>();
    EXPECT_EQ(
        Stable::moves, 3
    ); // Failed attempt backs up and restores; success backs up.
    copy_transition_test<false, Fragile, Stable>();
    EXPECT_EQ(Stable::moves, 3);
}

TEST(ResultEdge, CrossStateMoveRollsBackOldAlternative) {
    move_rollback_test<true>();
    move_rollback_test<false>();
}

TEST(ResultEdge, DirectNothrowConstructionNeedsNoMove) {
    DirectCopy::reset();
    {
        Result<DirectCopy, DirectCopy> a{Ok, 1}, b{Err, 2};
        a = b;
        EXPECT_TRUE(a.is_err());
        EXPECT_EQ(a.error().value, 2);
        Result<DirectCopy, DirectCopy> c{Ok, 3};
        b = c;
        EXPECT_TRUE(b.is_ok());
        EXPECT_EQ(b.value().value, 3);
        EXPECT_EQ(DirectCopy::moves, 0);
    }
    EXPECT_EQ(DirectCopy::live, 0);
}

TEST(ResultEdge, SameStateAssignmentFailuresKeepAlternativeAndLifetime) {
    for (bool success : {false, true}) {
        Stable::reset();
        {
            Result<Stable, Stable> source{Ok, 20}, destination{Ok, 10};
            if (!success) {
                source = Result<Stable, Stable>{Err, 20};
                destination = Result<Stable, Stable>{Err, 10};
            }
            Stable::fail_copy_assignment = true;
            EXPECT_THROW(destination = source, std::runtime_error);
            Stable::fail_copy_assignment = false;
            EXPECT_EQ(destination.is_ok(), success);
            EXPECT_EQ(source.is_ok(), success);
            EXPECT_EQ(Stable::live, 2);
            Stable::fail_move_assignment = true;
            EXPECT_THROW(destination = std::move(source), std::runtime_error);
            Stable::fail_move_assignment = false;
            EXPECT_EQ(destination.is_ok(), success);
            EXPECT_EQ(source.is_ok(), success);
            EXPECT_EQ(Stable::live, 2);
            // Even hostile payload assignment is never invoked on self assignment.
            Stable::fail_copy_assignment = Stable::fail_move_assignment = true;
            auto& alias = destination;
            EXPECT_NO_THROW(destination = alias);
            EXPECT_NO_THROW(destination = std::move(alias));
            Stable::fail_copy_assignment = Stable::fail_move_assignment = false;
        }
        EXPECT_EQ(Stable::live, 0);
    }
}

TEST(ResultEdge, ConstructorFailuresDestroyNoInactiveMember) {
    for (bool success : {false, true}) {
        Fragile::reset();
        {
            Fragile::fail_construction = true;
            if (success) {
                EXPECT_THROW(
                    ((void)Result<Fragile, Fragile>{Ok, 10}), std::runtime_error
                );
            } else {
                EXPECT_THROW(
                    ((void)Result<Fragile, Fragile>{Err, 10}), std::runtime_error
                );
            }
            Fragile::fail_construction = false;
            EXPECT_EQ(Fragile::live, 0);
            EXPECT_EQ(Fragile::destroyed, 0);
            auto source = success ? Result<Fragile, Fragile>{Ok, 20}
                                  : Result<Fragile, Fragile>{Err, 20};
            Fragile::fail_copy = true;
            EXPECT_THROW(((void)Result<Fragile, Fragile>{source}), std::runtime_error);
            Fragile::fail_copy = false;
            Fragile::fail_move = true;
            EXPECT_THROW(
                ((void)Result<Fragile, Fragile>{std::move(source)}), std::runtime_error
            );
            Fragile::fail_move = false;
            EXPECT_EQ(source.is_ok(), success);
            EXPECT_EQ(Fragile::live, 1);
            EXPECT_EQ(Fragile::destroyed, 0);
        }
        EXPECT_EQ(Fragile::live, 0);
        EXPECT_EQ(Fragile::destroyed, 1);
    }
}

TEST(ResultEdge, FailedOptionConversionsPreserveStateAndLifetime) {
    Fragile::reset();
    {
        Result<Fragile, Fragile> a{Ok, 1}, b{Err, 2};
        Fragile::fail_copy = true;
        EXPECT_THROW((void)a.ok(), std::runtime_error);
        EXPECT_THROW((void)b.err(), std::runtime_error);
        EXPECT_FALSE(a.err().has_value());
        EXPECT_FALSE(b.ok().has_value());
        Fragile::fail_copy = false;
        Fragile::fail_move = true;
        EXPECT_THROW((void)std::move(a).ok(), std::runtime_error);
        EXPECT_THROW((void)std::move(b).err(), std::runtime_error);
        Fragile::fail_move = false;
        EXPECT_TRUE(a.is_ok());
        EXPECT_TRUE(b.is_err());
        EXPECT_EQ(Fragile::live, 2);
    }
    EXPECT_EQ(Fragile::live, 0);
}

namespace {
struct ImmovableCounter {
    inline static int live = 0;
    inline static int constructed = 0;
    inline static int destroyed = 0;
    int               value;

    explicit ImmovableCounter(int n) noexcept
        : value(n) {
        ++live;
        ++constructed;
    }

    ImmovableCounter(const ImmovableCounter&) = delete;
    ImmovableCounter(ImmovableCounter&&) = delete;

    ~ImmovableCounter() noexcept {
        --live;
        ++destroyed;
    }
};
} // namespace

TEST(ResultEdge, EmplacementReconstructsAndDestroysOnlyActivePayload) {
    ImmovableCounter::constructed = ImmovableCounter::destroyed = 0;
    {
        Result<ImmovableCounter, ImmovableCounter> r{Ok, 1};
        EXPECT_EQ(ImmovableCounter::live, 1);
        EXPECT_EQ(r.emplace_ok(2).value, 2);
        EXPECT_EQ(ImmovableCounter::destroyed, 1);
        EXPECT_EQ(r.emplace_err(3).value, 3);
        EXPECT_TRUE(r.is_err());
        EXPECT_EQ(ImmovableCounter::destroyed, 2);
        EXPECT_EQ(r.emplace_err(4).value, 4);
        EXPECT_EQ(ImmovableCounter::destroyed, 3);
        EXPECT_EQ(r.emplace_ok(5).value, 5);
        EXPECT_EQ(ImmovableCounter::live, 1);
    }
    EXPECT_EQ(ImmovableCounter::live, 0);
    EXPECT_EQ(ImmovableCounter::constructed, 5);
    EXPECT_EQ(ImmovableCounter::destroyed, 5);
}
