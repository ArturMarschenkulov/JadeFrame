#include <gtest/gtest.h>
#include <type_traits>
#include "JadeFrame/utils/option.h"

using namespace JadeFrame;

TEST(Option, Basics) {

    // Testing functions
    {
        Option<u32> x0 = Option<u32>(2);
        EXPECT_EQ(x0.is_some(), true);

        Option<u32> x1 = Option<u32>();
        EXPECT_EQ(x1.is_some(), false);
    }
    {
        Option<u32> x0 = Option<u32>(2);
        EXPECT_EQ(x0.is_none(), false);

        Option<u32> x1 = Option<u32>();
        EXPECT_EQ(x1.is_none(), true);
    }

    // Testing "and"
    // auto s = Option<i32>(2_i32);
    // s.and_(Option<i32>(3_i32));
    {
        Option<i32>         x0 = Option<i32>(2_i32);
        Option<const char*> y0 = Option<const char*>();
        EXPECT_EQ(x0.and_(y0), Option<const char*>());

        Option<i32>         x1 = Option<i32>();
        Option<const char*> y1 = Option<const char*>("foo");
        EXPECT_EQ(x1.and_(y1), Option<const char*>());

        Option<i32>         x2 = Option<i32>(2_i32);
        Option<const char*> y2 = Option<const char*>("foo");
        EXPECT_EQ(x2.and_(y2), Option<const char*>("foo"));

        Option<i32>         x3 = Option<i32>();
        Option<const char*> y3 = Option<const char*>();
        EXPECT_EQ(x3.and_(x3), Option<i32>());

        EXPECT_EQ(Option<int>{1}.and_(Option<int>{2}), Option<int>{2});
        EXPECT_EQ(Option<int>{1}.and_(Option<int>{}), Option<int>{});
        EXPECT_EQ(Option<int>{}.and_(Option<int>{2}), Option<int>{});
        EXPECT_EQ(Option<int>{}.and_(Option<int>{}), Option<int>{});
    }

    {
        EXPECT_EQ(Option<int>{1}.or_(Option<int>{2}), Option<int>{1});
        EXPECT_EQ(Option<int>{1}.or_(Option<int>{}), Option<int>{1});
        EXPECT_EQ(Option<int>{}.or_(Option<int>{2}), Option<int>{2});
        EXPECT_EQ(Option<int>{}.or_(Option<int>{}), Option<int>{});
    }

    {
        EXPECT_EQ(Option<int>{1}.xor_(Option<int>{2}), Option<int>{});
        EXPECT_EQ(Option<int>{1}.xor_(Option<int>{}), Option<int>{1});
        EXPECT_EQ(Option<int>{}.xor_(Option<int>{2}), Option<int>{2});
        EXPECT_EQ(Option<int>{}.xor_(Option<int>{}), Option<int>{});
    }
}

TEST(Option, Basics2) {
    // Option<int> a = Option<int>(333);

    // Testing functions
    {
        Option<u32> x0 = Option<u32>(2);
        EXPECT_EQ(x0.is_some(), true);

        Option<u32> x1 = Option<u32>();
        EXPECT_EQ(x1.is_some(), false);
    }
    {
        Option<u32> x0 = Option<u32>(2);
        EXPECT_EQ(x0.is_none(), false);

        Option<u32> x1 = Option<u32>();
        EXPECT_EQ(x1.is_none(), true);
    }
    // Testing "and"
    // auto s = Option<i32>(2_i32);
    // s.and_(Option<i32>(3_i32));

    {
        Option<i32>         x0 = Option<i32>(2_i32);
        Option<const char*> y0 = Option<const char*>();
        EXPECT_EQ(x0.and_(y0), Option<const char*>());

        Option<i32>         x1 = Option<i32>();
        Option<const char*> y1 = Option<const char*>("foo");
        EXPECT_EQ(x1.and_(y1), Option<const char*>());

        Option<i32>         x2 = Option<i32>(2_i32);
        Option<const char*> y2 = Option<const char*>("foo");
        EXPECT_EQ(x2.and_(y2), Option<const char*>("foo"));

        Option<i32>         x3 = Option<i32>();
        Option<const char*> y3 = Option<const char*>();
        EXPECT_EQ(x3.and_(x3), Option<i32>());

        EXPECT_EQ(Option<int>{1}.and_(Option<int>{2}), Option<int>{2});
        EXPECT_EQ(Option<int>{1}.and_(Option<int>{}), Option<int>{});
        EXPECT_EQ(Option<int>{}.and_(Option<int>{2}), Option<int>{});
        EXPECT_EQ(Option<int>{}.and_(Option<int>{}), Option<int>{});
    }

    {
        EXPECT_EQ(Option<int>{1}.or_(Option<int>{2}), Option<int>{1});
        EXPECT_EQ(Option<int>{1}.or_(Option<int>{}), Option<int>{1});
        EXPECT_EQ(Option<int>{}.or_(Option<int>{2}), Option<int>{2});
        EXPECT_EQ(Option<int>{}.or_(Option<int>{}), Option<int>{});
    }

    {
        EXPECT_EQ(Option<int>{1}.xor_(Option<int>{2}), Option<int>{});
        EXPECT_EQ(Option<int>{1}.xor_(Option<int>{}), Option<int>{1});
        EXPECT_EQ(Option<int>{}.xor_(Option<int>{2}), Option<int>{2});
        EXPECT_EQ(Option<int>{}.xor_(Option<int>{}), Option<int>{});
    }
}

