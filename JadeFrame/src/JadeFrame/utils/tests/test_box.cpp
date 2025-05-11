#include <gtest/gtest.h>
#include <type_traits>
#include "JadeFrame/utils/box.h"

using namespace JadeFrame;

class Rectangle {
public:
    static auto create(int x, int y) -> Rectangle { return Rectangle(x, y); }

public:
    int x;
    int y;
};

TEST(Box, Basics) {
    Box<Rectangle> box = Box<Rectangle>::make(Rectangle::create, 10, 20);
    EXPECT_EQ(box->x, 10);
    EXPECT_EQ(box->y, 20);
}