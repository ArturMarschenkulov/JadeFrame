

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

#include "JadeFrame/math/math.h"
#include "JadeFrame/math/vec.h"
#include "JadeFrame/math/math.h"
#include "JadeFrame/prelude.h"
#include "JadeFrame/types.h"
#include "map"
using namespace JadeFrame;

TEST(Math_Angles, Basics) {
    {
        for (int i = 0; i < 360; i++) {
            f32 radian = to_radians((f32)i);
            f32 degree = to_degrees(radian);
            EXPECT_NEAR(i, degree, 1e-4);
        }

        for (int i = 0; i < 360; i++) {
            f64 radian = to_radians((f64)i);
            f64 degree = to_degrees(radian);
            EXPECT_NEAR(i, degree, 1e-5);
        }
    }
}

TEST(Math_Angles, ToRadians) {
    std::map<f32, f32> mapping_degrees_to_radians = {
        std::pair{  0.0F,               0.0F},
        std::pair{  5.0F,       M_PI / 36.0F},
        std::pair{ 15.0F,       M_PI / 12.0F},
        std::pair{ 30.0F,        M_PI / 6.0F},
        std::pair{ 36.0F,        M_PI / 5.0F},
        std::pair{ 45.0F,        M_PI / 4.0F},
        std::pair{ 60.0F,        M_PI / 3.0F},
        std::pair{ 72.0F, 2.0F * M_PI / 5.0F},
        std::pair{ 90.0F,        M_PI / 2.0F},
        std::pair{120.0F, 2.0F * M_PI / 3.0F},
        std::pair{144.0F, 4.0F * M_PI / 5.0F},
        std::pair{180.0F,               M_PI},
        std::pair{270.0F, 3.0F * M_PI / 2.0F},
        std::pair{360.0F,        2.0F * M_PI},
    };

    for (auto& [degree, radian] : mapping_degrees_to_radians) {
        auto result = to_radians(degree);
        EXPECT_NEAR(result, radian, 1e-20);
    }
}

TEST(Math_Constexpr_Angles, General) {
    const f32 max_degree = 360.0F;
    const f32 degree_step = 0.01F;
    // compare math::sin and std::sin
    {
        for (f32 i = 0; i < max_degree; i += degree_step) {
            f32 radian = to_radians((f32)i);
            f32 sin = static_cast<f32>(math::sin_constexpr(radian, 13));
            f32 sin_std = std::sin(radian);
            EXPECT_NEAR(sin, sin_std, 1e-6);
        }

        for (f32 i = 0; i < max_degree; i += degree_step) {
            f32 radian = to_radians((f32)i);
            f32 sin = static_cast<f32>(math::sin_constexpr(radian, 14));
            f32 sin_std = std::sin(radian);
            EXPECT_NEAR(sin, sin_std, 1e-7);
        }
    }
    // compare math::cos and std::cos
    {
        for (f32 i = 0; i < max_degree; i += degree_step) {
            f32 radian = to_radians((f32)i);
            f32 cos = static_cast<f32>(math::cos_constexpr(radian, 14));
            f32 cos_std = std::cos(radian);
            EXPECT_NEAR(cos, cos_std, 1e-6);
        }
        for (f32 i = 0; i < max_degree; i += degree_step) {
            f32 radian = to_radians((f32)i);
            f32 cos = static_cast<f32>(math::cos_constexpr(radian, 15));
            f32 cos_std = std::cos(radian);
            EXPECT_NEAR(cos, cos_std, 1e-7);
        }
    }
    // compare math::tan and std::tan
    {
        for (f32 i = 0; i < max_degree; i += degree_step) {
            f32 radian = to_radians((f32)i);
            f32 tan = static_cast<f32>(math::tan_constexpr(radian, 16));
            f32 tan_std = std::tan(radian);
            EXPECT_NEAR(tan, tan_std, 1e-6);
        }
    }
    // // compare math::atan and std::tan
    // {
    //     // for f32
    //     f32 eps_f32_approx = 0.01F;
    //     for (f32 i = 0; i < 360; i += eps_f32_approx) {
    //         f32 radian = to_radians((f32)i);
    //         f32 atan = math::atan_constexpr(radian, 50);
    //         f32 atan_std = std::atan(radian);
    //         EXPECT_NEAR(atan, atan_std, 1e-3);
    //     }
    // }
}