TEST(Option, Basics3) {
    struct Student {
        int         age;
        std::string name;

        [[nodiscard]] auto get_name() const -> Option<const std::string&> {
            if (age >= 18) {
                return Option<const std::string&>(name);
            } else {
                return Option<const std::string&>();
            }
        }
    };

    Student s = {16, "John"};
    auto    x = s.get_name();
    EXPECT_EQ(x.is_some(), false);
}

TEST(OptionTest, DefaultConstruction) {
    Option<int> opt;
    EXPECT_FALSE(opt.has_value());

    opt = Option(10);
    ASSERT_TRUE(opt.has_value());
}

TEST(OptionTest, ValueConstruction) {
    Option<int> opt(10);
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt.value(), 10);
}

TEST(OptionTest, CopyConstruction) {
    Option<int> original(10);
    Option<int> copy(original);
    ASSERT_TRUE(copy.has_value());
    EXPECT_EQ(copy.value(), 10);
}

TEST(OptionTest, MoveConstruction) {
    Option<int> original(10);
    Option<int> moved(std::move(original));
    EXPECT_TRUE(moved.has_value());
    EXPECT_EQ(moved.value(), 10);
    EXPECT_FALSE(original.has_value());
}

// TEST(OptionTest, CopyAssignment) {
//     Option<int> opt1(10);
//     Option<int> opt2;
//     opt2 = opt1;
//     ASSERT_TRUE(opt2.has_value());
//     EXPECT_EQ(opt2.value(), 10);
// }

TEST(OptionTest, MoveAssignment) {
    Option<int> opt1(10);
    Option<int> opt2;
    opt2 = std::move(opt1);
    EXPECT_TRUE(opt2.has_value());
    EXPECT_EQ(opt2.value(), 10);
    EXPECT_FALSE(opt1.has_value());
}

// TEST(OptionTest, SelfAssignment) {
//     Option<int> opt(10);
//     opt = opt;
//     ASSERT_TRUE(opt.has_value());
//     EXPECT_EQ(opt.value(), 10);
// }

TEST(OptionTest, Comparison) {
    Option<int> opt1(10);
    Option<int> opt2(10);
    Option<int> opt3(20);
    Option<int> none;

    EXPECT_TRUE(opt1 == opt2);
    EXPECT_FALSE(opt1 == opt3);
    EXPECT_FALSE(opt1 == none);
}

TEST(OptionTest, Unwrap) {
    Option<const int> opt(10);
    EXPECT_EQ(opt.unwrap(), 10);
}

TEST(OptionTest, UnwrapOr) {
    Option<int> opt(10);
    EXPECT_EQ(opt.or_(Option<int>(20)).unwrap(), 10);

    Option<int> none;
    EXPECT_EQ(none.or_(Option<int>(20)).unwrap(), 20);
}

TEST(OptionTest, UnwrapOrElse) {
    Option<int> opt(10);
    auto        or_else = []() { return Option<int>(20); };
    EXPECT_EQ(opt.or_else(or_else).unwrap(), 10);

    Option<int> none;
    EXPECT_EQ(none.or_else(or_else).unwrap(), 20);
}

TEST(OptionTest, AndThen) {
    Option<int> opt(10);
    auto        multiply_by_two = [](int value) { return Option<int>(value * 2); };
    EXPECT_EQ(opt.and_then(multiply_by_two).unwrap(), 20);

    Option<int> none;
    EXPECT_FALSE(none.and_then(multiply_by_two).has_value());
}

TEST(OptionTest, Xor) {
    Option<int> opt1(10);
    Option<int> opt2;
    EXPECT_TRUE(opt1.xor_(opt2).has_value());
    EXPECT_EQ(opt1.xor_(opt2).unwrap(), 10);

    Option<int> opt3(10);
    EXPECT_FALSE(opt1.xor_(opt3).has_value());
}

