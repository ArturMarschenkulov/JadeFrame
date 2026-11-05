#include "JadeFrame/utils/option.h"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace JadeFrame;

namespace {
struct Tracked {
    static inline int  live = 0, constructed = 0, destroyed = 0;
    static inline int  copies = 0, moves = 0, copy_assignments = 0, move_assignments = 0;
    static inline bool fail_construct = false, fail_copy = false, fail_move = false;
    static inline bool fail_copy_assign = false, fail_move_assign = false,
                       fail_destroy = false;
    int                value;

    static void fail(bool enabled) {
#if defined(__cpp_exceptions)
        if (enabled) { throw std::runtime_error("payload failure"); }
#else
        (void)enabled;
#endif
    }

    static void clear() {
        live = constructed = destroyed = copies = moves = copy_assignments =
            move_assignments = 0;
        fail_construct = fail_copy = fail_move = fail_copy_assign = fail_move_assign =
            fail_destroy = false;
    }

    explicit Tracked(int n)
        : value(n) {
        fail(fail_construct);
        ++live;
        ++constructed;
    }

    Tracked(const Tracked& other)
        : value(other.value) {
        fail(fail_copy);
        ++copies;
        ++live;
        ++constructed;
    }

    Tracked(Tracked&& other)
        : value(other.value) {
        other.value = -1;
        fail(fail_move);
        ++moves;
        ++live;
        ++constructed;
    }

    Tracked& operator=(const Tracked& other) {
        value = other.value;
        fail(fail_copy_assign);
        ++copy_assignments;
        return *this;
    }

    Tracked& operator=(Tracked&& other) {
        value = other.value;
        other.value = -1;
        fail(fail_move_assign);
        ++move_assignments;
        return *this;
    }

    ~Tracked() noexcept(false) {
        --live;
        ++destroyed;
        fail(fail_destroy);
    }
};

static_assert(!std::is_nothrow_move_constructible_v<Option<Tracked>>);
static_assert(!std::is_nothrow_copy_constructible_v<Option<Tracked>>);
static_assert(!noexcept(std::declval<Option<Tracked>&>().take()));
static_assert(
    !noexcept(std::declval<Option<Tracked>&>() = std::declval<Option<Tracked>&&>())
);

struct alignas(128) Aligned {
    int n;
};

static_assert(alignof(Option<Aligned>) >= alignof(Aligned));
} // namespace

TEST(OptionLifetime, EveryLiveObjectIsDestroyedExactlyOnce) {
    Tracked::clear();
    {
        Option<Tracked> a;
        a.reset();
        a.emplace(1);
        EXPECT_EQ(Tracked::live, 1);
        auto b = a;
        auto c = std::move(a);
        EXPECT_TRUE(a.has_value());
        EXPECT_EQ(Tracked::live, 3);
        EXPECT_EQ(Tracked::copies, 1);
        EXPECT_EQ(Tracked::moves, 1);
        b = c;
        a = std::move(c);
        EXPECT_EQ(Tracked::copy_assignments, 1);
        EXPECT_EQ(Tracked::move_assignments, 1);
        EXPECT_EQ(Tracked::constructed, 3);
        b.emplace(2);
        EXPECT_EQ(Tracked::destroyed, 1);
        auto old = b.take();
        EXPECT_EQ(Tracked::moves, 2); // Exactly one move even without NRVO.
        EXPECT_FALSE(b.has_value());
        a = None;
        a.reset();
        EXPECT_EQ(Tracked::live, 2);
        Option<Tracked&> ref(*old);
        auto             moved_ref = std::move(ref);
        ref = moved_ref;
        auto prior_ref = ref.take();
        EXPECT_EQ(prior_ref.operator->(), old.operator->());
        EXPECT_EQ(Tracked::constructed, 5);
        EXPECT_EQ(Tracked::live, 2);
    }
    EXPECT_EQ(Tracked::live, 0);
    EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
    auto aligned = some<Aligned>(Aligned{8});
    EXPECT_EQ(aligned->n, 8);
}

#if defined(__cpp_exceptions)
TEST(OptionExceptions, FailedConstructionNeverCreatesALivePayload) {
    Tracked::clear();
    {
        auto a = some<Tracked>(1);
        Tracked::fail_construct = true;
        EXPECT_THROW(a.emplace(2), std::runtime_error);
        EXPECT_FALSE(a.has_value());
        EXPECT_EQ(Tracked::live, 0);
        EXPECT_THROW((Option<Tracked>{InPlace, 3}), std::runtime_error);
        Tracked::fail_construct = false;
        a.emplace(4);
        Tracked::fail_copy = true;
        EXPECT_THROW((Option<Tracked>{a}), std::runtime_error);
        Option<Tracked> b;
        EXPECT_THROW(b = a, std::runtime_error);
        EXPECT_FALSE(b.has_value());
        Tracked::fail_copy = false;
        Tracked::fail_move = true;
        EXPECT_THROW((Option<Tracked>{std::move(a)}), std::runtime_error);
        EXPECT_THROW(b = std::move(a), std::runtime_error);
        EXPECT_THROW((void)a.take(), std::runtime_error);
        EXPECT_FALSE(b.has_value());
        EXPECT_TRUE(a.has_value());
        EXPECT_EQ(Tracked::live, 1);
        Tracked::fail_move = false;
        b.emplace(5);
    }
    EXPECT_EQ(Tracked::live, 0);
    EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
}

TEST(OptionExceptions, FailedAssignmentsRetainBothEngagementStates) {
    Tracked::clear();
    {
        auto a = some<Tracked>(1);
        auto b = some<Tracked>(2);
        Tracked::fail_copy_assign = true;
        EXPECT_THROW(a = b, std::runtime_error);
        EXPECT_TRUE(a.has_value());
        EXPECT_TRUE(b.has_value());
        Tracked::fail_move_assign = true;
        EXPECT_THROW(a = std::move(b), std::runtime_error);
        EXPECT_TRUE(a.has_value());
        EXPECT_TRUE(b.has_value());
        EXPECT_EQ(Tracked::live, 2);
        EXPECT_EQ(Tracked::destroyed, 0);
    }
    EXPECT_EQ(Tracked::live, 0);
    EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
}

TEST(OptionExceptions, ThrowingDestructionClearsStateAndCleansUpTakeResult) {
    Tracked::clear();
    {
        auto a = some<Tracked>(1);
        Tracked::fail_destroy = true;
        EXPECT_THROW(a.reset(), std::runtime_error);
        EXPECT_FALSE(a.has_value());
        Tracked::fail_destroy = false;
        a.reset();
        a.emplace(2);

        // Only the source destructor throws; destroying the completed destination
        // while unwinding must succeed.
        struct ThrowsOnce {
            int* count;

            explicit ThrowsOnce(int& n)
                : count(&n) {
                ++*count;
            }

            ThrowsOnce(ThrowsOnce&& other)
                : count(other.count) {
                ++*count;
            }

            ~ThrowsOnce() noexcept(false) {
                if (--*count == 1) { throw std::runtime_error("source destruction"); }
            }
        };

        int                count = 0;
        Option<ThrowsOnce> source{InPlace, count};
        EXPECT_THROW((void)source.take(), std::runtime_error);
        EXPECT_FALSE(source.has_value());
        EXPECT_EQ(count, 0);
    }
    EXPECT_EQ(Tracked::live, 0);
    EXPECT_EQ(Tracked::constructed, Tracked::destroyed);
}
#endif
