#include "JadeFrame/utils/result.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

using namespace JadeFrame;

namespace {

struct NonMovable {
    int value;

    explicit constexpr NonMovable(int n) noexcept
        : value(n) {}

    NonMovable(const NonMovable&) = delete;
    NonMovable(NonMovable&&) = delete;
    auto operator=(const NonMovable&) -> NonMovable& = delete;
    auto operator=(NonMovable&&) -> NonMovable& = delete;
};

struct CopyOnly {
    CopyOnly() = default;
    CopyOnly(const CopyOnly&) = default;
    CopyOnly(CopyOnly&&) = delete;
    auto operator=(const CopyOnly&) -> CopyOnly& = default;
    auto operator=(CopyOnly&&) -> CopyOnly& = delete;
};

struct NonAssignable {
    int value;

    explicit NonAssignable(int n = 0) noexcept
        : value(n) {}

    NonAssignable(const NonAssignable&) = default;
    NonAssignable(NonAssignable&&) = default;
    auto operator=(const NonAssignable&) -> NonAssignable& = delete;
    auto operator=(NonAssignable&&) -> NonAssignable& = delete;
};

struct MoveSensitive {
    int               value;
    inline static int copies = 0;
    inline static int moves = 0;

    explicit MoveSensitive(int n) noexcept
        : value(n) {}

    MoveSensitive(const MoveSensitive& other) noexcept
        : value(other.value) {
        ++copies;
    }

    MoveSensitive(MoveSensitive&& other) noexcept
        : value(other.value) {
        ++moves;
        other.value = -1;
    }
};

struct ThrowingDestructor {
    ~ThrowingDestructor() noexcept(false) {}
};

struct NonTrivial {
    NonTrivial() = default;

    NonTrivial(const NonTrivial&) noexcept {}

    NonTrivial(NonTrivial&&) noexcept {}

    auto operator=(const NonTrivial&) noexcept -> NonTrivial& { return *this; }

    auto operator=(NonTrivial&&) noexcept -> NonTrivial& { return *this; }

    ~NonTrivial() noexcept {}
};

struct NonTrivialMove {
    NonTrivialMove() = default;
    NonTrivialMove(const NonTrivialMove&) = default;

    NonTrivialMove(NonTrivialMove&&) noexcept {}

    auto operator=(const NonTrivialMove&) -> NonTrivialMove& = default;

    auto operator=(NonTrivialMove&&) noexcept -> NonTrivialMove& { return *this; }
};

struct NonTrivialCopy {
    NonTrivialCopy() = default;

    NonTrivialCopy(const NonTrivialCopy&) noexcept {}

    NonTrivialCopy(NonTrivialCopy&&) = default;

    auto operator=(const NonTrivialCopy&) noexcept -> NonTrivialCopy& { return *this; }

    auto operator=(NonTrivialCopy&&) -> NonTrivialCopy& = default;
};

struct NonTrivialAssignment {
    auto operator=(const NonTrivialAssignment&) noexcept -> NonTrivialAssignment& {
        return *this;
    }

    auto operator=(NonTrivialAssignment&&) noexcept -> NonTrivialAssignment& {
        return *this;
    }

