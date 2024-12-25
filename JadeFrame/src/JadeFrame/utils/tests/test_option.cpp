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
