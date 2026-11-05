#include <gtest/gtest.h>

#include "JadeFrame/utils/option.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using namespace JadeFrame;

namespace {

struct MoveOnly {
    explicit MoveOnly(int value)
        : value(value) {}

    MoveOnly(const MoveOnly&) = delete;
    auto operator=(const MoveOnly&) -> MoveOnly& = delete;
    MoveOnly(MoveOnly&&) noexcept = default;
    auto operator=(MoveOnly&&) noexcept -> MoveOnly& = default;

    int value;
};

struct LifetimeCounter {
    static inline int live_count = 0;

    explicit LifetimeCounter(int value)
        : value(value) {
        ++live_count;
    }

    LifetimeCounter(const LifetimeCounter& other)
        : value(other.value) {
        ++live_count;
    }

    LifetimeCounter(LifetimeCounter&& other) noexcept
        : value(other.value) {
        ++live_count;
        other.value = -1;
    }

    ~LifetimeCounter() { --live_count; }

    auto operator=(const LifetimeCounter& other) -> LifetimeCounter& {
        value = other.value;
        return *this;
    }

    auto operator=(LifetimeCounter&& other) noexcept -> LifetimeCounter& {
        value = other.value;
        other.value = -1;
        return *this;
    }

    int value;
};

struct ThrowingValue {
    static inline bool should_throw = false;

    explicit ThrowingValue(int value) {
        if (should_throw) { throw std::runtime_error("construction failed"); }
        this->value = value;
    }

    ThrowingValue(const ThrowingValue&) = default;
    ThrowingValue(ThrowingValue&&) = default;
    auto operator=(const ThrowingValue&) -> ThrowingValue& = default;
    auto operator=(ThrowingValue&&) -> ThrowingValue& = default;

    int value = 0;
};

static_assert([] {
    constexpr Option<int> none;
    constexpr Option<int> some(4);
    return !none.has_value() && some.has_value() && some.unwrap() == 4;
}());

static_assert(std::is_trivially_destructible_v<Option<int>>);
static_assert(std::is_default_constructible_v<Option<MoveOnly>>);

} // namespace

TEST(OptionEdge, ExplicitFactoriesAndNoneTag) {
    auto value = Some(42);
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(value.unwrap(), 42);

    Option<int> none = None;
    EXPECT_TRUE(!none.has_value());
    EXPECT_TRUE(none == None);

    auto factory_value = Option<int>::some(7);
    auto factory_none = Option<int>::none();
    EXPECT_EQ(factory_value.unwrap(), 7);
    EXPECT_TRUE(!factory_none.has_value());
}

TEST(OptionEdge, CopyAssignmentHandlesEveryStateTransition) {
    Option<int> source(12);
    Option<int> destination;

    destination = source;
    ASSERT_TRUE(destination.has_value());
    EXPECT_EQ(destination.unwrap(), 12);

    source.reset();
    destination = source;
    EXPECT_TRUE(!destination.has_value());

    source = Option<int>(19);
    destination = source;
    EXPECT_EQ(destination.unwrap(), 19);

    destination = destination;
    EXPECT_EQ(destination.unwrap(), 19);
}

TEST(OptionEdge, MoveOnlyPayloadAndStateChangingOperations) {
    Option<MoveOnly> option(MoveOnly(10));
    auto mapped = std::move(option).map([](MoveOnly value) { return value.value * 2; });

    EXPECT_TRUE(!option.has_value());
    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(mapped.unwrap(), 20);

    Option<std::unique_ptr<int>> pointer(std::make_unique<int>(3));
    auto                         old = pointer.replace(std::make_unique<int>(8));
    ASSERT_TRUE(old.has_value());
    ASSERT_TRUE(pointer.has_value());
    EXPECT_EQ(*old.unwrap(), 3);
    EXPECT_EQ(*pointer.unwrap(), 8);

    Option<std::unique_ptr<int>> empty;
    auto& inserted = empty.get_or_insert(std::make_unique<int>(11));
    ASSERT_TRUE(inserted);
    EXPECT_EQ(*inserted, 11);

    Option<MoveOnly> chain_source(MoveOnly(4));
    auto             chained = std::move(chain_source).and_then([](MoveOnly value) {
        return Option<int>(value.value + 1);
    });
    EXPECT_TRUE(!chain_source.has_value());
    EXPECT_EQ(chained.unwrap(), 5);
}