    NonTrivialAssignment() = default;
    NonTrivialAssignment(const NonTrivialAssignment&) = default;
    NonTrivialAssignment(NonTrivialAssignment&&) = default;
};

template<class R>
concept Dereferenceable = requires(R& r) {
    *r;
    r.operator->();
};
template<class R>
concept HasBool = requires(R& r) { static_cast<bool>(r); };
template<class R>
concept CopyOk = requires(const R& r) { r.ok(); };
template<class R>
concept CopyErr = requires(const R& r) { r.err(); };
template<class R, class... Args>
concept EmplaceOk =
    requires(R& r, Args&&... args) { r.emplace_ok(std::forward<Args>(args)...); };
template<class R, class... Args>
concept EmplaceErr =
    requires(R& r, Args&&... args) { r.emplace_err(std::forward<Args>(args)...); };

template<class R>
constexpr bool all_trivial =
    std::is_trivially_copy_constructible_v<R> &&
    std::is_trivially_move_constructible_v<R> && std::is_trivially_copy_assignable_v<R> &&
    std::is_trivially_move_assignable_v<R> && std::is_trivially_destructible_v<R> &&
    std::is_trivially_copyable_v<R>;

static_assert(all_trivial<Result<int, int>>);
static_assert(all_trivial<Result<int&, int>>);
static_assert(all_trivial<Result<void, int>>);
static_assert(all_trivial<Result<void, void>>);
static_assert(all_trivial<Result<NonAssignable&, ThrowingDestructor&>>);
static_assert(!std::is_trivially_copy_constructible_v<Result<int, NonTrivial>>);
static_assert(!std::is_trivially_move_constructible_v<Result<int, NonTrivial>>);
static_assert(!std::is_trivially_copy_assignable_v<Result<int, NonTrivial>>);
static_assert(!std::is_trivially_move_assignable_v<Result<int, NonTrivial>>);
static_assert(!std::is_trivially_destructible_v<Result<int, NonTrivial>>);
static_assert(std::is_trivially_copy_constructible_v<Result<int, NonTrivialMove>>);
static_assert(!std::is_trivially_move_constructible_v<Result<int, NonTrivialMove>>);
static_assert(std::is_trivially_copy_assignable_v<Result<int, NonTrivialMove>>);
static_assert(!std::is_trivially_move_assignable_v<Result<int, NonTrivialMove>>);
static_assert(std::is_trivially_destructible_v<Result<int, NonTrivialMove>>);
static_assert(!std::is_trivially_copy_constructible_v<Result<NonTrivialCopy, int>>);
static_assert(std::is_trivially_move_constructible_v<Result<NonTrivialCopy, int>>);
static_assert(!std::is_trivially_copy_assignable_v<Result<NonTrivialCopy, int>>);
static_assert(std::is_trivially_move_assignable_v<Result<NonTrivialCopy, int>>);
static_assert(std::is_trivially_destructible_v<Result<NonTrivialCopy, int>>);
static_assert(std::is_trivially_copy_constructible_v<Result<NonTrivialAssignment, int>>);
static_assert(std::is_trivially_move_constructible_v<Result<NonTrivialAssignment, int>>);
static_assert(!std::is_trivially_copy_assignable_v<Result<NonTrivialAssignment, int>>);
static_assert(!std::is_trivially_move_assignable_v<Result<NonTrivialAssignment, int>>);
static_assert(std::is_trivially_destructible_v<Result<NonTrivialAssignment, int>>);

static_assert(ok<NonMovable, int>(12).value().value == 12);
static_assert(std::is_nothrow_copy_assignable_v<Result<int, int>>);
static_assert(std::is_nothrow_move_assignable_v<Result<int, int>>);
static_assert(std::is_nothrow_destructible_v<Result<NonTrivial, void>>);
static_assert(!std::is_default_constructible_v<Result<int, int>>);
static_assert(!std::is_default_constructible_v<Result<void, void>>);
static_assert(!std::is_constructible_v<Result<int, int>, int>);
static_assert(!std::is_convertible_v<int, Result<int, int>>);
static_assert(!HasBool<Result<int, int>>);
static_assert(!std::is_constructible_v<Result<int, int>, Result<short, short>>);
static_assert(std::is_constructible_v<Result<NonMovable, int>, OkTag, int>);
static_assert(!std::is_copy_constructible_v<Result<NonMovable, int>>);
static_assert(!std::is_move_constructible_v<Result<NonMovable, int>>);
static_assert(std::is_copy_constructible_v<Result<CopyOnly, int>>);
static_assert(!std::is_move_constructible_v<Result<CopyOnly, int>>);
static_assert(std::is_copy_assignable_v<Result<CopyOnly, int>>);
static_assert(!std::is_move_assignable_v<Result<CopyOnly, int>>);
static_assert(!std::is_copy_assignable_v<Result<NonAssignable, int>>);
static_assert(!std::is_move_assignable_v<Result<int, NonAssignable>>);
static_assert(!std::is_copy_constructible_v<Result<std::unique_ptr<int>, int>>);
static_assert(std::is_move_assignable_v<Result<std::unique_ptr<int>, int>>);
static_assert(!CopyOk<Result<std::unique_ptr<int>, int>>);
static_assert(CopyErr<Result<std::unique_ptr<int>, int>>);
static_assert(CopyOk<Result<int, std::unique_ptr<int>>>);
static_assert(!CopyErr<Result<int, std::unique_ptr<int>>>);
static_assert(!std::is_copy_assignable_v<Result<const int, int>>);
static_assert(!std::is_move_assignable_v<Result<int, const int>>);
static_assert(std::is_nothrow_constructible_v<Result<const int, void>, OkTag, int>);
static_assert(noexcept(ok<int, void>(1)));
static_assert(noexcept(err<void, int>(1)));
static_assert(noexcept(std::declval<Result<int, int>&>().value()));
static_assert(noexcept(std::declval<Result<int, int>&>().error()));
static_assert(!Dereferenceable<Result<void, int>>);
static_assert(!EmplaceOk<Result<void, int>, int>);
static_assert(!EmplaceErr<Result<int, void>, int>);
static_assert(EmplaceOk<Result<NonMovable, int>, int>);
static_assert(!std::is_constructible_v<Result<const int&, int>, OkTag, int>);
static_assert(!std::is_constructible_v<Result<int, const int&>, ErrTag, int>);
static_assert(!EmplaceOk<Result<const int&, int>, int>);
static_assert(!EmplaceErr<Result<int, const int&>, int>);
static_assert(std::is_constructible_v<Result<int (&)[3], void>, OkTag, int (&)[3]>);

using Object = Result<int, long>;
using ConstObject = Result<const int, const long>;
using Reference = Result<int&, const long&>;
static_assert(std::is_same_v<decltype(std::declval<Object&>().value()), int&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Object&>().value()), const int&>
);
static_assert(std::is_same_v<decltype(std::declval<Object&&>().value()), int&&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Object&&>().value()), const int&&>
);
static_assert(std::is_same_v<decltype(std::declval<Object&>().error()), long&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Object&>().error()), const long&>
);
static_assert(std::is_same_v<decltype(std::declval<Object&&>().error()), long&&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Object&&>().error()), const long&&>
);
static_assert(std::is_same_v<decltype(std::declval<ConstObject&>().value()), const int&>);
static_assert(
    std::is_same_v<decltype(std::declval<ConstObject&&>().value()), const int&&>
);
static_assert(
    std::is_same_v<decltype(std::declval<ConstObject&>().error()), const long&>
);
static_assert(
    std::is_same_v<decltype(std::declval<ConstObject&&>().error()), const long&&>
);
static_assert(std::is_same_v<decltype(std::declval<const Reference&>().value()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const Reference&&>().value()), int&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Reference&>().error()), const long&>
);
static_assert(std::is_same_v<decltype(std::declval<Reference&&>().error()), const long&>);
static_assert(
    std::is_same_v<decltype(std::declval<const Reference&>().operator->()), int*>
);
static_assert(
    std::is_same_v<decltype(std::declval<ConstObject&>().operator->()), const int*>
);
static_assert(
    std::is_same_v<decltype(std::declval<Result<void, void>&>().value()), void>
);
static_assert(
    std::is_same_v<decltype(std::declval<Result<void, void>&>().error()), void>
);

