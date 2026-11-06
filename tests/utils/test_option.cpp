#include "JadeFrame/utils/option.h"
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

using namespace JadeFrame;

namespace {
struct Immobile {
    explicit constexpr Immobile(int v)
        : value(v) {}

    Immobile(const Immobile&) = delete;
    Immobile(Immobile&&) = delete;
    auto operator=(const Immobile&) -> Immobile& = delete;
    auto operator=(Immobile&&) -> Immobile& = delete;
    int  value;
};

struct NonAssignable {
    explicit NonAssignable(int) {}

    NonAssignable(const NonAssignable&) = default;
    NonAssignable(NonAssignable&&) = default;
    auto operator=(const NonAssignable&) -> NonAssignable& = delete;
    auto operator=(NonAssignable&&) -> NonAssignable& = delete;
};

struct CopyOnly {
    CopyOnly() = default;
    CopyOnly(const CopyOnly&) = default;
    CopyOnly(CopyOnly&&) = delete;
    auto operator=(const CopyOnly&) -> CopyOnly& = default;
    auto operator=(CopyOnly&&) -> CopyOnly& = delete;
};

struct Nontrivial {
    Nontrivial() {}

    Nontrivial(const Nontrivial&) {}

    Nontrivial(Nontrivial&&) {}

    auto operator=(const Nontrivial&) -> Nontrivial& { return *this; }

    auto operator=(Nontrivial&&) -> Nontrivial& { return *this; }

    ~Nontrivial() {}
};

struct ThrowingDestructor {
    ~ThrowingDestructor() noexcept(false) {}
};

struct Derived : Nontrivial {};

struct ReferenceConversion {
    operator const std::string&() const;
};

template<class T>
concept ContextualBool = requires(T x) { x ? 1 : 0; };
template<class T>
concept Dereferenceable = requires(T x) { *x; };
template<class T>
concept Arrow = requires(T x) { x.operator->(); };
template<class T>
concept Takeable = requires(T& x) { x.take(); };
template<class T>
concept RvalueTakeable = requires(T x) { std::move(x).take(); };
template<class T, class U>
concept Rebindable = requires(T x, U&& v) { x.rebind(std::forward<U>(v)); };
template<class T, class... Args>
concept Someable = requires(Args&&... args) { some<T>(std::forward<Args>(args)...); };

static_assert(std::is_default_constructible_v<Option<int>>);
static_assert(std::is_default_constructible_v<Option<Immobile>>);
static_assert(std::is_constructible_v<Option<int>, int>);
static_assert(!std::is_convertible_v<int, Option<int>>);
static_assert(!std::is_convertible_v<NoneTag, Option<int>>);
static_assert(!std::is_convertible_v<NoneTag, Option<int&>>);
static_assert(!std::is_convertible_v<NoneTag, Option<void>>);
static_assert(!std::is_convertible_v<InPlaceTag, Option<void>>);
static_assert(!std::is_assignable_v<Option<int>&, int>);
static_assert(!std::is_assignable_v<Option<int&>&, int&>);
static_assert(
    !ContextualBool<Option<int>> && !ContextualBool<Option<int&>> &&
    !ContextualBool<Option<void>>
);
static_assert(!Dereferenceable<Option<void>> && !Arrow<Option<void>>);
static_assert(
    !RvalueTakeable<Option<int>> && !RvalueTakeable<Option<int&>> &&
    !RvalueTakeable<Option<void>>
);
static_assert(std::is_same_v<decltype(some<int>(42)), Option<int>>);
static_assert(std::is_same_v<decltype(none<int>()), Option<int>>);
static_assert(std::is_same_v<decltype(some<void>()), Option<void>>);
static_assert(!Someable<void, int>);
static_assert(!Someable<Immobile, std::string>);
static_assert(std::is_trivially_destructible_v<Option<int>>);
static_assert(std::is_trivially_copy_constructible_v<Option<int>>);
static_assert(std::is_trivially_move_constructible_v<Option<int>>);
static_assert(std::is_trivially_copy_assignable_v<Option<int>>);
static_assert(std::is_trivially_move_assignable_v<Option<int>>);
static_assert(std::is_trivially_copyable_v<Option<int>>);
static_assert(std::is_trivially_copyable_v<Option<int&>>);
static_assert(std::is_trivially_copyable_v<Option<void>>);
static_assert(!std::is_trivially_destructible_v<Option<Nontrivial>>);
static_assert(!std::is_trivially_copy_constructible_v<Option<Nontrivial>>);
static_assert(!std::is_trivially_move_constructible_v<Option<Nontrivial>>);
static_assert(!std::is_trivially_copy_assignable_v<Option<Nontrivial>>);
static_assert(!std::is_trivially_move_assignable_v<Option<Nontrivial>>);
static_assert(!std::is_copy_constructible_v<Option<Immobile>>);
static_assert(!std::is_move_constructible_v<Option<Immobile>>);
static_assert(!std::is_copy_assignable_v<Option<Immobile>>);
static_assert(!std::is_move_assignable_v<Option<Immobile>>);
static_assert(!Takeable<Option<Immobile>>);
static_assert(std::is_copy_constructible_v<Option<NonAssignable>>);
static_assert(std::is_move_constructible_v<Option<NonAssignable>>);
static_assert(!std::is_copy_assignable_v<Option<NonAssignable>>);
static_assert(!std::is_move_assignable_v<Option<NonAssignable>>);
static_assert(std::is_copy_constructible_v<Option<CopyOnly>>);
static_assert(!std::is_move_constructible_v<Option<CopyOnly>>);
static_assert(!std::is_move_assignable_v<Option<CopyOnly>>);
static_assert(!std::is_copy_constructible_v<Option<std::unique_ptr<int>>>);
static_assert(!std::is_copy_assignable_v<Option<std::unique_ptr<int>>>);
static_assert(std::is_nothrow_move_constructible_v<Option<std::unique_ptr<int>>>);
static_assert(std::is_nothrow_move_assignable_v<Option<std::unique_ptr<int>>>);
static_assert(!noexcept(std::declval<Option<ThrowingDestructor>&>().reset()));
static_assert(!noexcept(std::declval<Option<ThrowingDestructor>&>() = None));
static_assert(!noexcept(std::declval<Option<ThrowingDestructor>&>().emplace()));
static_assert(!std::is_nothrow_destructible_v<Option<ThrowingDestructor>>);
static_assert(std::is_constructible_v<Option<const int&>, int&>);
static_assert(std::is_constructible_v<Option<const int&>, const int&>);
static_assert(!std::is_constructible_v<Option<int&>, const int&>);
static_assert(!std::is_constructible_v<Option<const int&>, int>);
static_assert(!std::is_constructible_v<Option<const std::string&>, std::string>);
static_assert(!std::is_constructible_v<Option<const std::string&>, const char (&)[4]>);
static_assert(!std::is_constructible_v<Option<const std::string&>, ReferenceConversion&>);
static_assert(!Someable<const std::string&, std::string>);
static_assert(!Rebindable<Option<const std::string&>, std::string>);
static_assert(!Rebindable<Option<const std::string&>, const char (&)[4]>);
static_assert(std::is_constructible_v<Option<Nontrivial&>, Derived&>);
static_assert(std::is_same_v<decltype(std::declval<Option<int>&>().value()), int&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Option<int>&>().value()), const int&>
);
static_assert(std::is_same_v<decltype(std::declval<Option<int>&&>().value()), int&&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Option<int>&&>().value()), const int&&>
);
static_assert(std::is_same_v<decltype(*std::declval<Option<int>&&>()), int&&>);
static_assert(
    std::is_same_v<decltype(*std::declval<const Option<int>&&>()), const int&&>
);
static_assert(
    std::is_same_v<decltype(std::declval<const Option<int&>&>().value()), int&>
);
static_assert(std::is_same_v<decltype(std::declval<Option<void>>().value()), void>);
static_assert(!std::is_assignable_v<Option<const int>&, Option<const int>>);
static_assert(!std::is_constructible_v<Option<long>, Option<int>>);