TEST(Option, Misc_0) {
    // static_assert(std::is_trivially_destructible<Option<u8>>());

    struct DestructionChecker {
        explicit DestructionChecker(bool& was_destroyed)
            : m_was_destroyed(was_destroyed) {}

        ~DestructionChecker() { m_was_destroyed = true; }

        bool& m_was_destroyed;
    };

    static_assert(!std::is_trivially_destructible<Option<DestructionChecker>>());
    bool was_destroyed = false;
    { Option<DestructionChecker> opt = Option(DestructionChecker(was_destroyed)); }
    EXPECT_TRUE(was_destroyed);

    struct CopyChecker {
        explicit CopyChecker(bool& was_copy_constructed)
            : m_was_copy_constructed(was_copy_constructed) {}

        CopyChecker(CopyChecker const& other)
            : m_was_copy_constructed(other.m_was_copy_constructed) {
            m_was_copy_constructed = true;
        }

        bool& m_was_copy_constructed;
    };

    static_assert(std::is_copy_constructible<Option<CopyChecker>>());
    static_assert(!std::is_trivially_copy_constructible<Option<CopyChecker>>());
    bool was_copy_constructed = false;
    {
        Option<CopyChecker> opt = Option(CopyChecker(was_copy_constructed));
        Option<CopyChecker> opt2 = opt;
    }
    EXPECT_TRUE(was_copy_constructed);

    struct MoveChecker {
        explicit MoveChecker(bool& was_move_constructed)
            : m_was_move_constructed(was_move_constructed) {}

        MoveChecker(MoveChecker const& other)
            : m_was_move_constructed(other.m_was_move_constructed) {
            EXPECT_TRUE(false);
        }

        MoveChecker(MoveChecker&& other)
            : m_was_move_constructed(other.m_was_move_constructed) {
            m_was_move_constructed = true;
        }

        bool& m_was_move_constructed;
    };

    static_assert(std::is_move_constructible<Option<MoveChecker>>());
    static_assert(!std::is_trivially_move_constructible<Option<MoveChecker>>());
    bool was_moved = false;
    {
        Option<MoveChecker> opt = Option(MoveChecker(was_moved));
        Option<MoveChecker> opt2 = std::move(opt);
    }
    EXPECT_TRUE(was_moved);
}

// This test is about testing that non-trivial types are handled correctly.
TEST(Option, Complex) {
    struct ComplexType {
        ComplexType() = default;

        ComplexType(const ComplexType& other)
            : data(other.data) {}

        ComplexType(ComplexType&& other) noexcept
            : data(std::move(other.data)) {}

        ~ComplexType() = default;

        std::string data = "Hello, World!";
    };

    static_assert(std::is_copy_constructible<Option<ComplexType>>());
    static_assert(std::is_move_constructible<Option<ComplexType>>());

    // Test copy construction
    ComplexType ct;
    ct.data = "Test Data";
    Option<ComplexType> opt1(ct);
    Option<ComplexType> opt2(opt1);
    EXPECT_EQ(opt2.unwrap().data, "Test Data");

    // Test move construction
    Option<ComplexType> opt3(std::move(opt1));
    EXPECT_EQ(opt3.unwrap().data, "Test Data");
}

// ------------------------------------------------------------
// Compile-time checks (C++20 requires / concepts)
// ------------------------------------------------------------

// 1) unwrap() & / const& return types for Option<T&>
static_assert(
    std::is_same_v<decltype(std::declval<Option<int&>&>().unwrap()), int&>,
    "Option<int&>::unwrap() & should return int&"
);

static_assert(
    std::is_same_v<
        decltype(std::declval<const Option<int&>&>().unwrap()),
        int&>, // const T& with T = int& -> int&
    "Option<int&>::unwrap() const& should return int&"
);

// 2) unwrap_unchecked() & / const& return types for Option<T&>
static_assert(
    std::is_same_v<decltype(std::declval<Option<int&>&>().unwrap_unchecked()), int&>,
    "Option<int&>::unwrap_unchecked() & should return int&"
);

static_assert(
    std::
        is_same_v<decltype(std::declval<const Option<int&>&>().unwrap_unchecked()), int&>,
    "Option<int&>::unwrap_unchecked() const& should return int&"
);

// 3) For value types, unwrap() && must be available
static_assert(
    requires(Option<int> opt) {
        { std::move(opt).unwrap() } -> std::same_as<int>;
    },
    "Option<int>::unwrap() && should be available and return int"
);

// // 4) For reference types, unwrap() && should NOT be available
// static_assert(
//     !requires(Option<int&> opt) { std::move(opt).unwrap(); },
//     "Option<int&>::unwrap() && should be deleted / not available"
// );

// 5) For value types, unwrap_unchecked() && must be available
static_assert(
    requires(Option<int> opt) {
        { std::move(opt).unwrap_unchecked() } -> std::same_as<int>;
    },
    "Option<int>::unwrap_unchecked() && should be available and return int"
);