// Validate payload abstractions before exercising the Result facade.
using namespace result::details;
static_assert(std::is_same_v<PayloadTraits<const int>::storage_type, int>);
static_assert(!PayloadTraits<const int>::copy_assignable);
static_assert(PayloadTraits<const int>::copy_constructible);
static_assert(PayloadTraits<NonMovable&>::copy_assignable);
static_assert(PayloadTraits<NonMovable&>::nothrow_move);
static_assert(PayloadTraits<int&>::constructible<int&>);
static_assert(!PayloadTraits<const int&>::constructible<int>);
static_assert(PayloadTraits<void>::constructible<>);
static_assert(!PayloadTraits<void>::constructible<int>);

static_assert([] {
    auto a = ok<int, int>(4);
    auto b = err<int, int>(7);
    auto c = a;
    auto d = std::move(b);
    if (!b.is_err() || d.error() != 7 || *c != 4) { return false; }
    a = d;
    c = std::move(d);
    if (a.error() != 7 || c.error() != 7 || !d.is_err()) { return false; }
    a.emplace_ok(8);
    a.emplace_ok(9);
    c.emplace_err(10);
    return a.has_value() && a.value() == 9 && a.ok().value() == 9 &&
           !a.err().has_value() && c.err().value() == 10;
}());
static_assert([] {
    int                a = 4, b = 7;
    Result<int&, void> x{Ok, a}, y{Ok, b};
    x = y;
    x.value() = 8;
    const auto z = std::move(x);
    if (&z.value() != &b || &z.ok().value() != &b || a != 4) { return false; }
    x.emplace_err();
    x.error();
    x.emplace_ok(a);
    Result<void, void> v{Ok};
    auto               w = v;
    w.emplace_err();
    v = std::move(w);
    v.error();
    v.emplace_ok();
    v.value();
    return b == 8 && x.value() == 4 && v.ok().has_value() && !v.err().has_value();
}());

} // namespace