constexpr auto answer = some<int>(42);
static_assert(answer.has_value() && answer.value() == 42);
static_assert(!none<int>().has_value());
static_assert(some<const int>(7).value() == 7);

constexpr auto make_immobile() -> Option<Immobile> { return some<Immobile>(9); }

static_assert(make_immobile()->value == 9);
static_assert([] {
    Option<int> x;
    x.emplace(5);
    auto old = x.take();
    x = old;
    x = None;
    x.emplace(8);
    Option<int> y(x);
    y = std::move(x);
    return x.has_value() && y.value() == 8 && old.value() == 5;
}());
static_assert([] {
    int                n = 1;
    auto               ref = some<int&>(n);
    const Option<int&> copy = ref;
    copy.value() = 3;
    auto old = ref.take();
    auto marker = some<void>();
    marker.value();
    auto previous = marker.take();
    marker = None;
    marker.emplace();
    return n == 3 && old.has_value() && !ref.has_value() && marker.has_value() &&
           previous.has_value();
}());
} // namespace

TEST(Option, ExplicitConstructionAndFactories) {
    Option<int> empty{None};
    EXPECT_FALSE(empty.has_value());
    auto value = some<int>(42);
    EXPECT_EQ(value.value(), 42);
    EXPECT_EQ(*value, 42);
    auto resource = make_immobile();
    EXPECT_EQ(resource->value, 9);
    resource.emplace(11);
    EXPECT_EQ(resource->value, 11);
    resource = None;
    EXPECT_FALSE(resource.has_value());
    Option<std::vector<int>> list{
        InPlace, std::initializer_list<int>{1, 2, 3}
    };
    EXPECT_EQ(list->size(), 3u);
    Option<const int> constant{InPlace, 4};
    constant.emplace(7);
    EXPECT_EQ(*constant, 7);
}