// // 6) For reference types, unwrap_unchecked() && should NOT be available
// static_assert(
//     !requires(Option<int&> opt) { std::move(opt).unwrap_unchecked(); },
//     "Option<int&>::unwrap_unchecked() && should be deleted / not available"
// );

// ------------------------------------------------------------
// Runtime tests
// ------------------------------------------------------------

TEST(OptionRef, UnwrapLvalueReturnsReferenceAndIsMutable) {
    int          x = 42;
    Option<int&> opt{x};

    ASSERT_TRUE(opt.has_value());

    int& ref1 = opt.unwrap(); // lvalue overload
    EXPECT_EQ(&ref1, &x);
    EXPECT_EQ(ref1, 42);

    // Mutate through the reference returned by unwrap()
    ref1 = 99;
    EXPECT_EQ(x, 99);

    // Calling unwrap() again should still be fine for lvalue reference,
    // and should still refer to the same object.
    int& ref2 = opt.unwrap();
    EXPECT_EQ(&ref2, &x);
    EXPECT_EQ(ref2, 99);
}

TEST(OptionRef, ConstUnwrapLvalueReturnsReferenceToSameObject) {
    int                x = 10;
    const Option<int&> opt{x};

    ASSERT_TRUE(opt.has_value());

    const int& cref = opt.unwrap();
    EXPECT_EQ(&cref, &x);
    EXPECT_EQ(cref, 10);
}

TEST(OptionRef, UnwrapUncheckedLvalueReturnsReference) {
    int          x = 5;
    Option<int&> opt{x};

    ASSERT_TRUE(opt.has_value());

    int& ref = opt.unwrap_unchecked();
    EXPECT_EQ(&ref, &x);
    ref = 123;
    EXPECT_EQ(x, 123);
}

TEST(OptionValue, MoveUnwrapResetsOption) {
    Option<int> opt{42};

    ASSERT_TRUE(opt.has_value());

    int value = std::move(opt).unwrap();
    EXPECT_EQ(value, 42);
    EXPECT_FALSE(opt.has_value()); // after move-unwrap, option should be None
}

TEST(OptionValue, MoveUnwrapUncheckedResetsOption) {
    Option<int> opt{123};

    ASSERT_TRUE(opt.has_value());

    int value = std::move(opt).unwrap_unchecked();
    EXPECT_EQ(value, 123);
    EXPECT_FALSE(opt.has_value());
}

TEST(OptionValue, CopyAndEqualitySemantics) {
    Option<int> a{7};
    Option<int> b{7};
    Option<int> c; // None

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(c == a);

    // Move-construct
    Option<int> d{std::move(a)};
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(d.unwrap(), 7);
}

TEST(OptionRef, CopyAndEqualitySemantics) {
    int x = 1;
    int y = 1;
    int z = 2;

    Option<int&> a{x};
    Option<int&> b{x};
    Option<int&> c{y};
    Option<int&> d{z};
    Option<int&> none; // default-constructed None

    EXPECT_TRUE(a == b);  // same object
    EXPECT_TRUE(a == c);  // different object, same value
    EXPECT_FALSE(a == d); // different value
    EXPECT_FALSE(a == none);
    EXPECT_FALSE(none == a);
}

TEST(MISC, MISC_0) {
    int          global = 10;
    Option<int&> opt(global);

    // Caller code:
    int& r = std::move(opt).unwrap();

    EXPECT_EQ(r, 10);
    EXPECT_EQ(global, 10);
    EXPECT_EQ(opt.has_value(), true);
}

// #include <gtest/gtest.h>

// #include "Option.h" // include your Option implementation

// using JadeFrame::Option;

TEST(OptionReferenceTest, ConstOptionDoesNotConstQualifyReferredObject)
{
    int value = 42;

    Option<int&> opt(value);
    const Option<int&>& const_opt = opt;

    // This MUST compile and work:
    int& ref = const_opt.unwrap();
    ref = 100;

    EXPECT_EQ(value, 100);
}

TEST(OptionReferenceTest, UnwrapReturnsExactReferenceType)
{
    int value = 7;
    Option<int&> opt(value);

    static_assert(std::is_same_v<decltype(opt.unwrap()), int&>,
                  "unwrap() must return int&");

    static_assert(std::is_same_v<decltype(std::as_const(opt).unwrap()), int&>,
                  "const Option<int&>::unwrap() must still return int&");
}

TEST(OptionReferenceTest, OperatorStarBehavesLikePointer)
{
    int value = 5;
    Option<int&> opt(value);
    const Option<int&> const_opt(value);

    *opt = 10;
    EXPECT_EQ(value, 10);

    *const_opt = 20;
    EXPECT_EQ(value, 20);
}

TEST(OptionReferenceTest, NoneReferencePanicsOnUnwrap)
{
    Option<int&> none;

    EXPECT_DEATH(
        {
            none.unwrap();
        },
        ""
    );
}