TEST(Result, TaggedConstructionAndFactories) {
    Result<int, int> a{Ok, 42}, b{Err, 42};
    EXPECT_TRUE(a.is_ok());
    EXPECT_FALSE(a.is_err());
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a.value(), 42);
    EXPECT_TRUE(b.is_err());
    EXPECT_FALSE(b.has_value());
    EXPECT_EQ(b.error(), 42);
    EXPECT_EQ((ok<int, int>(12).value()), 12);
    EXPECT_EQ((err<int, int>(13).error()), 13);
    const auto nonmovable = ok<NonMovable, int>(14);
    EXPECT_EQ(nonmovable->value, 14);
    const auto error = err<int, NonMovable>(15);
    EXPECT_EQ(error.error().value, 15);
}

TEST(Result, CopyMoveAllStateCombinations) {
    for (bool source_ok : {false, true}) {
        for (bool target_ok : {false, true}) {
            Result<std::string, std::string> source{Ok, "source"};
            Result<std::string, std::string> target{Ok, "target"};
            if (!source_ok) { source.emplace_err(std::string(6, 's')); }
            if (!target_ok) { target.emplace_err(std::string(6, 't')); }
            auto copy = source;
            EXPECT_EQ(copy.is_ok(), source_ok);
            EXPECT_EQ(
                copy.is_ok() ? copy.value() : copy.error(),
                source_ok ? "source" : "ssssss"
            );
            target = source;
            EXPECT_EQ(target.is_ok(), source_ok);
            EXPECT_EQ(
                target.is_ok() ? target.value() : target.error(),
                source_ok ? "source" : "ssssss"
            );
            auto moved = std::move(copy);
            EXPECT_EQ(copy.is_ok(), source_ok);
            EXPECT_EQ(moved.is_ok(), source_ok);
            if (target_ok) {
                target.emplace_ok(std::string(6, 't'));
            } else {
                target.emplace_err(std::string(6, 't'));
            }
            target = std::move(moved);
            EXPECT_EQ(moved.is_ok(), source_ok);
            EXPECT_EQ(target.is_ok(), source_ok);
            EXPECT_EQ(
                target.is_ok() ? target.value() : target.error(),
                source_ok ? "source" : "ssssss"
            );
            auto& alias = target;
            target = alias;
            target = std::move(alias);
            EXPECT_EQ(
                target.is_ok() ? target.value() : target.error(),
                source_ok ? "source" : "ssssss"
            );
        }
    }
}