TEST(Option, CopyAndMoveAssignmentsCoverAllStates) {
    Option<std::string> source;
    Option<std::string> destination;
    destination = source;
    EXPECT_FALSE(destination.has_value());
    source.emplace("first");
    destination = source;
    EXPECT_EQ(destination.value(), "first");
    source.emplace("second");
    destination = source;
    EXPECT_EQ(destination.value(), "second");
    Option<std::string> copy(source);
    EXPECT_EQ(*copy, "second");
    source.reset();
    destination = source;
    EXPECT_FALSE(destination.has_value());

    destination = std::move(source);
    EXPECT_FALSE(destination.has_value());
    source.emplace("third");
    destination = std::move(source);
    EXPECT_TRUE(source.has_value());
    EXPECT_EQ(*destination, "third");
    source.emplace("fourth");
    destination = std::move(source);
    EXPECT_TRUE(source.has_value());
    EXPECT_EQ(*destination, "fourth");
    source.reset();
    destination = std::move(source);
    EXPECT_FALSE(destination.has_value());
    source.emplace("self");
    auto& alias = source;
    source = alias;
    source = std::move(alias);
    EXPECT_TRUE(source.has_value());
    EXPECT_EQ(*source, "self");
    Option<std::string> empty_copy(destination);
    Option<std::string> empty_move(std::move(destination));
    EXPECT_FALSE(empty_copy.has_value());
    EXPECT_FALSE(empty_move.has_value());
}

TEST(Option, MovePreservesEngagementAndTakeClearsIt) {
    auto source = some<std::unique_ptr<int>>(std::make_unique<int>(7));
    Option<std::unique_ptr<int>> moved(std::move(source));
    EXPECT_TRUE(source.has_value());
    EXPECT_EQ(source.value(), nullptr);
    EXPECT_EQ(**moved, 7);
    source = std::move(moved);
    EXPECT_TRUE(moved.has_value());
    EXPECT_EQ(moved.value(), nullptr);
    auto old = source.take();
    EXPECT_FALSE(source.has_value());
    EXPECT_EQ(**old, 7);
    EXPECT_FALSE(source.take().has_value());
    auto moved_from = moved.take();
    EXPECT_TRUE(moved_from.has_value());
    EXPECT_EQ(*moved_from, nullptr);
    EXPECT_FALSE(moved.has_value());
}

TEST(Option, ReferencesCopyBindingsAndNeverTouchReferents) {
    int                first = 1, second = 2;
    auto               ref = some<int&>(first);
    const Option<int&> copy(ref);
    *copy = 10;
    EXPECT_EQ(first, 10);
    ref.rebind(second);
    *ref = 20;
    EXPECT_EQ(second, 20);
    EXPECT_EQ(*copy, 10);
    ref = copy;
    EXPECT_EQ(ref.operator->(), &first);
    Option<int&> moved(std::move(ref));
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(moved.operator->(), &first);
    moved = std::move(ref);
    EXPECT_TRUE(ref.has_value());
    auto previous = ref.take();
    EXPECT_FALSE(ref.has_value());
    EXPECT_EQ(previous.operator->(), &first);
    EXPECT_FALSE(ref.take().has_value());
    moved = None;
    EXPECT_FALSE(moved.has_value());
    const int          constant = 4;
    Option<const int&> const_ref(constant);
    EXPECT_EQ(const_ref.operator->(), &constant);
    auto              immobile = make_immobile();
    Option<Immobile&> a(*immobile);
    auto              b = std::move(a);
    a = b;
    b.reset();
    EXPECT_EQ(a->value, 9);
}

TEST(Option, VoidPresence) {
    auto empty = none<void>();
    EXPECT_FALSE(empty.has_value());
    EXPECT_FALSE(empty.take().has_value());
    empty.emplace();
    empty.value();
    auto moved = std::move(empty);
    EXPECT_TRUE(empty.has_value());
    EXPECT_TRUE(moved.has_value());
    Option<void> copied(moved);
    moved = None;
    moved = copied;
    EXPECT_TRUE(moved.has_value());
    moved = std::move(copied);
    EXPECT_TRUE(copied.has_value());
    auto previous = moved.take();
    EXPECT_FALSE(moved.has_value());
    EXPECT_TRUE(previous.has_value());
    previous.reset();
    EXPECT_FALSE(previous.has_value());
}

TEST(Option, CheckedEmptyAccessIsAlwaysFatal) {
    EXPECT_DEATH((Option<int>{}.value()), "");
    EXPECT_DEATH((Option<int&>{}.value()), "");
    EXPECT_DEATH((Option<void>{}.value()), "");
    const Option<int> empty;
    EXPECT_DEATH(empty.value(), "");
    EXPECT_DEATH(std::move(empty).value(), "");
}