TEST(OptionEdge, NonTrivialLifetimeAndEmplaceException) {
    LifetimeCounter::live_count = 0;
    {
        Option<LifetimeCounter> option;
        option.emplace(5);
        EXPECT_EQ(LifetimeCounter::live_count, 1);

        Option<LifetimeCounter> copy(option);
        EXPECT_EQ(LifetimeCounter::live_count, 2);

        option.reset();
        EXPECT_EQ(LifetimeCounter::live_count, 1);
        EXPECT_EQ(copy.unwrap().value, 5);
    }
    EXPECT_EQ(LifetimeCounter::live_count, 0);

    Option<ThrowingValue> option(1);
    ThrowingValue::should_throw = true;
    EXPECT_THROW(option.emplace(2), std::runtime_error);
    EXPECT_TRUE(!option.has_value());
    ThrowingValue::should_throw = false;
}

TEST(OptionEdge, LazyCombinatorsOnlyInvokeWhenNeeded) {
    int         fallback_calls = 0;
    Option<int> some(5);
    Option<int> none;

    auto fallback = [&] {
        ++fallback_calls;
        return Option<int>(9);
    };

    EXPECT_EQ(some.or_else(fallback).unwrap(), 5);
    EXPECT_EQ(fallback_calls, 0);
    EXPECT_EQ(none.or_else(fallback).unwrap(), 9);
    EXPECT_EQ(fallback_calls, 1);

    int  map_calls = 0;
    auto mapped_none = none.map([&](int value) {
        ++map_calls;
        return value + 1;
    });
    EXPECT_TRUE(!mapped_none.has_value());
    EXPECT_EQ(map_calls, 0);
}

TEST(OptionEdge, ReferencesRemainNonOwningAndRebindable) {
    int first = 1;
    int second = 2;

    Option<int&>        reference(first);
    const Option<int&>& const_reference = reference;
    const_reference.unwrap() = 10;
    EXPECT_EQ(first, 10);

    reference = second;
    EXPECT_EQ(reference.unwrap(), 2);
    reference.reset();
    EXPECT_TRUE(!reference.has_value());

    auto& inserted = reference.get_or_insert(first);
    inserted = 15;
    EXPECT_EQ(first, 15);
}

TEST(OptionEdge, NestedAndAggregateOperations) {
    Option<int>         left(2);
    auto changed_type = left.and_then([](int number) {
        return Option<std::string>(std::to_string(number));
    });
    EXPECT_EQ(changed_type.unwrap(), "2");

    auto references = left.as_ref();
    ASSERT_TRUE(references.has_value());
    EXPECT_EQ(references.unwrap(), 2);

    auto mutable_reference = left.as_mut();
    mutable_reference.unwrap() = 3;
    EXPECT_EQ(left.unwrap(), 3);

    EXPECT_EQ(Option<int>(4).map_or(0, [](int value) { return value + 2; }), 6);
    EXPECT_EQ(Option<int>().map_or(9, [](int value) { return value + 2; }), 9);
    EXPECT_EQ(
        Option<int>().map_or_else([] { return 10; }, [](int value) { return value + 2; }),
        10
    );
}

TEST(OptionEdge, MoveAndSwapResetSources) {
    Option<int> source(7);
    Option<int> moved(std::move(source));
    EXPECT_TRUE(!source.has_value());
    EXPECT_EQ(moved.unwrap(), 7);

    Option<int> destination;
    destination = std::move(moved);
    EXPECT_TRUE(!moved.has_value());
    EXPECT_EQ(destination.unwrap(), 7);

    Option<int> first(1);
    Option<int> second(2);
    swap(first, second);
    EXPECT_EQ(first.unwrap(), 2);
    EXPECT_EQ(second.unwrap(), 1);
}

TEST(OptionEdge, WrongVariantAccessTerminates) {
    Option<int> none;
    EXPECT_DEATH({ none.unwrap(); }, "");
}