TEST(Result, ReferencesRebindAndNeverMoveReferents) {
    int                a = 10, b = 20;
    Result<int&, int&> x{Ok, a}, y{Ok, b};
    x = y;
    EXPECT_EQ(&x.value(), &b);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 20);
    y.emplace_err(a);
    x = std::move(y);
    EXPECT_TRUE(y.is_err());
    EXPECT_EQ(&x.error(), &a);
    const auto z = std::move(x);
    EXPECT_EQ(&std::move(z).error(), &a);
    EXPECT_EQ(&z.err().value(), &a);
    EXPECT_EQ(&y.emplace_ok(b), &b);
    EXPECT_EQ(&(*std::move(y)), &b);
    EXPECT_EQ(y.operator->(), &b);

    NonMovable                       first(1), second(2);
    Result<NonMovable&, NonMovable&> r{Ok, first}, s{Err, second};
    r = s;
    EXPECT_EQ(&r.error(), &second);
    s.emplace_ok(first);
    r = std::move(s);
    EXPECT_EQ(&r.value(), &first);
    EXPECT_EQ(first.value, 1);
    EXPECT_EQ(second.value, 2);
    EXPECT_EQ(&std::move(r).ok().value(), &first);
    Result<const NonMovable&, int> constant{Ok, first};
    EXPECT_EQ(&std::move(constant).value(), &first);
    int                      array[3] = {1, 2, 3};
    Result<int (&)[3], void> array_ref{Ok, array};
    EXPECT_EQ(&array_ref.value(), &array);
}

TEST(Result, VoidOnEitherSide) {
    Result<void, std::string> a{Ok};
    a.value();
    EXPECT_TRUE(a.ok().has_value());
    a.emplace_err(std::string(3, 'x'));
    EXPECT_EQ(a.error(), "xxx");
    a.emplace_ok();
    Result<int, void> b{Err};
    b.error();
    EXPECT_TRUE(b.err().has_value());
    b.emplace_ok(3);
    EXPECT_EQ(b.value(), 3);
    b.emplace_err();
    for (bool source_ok : {false, true}) {
        Result<void, void> source{Ok}, target{Err};
        if (!source_ok) { source.emplace_err(); }
        auto copy = source;
        auto moved = std::move(copy);
        target = moved;
        EXPECT_EQ(target.is_ok(), source_ok);
        target.emplace_ok();
        target = std::move(source);
        EXPECT_EQ(target.is_ok(), source_ok);
        EXPECT_EQ(source.is_ok(), source_ok);
        EXPECT_EQ(std::move(target).ok().has_value(), source_ok);
        EXPECT_EQ(std::move(target).err().has_value(), !source_ok);
    }
}

TEST(Result, ConstPayloadsAndOptionConversions) {
    Result<const int, const std::string> a{Ok, 5}, b{Err, "bad"};
    EXPECT_EQ(a.ok().value(), 5);
    EXPECT_FALSE(a.err().has_value());
    EXPECT_EQ(std::move(b).err().value(), "bad");
    a.emplace_err(std::string(4, 'a'));
    EXPECT_EQ(a.error(), "aaaa");
    a.emplace_ok(6);
    EXPECT_EQ(*a, 6);

    Result<std::unique_ptr<int>, std::unique_ptr<int>> owned{
        Ok, std::make_unique<int>(9)
    };
    auto selected = std::move(owned).ok();
    EXPECT_TRUE(owned.is_ok());
    EXPECT_EQ(owned.value(), nullptr);
    EXPECT_EQ(*selected.value(), 9);
    owned.emplace_err(std::make_unique<int>(10));
    auto failure = std::move(owned).err();
    EXPECT_TRUE(owned.is_err());
    EXPECT_EQ(owned.error(), nullptr);
    EXPECT_EQ(*failure.value(), 10);
    Result<int, std::unique_ptr<int>> uncopyable_error{Ok, 11};
    EXPECT_EQ(uncopyable_error.ok().value(), 11);
}

TEST(Result, ConstOwnedMovesUseTheLogicalConstPayload) {
    MoveSensitive::copies = MoveSensitive::moves = 0;
    Result<const MoveSensitive, const MoveSensitive> a{Ok, 1}, b{Err, 2};
    auto                                             moved_a = std::move(a);
    auto                                             moved_b = std::move(b);
    auto value_option = std::move(moved_a).ok();
    auto error_option = std::move(moved_b).err();
    EXPECT_EQ(MoveSensitive::moves, 0);
    EXPECT_EQ(MoveSensitive::copies, 4);
    EXPECT_EQ(a.value().value, 1);
    EXPECT_EQ(b.error().value, 2);
    EXPECT_EQ(value_option.value().value, 1);
    EXPECT_EQ(error_option.value().value, 2);
}

TEST(ResultDeathTest, CheckedAccessAlwaysAborts) {
    Result<int, int> a{Ok, 1}, b{Err, 2};
    EXPECT_DEATH(a.error(), "");
    EXPECT_DEATH(b.value(), "");
    const auto ca = a;
    const auto cb = b;
    EXPECT_DEATH(ca.error(), "");
    EXPECT_DEATH(cb.value(), "");
    EXPECT_DEATH(std::move(a).error(), "");
    EXPECT_DEATH(std::move(b).value(), "");
    EXPECT_DEATH(std::move(ca).error(), "");
    EXPECT_DEATH(std::move(cb).value(), "");
    Result<void, void> v{Ok}, w{Err};
    EXPECT_DEATH(v.error(), "");
    EXPECT_DEATH(w.value(), "");
    int                n = 1;
    Result<int&, int&> r{Ok, n}, s{Err, n};
    EXPECT_DEATH(r.error(), "");
    EXPECT_DEATH(s.value(), "");
}

TEST(Result, MixedVoidAssignmentsInEveryStateCombination) {
    for (bool source_ok : {false, true}) {
        for (bool target_ok : {false, true}) {
            auto source = source_ok ? Result<void, std::string>{Ok}
                                    : Result<void, std::string>{Err, "error"};
            auto target = target_ok ? Result<void, std::string>{Ok}
                                    : Result<void, std::string>{Err, "old"};
            auto copy = source;
            auto moved = std::move(copy);
            EXPECT_EQ(copy.is_ok(), source_ok);
            target = source;
            EXPECT_EQ(target.is_ok(), source_ok);
            if (!source_ok) { EXPECT_EQ(target.error(), "error"); }
            if (target_ok) {
                target.emplace_ok();
            } else {
                target.emplace_err(std::string("old"));
            }
            target = std::move(moved);
            EXPECT_EQ(moved.is_ok(), source_ok);
            EXPECT_EQ(target.is_ok(), source_ok);
            EXPECT_EQ(target.ok().has_value(), source_ok);
            EXPECT_EQ(std::move(target).err().has_value(), !source_ok);

            auto value_source =
                source_ok ? Result<int, void>{Ok, 7} : Result<int, void>{Err};
            auto value_target =
                target_ok ? Result<int, void>{Ok, 1} : Result<int, void>{Err};
            auto value_copy = value_source;
            auto value_moved = std::move(value_copy);
            EXPECT_EQ(value_copy.is_ok(), source_ok);
            value_target = value_source;
            EXPECT_EQ(value_target.is_ok(), source_ok);
            if (source_ok) { EXPECT_EQ(value_target.value(), 7); }
            if (target_ok) {
                value_target.emplace_ok(1);
            } else {
                value_target.emplace_err();
            }
            value_target = std::move(value_moved);
            EXPECT_EQ(value_moved.is_ok(), source_ok);
            EXPECT_EQ(value_target.is_ok(), source_ok);
            EXPECT_EQ(std::move(value_target).ok().has_value(), source_ok);
            EXPECT_EQ(value_target.err().has_value(), !source_ok);
        }
    }
}

TEST(Result, RestrictedPayloadsUseOnlyTheirSupportedOperations) {
    Result<CopyOnly, int> copy_only{Ok}, assigned{Err, 1};
    auto                  copied = copy_only;
    assigned = copied;
    EXPECT_TRUE(assigned.is_ok());
    Result<NonAssignable, void> construct_only{Ok, 8};
    auto                        constructed_copy = construct_only;
    auto                        constructed_move = std::move(construct_only);
    EXPECT_TRUE(construct_only.is_ok());
    EXPECT_EQ(constructed_copy.value().value, 8);
    EXPECT_EQ(constructed_move.value().value, 8);
    EXPECT_EQ(construct_only.emplace_ok(9).value, 9);
    Result<std::unique_ptr<int>, int> source{Ok, std::make_unique<int>(10)};
    auto                              moved = std::move(source);
    EXPECT_TRUE(source.is_ok());
    EXPECT_EQ(source.value(), nullptr);
    EXPECT_EQ(*moved.value(), 10);
    Result<std::unique_ptr<int>, int> destination{Err, 1};
    destination = std::move(moved);
    EXPECT_TRUE(moved.is_ok());
    EXPECT_EQ(*destination.value(), 10);
}
