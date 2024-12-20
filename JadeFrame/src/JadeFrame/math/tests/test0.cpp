
#include <gtest/gtest.h>

#include <iostream>
#include <sstream>

#include "JadeFrame/math/math.h"
#include "JadeFrame/math/vec.h"
#include "JadeFrame/prelude.h"
#include "JadeFrame/types.h"

#include "../mat_4.h"
using namespace JadeFrame;

auto matrices_are_equal(const mat4x4& a, const mat4x4& b, float tolerance = 1e-6f)
    -> bool {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (std::fabs(a[row][col] - b[row][col]) > tolerance) { return false; }
        }
    }
    return true;
}

auto vectors_are_equal(const v4& a, const v4& b, float tolerance = 1e-6f) -> bool {
    return std::fabs(a.x - b.x) < tolerance && std::fabs(a.y - b.y) < tolerance &&
           std::fabs(a.z - b.z) < tolerance && std::fabs(a.w - b.w) < tolerance;
}

auto vectors_are_equal(const v3& a, const v3& b, float tolerance = 1e-6f) -> bool {
    return std::fabs(a.x - b.x) < tolerance && std::fabs(a.y - b.y) < tolerance &&
           std::fabs(a.z - b.z) < tolerance;
}

static auto to_string(const mat4x4& mat) -> std::string {
    std::ostringstream out;
    out << "--" << std::endl;
    for (auto i = 0; i < 4; i++) {
        out << "| ";
        for (auto j = 0; j < 4; j++) {
            out << mat[j][i] << " ";
            if (j < 3) { out << ", "; }
        }
        out << "|";
        out << std::endl;
    }
    out << "--" << std::endl;
    return out.str();
}

static auto to_string(const v4& v) -> std::string {
    std::ostringstream out;
    out << "--" << std::endl;
    out << "| " << v.x << " |" << std::endl;
    out << "| " << v.y << " |" << std::endl;
    out << "| " << v.z << " |" << std::endl;
    out << "| " << v.w << " |" << std::endl;
    out << "--" << std::endl;
    return out.str();
}

static auto to_string(const v3& v) -> std::string {
    std::ostringstream out;
    out << "--" << std::endl;
    out << "| " << v.x << " |" << std::endl;
    out << "| " << v.y << " |" << std::endl;
    out << "| " << v.z << " |" << std::endl;
    out << "--" << std::endl;
    return out.str();
}

template<typename T>
auto operator<<(std::ostream& os, const mat4x4& m) -> std::ostream& {
    os << to_string(m);
    return os;
}

// TEST(Mat4x4_BasicCreate, DefaultConstructor) {
//     mat4x4 matrix;
//     for (int i = 0; i < 4; ++i) {
//         for (int j = 0; j < 4; ++j) { EXPECT_FLOAT_EQ(matrix[i][j], 0.0f); }
//     }
// }

TEST(Mat4x4_BasicCreate, FromColsAndFromRows) {
    v4 col1 = v4::create(1, 2, 3, 4);
    v4 col2 = v4::create(5, 6, 7, 8);
    v4 col3 = v4::create(9, 10, 11, 12);
    v4 col4 = v4::create(13, 14, 15, 16);

    mat4x4 matrix = mat4x4::from_cols(col1, col2, col3, col4);
    mat4x4 expected = mat4x4::from_rows(
        v4::create(col1.x, col2.x, col3.x, col4.x),
        v4::create(col1.y, col2.y, col3.y, col4.y),
        v4::create(col1.z, col2.z, col3.z, col4.z),
        v4::create(col1.w, col2.w, col3.w, col4.w)
    );

    EXPECT_EQ(matrix, expected);
}

TEST(Mat4x4_Arithmetic, Addition) {
    mat4x4 m_0 = mat4x4::from_cols(
        v4::create(1, 2, 3, 4),
        v4::create(5, 6, 7, 8),
        v4::create(9, 10, 11, 12),
        v4::create(13, 14, 15, 16)
    );
    mat4x4 m_1 = mat4x4::from_cols(
        v4::create(17, 18, 19, 20),
        v4::create(21, 22, 23, 24),
        v4::create(25, 26, 27, 28),
        v4::create(29, 30, 31, 32)
    );
    {
        mat4x4 result = m_0 + m_1;
        mat4x4 expected = mat4x4::from_cols(
            v4::create(18, 20, 22, 24),
            v4::create(26, 28, 30, 32),
            v4::create(34, 36, 38, 40),
            v4::create(42, 44, 46, 48)
        );
        EXPECT_EQ(result, expected);
    }
    {
        mat4x4 result = m_0 - m_1;
        mat4x4 expected = mat4x4::from_cols(
            v4::create(-16, -16, -16, -16),
            v4::create(-16, -16, -16, -16),
            v4::create(-16, -16, -16, -16),
            v4::create(-16, -16, -16, -16)
        );
        EXPECT_EQ(result, expected);
    }
}

TEST(Mat4x4_Arithmetic, Multiplication) {
    {
        mat4x4 m_0 = mat4x4::from_cols(
            v4::create(1, 2, 3, 4),
            v4::create(5, 6, 7, 8),
            v4::create(9, 10, 11, 12),
            v4::create(13, 14, 15, 16)
        );
        mat4x4 m_1 = mat4x4::from_cols(
            v4::create(17, 18, 19, 20),
            v4::create(21, 22, 23, 24),
            v4::create(25, 26, 27, 28),
            v4::create(29, 30, 31, 32)
        );
        {
            mat4x4 result = m_0 * m_1;
            mat4x4 expected = mat4x4::from_cols(
                v4::create(538, 612, 686, 760),
                v4::create(650, 740, 830, 920),
                v4::create(762, 868, 974, 1080),
                v4::create(874, 996, 1118, 1240)
            );
            EXPECT_EQ(result, expected);
        }
    }

    {
        mat4x4 m = mat4x4::rotation_rh(to_radians(90), v3::Z());

        {
            v3 result = m.transform_vector3(v3::Y());
            EXPECT_EQ(v3::create(-1.0F, 0.0F, 0.0F), result);
            v4 s = m * v4::create(0, 1, 0, 0);
            v3 s_ = v3::create(s.x, s.y, s.z);
            EXPECT_EQ(result, s_);
        }
        {
            v4 result = m * v4::Y();
            EXPECT_EQ(v4::create(-1.0F, 0.0F, 0.0F, 0.0F), result);
            EXPECT_EQ(result, m * v4::Y());
        }
    }

    {
        mat4x4 m = mat4x4::from_scale_rotation_translation(
            v3::create(0.5F, 1.5F, 2.0F),
            mat4x4::rotation_x_rh(to_radians(90)),
            v3::create(1.0F, 2.0F, 3.0F)
        );
        {
            v3 result = m.transform_vector3(v3::Y());
            EXPECT_EQ(v3::create(0.0F, 0.0F, 1.5F), result);
            v4 s = m * v4::create(0, 1, 0, 0);
            v3 s_ = v3::create(s.x, s.y, s.z);
            EXPECT_EQ(result, s_);
        }
        {
            auto result = m.transform_point3(v3::Y());
            EXPECT_EQ(v3::create(1.0F, 2.0F, 4.5F), result);
            auto s = m * v4::create(0, 1, 0, 1);
            auto s_ = v3::create(s.x, s.y, s.z);
            EXPECT_EQ(result, s_);
        }
    }
    {
        auto m = mat4x4::from_cols(
            v4::create(8.0F, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, 4.0F, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, 2.0F, 2.0F),
            v4::create(0.0F, 0.0F, 0.0F, 0.0F)
        );
        EXPECT_EQ(
            v3::create(4.0F, 2.0F, 1.0F), m.project_point3(v3::create(2.0F, 2.0F, 2.0F))
        );
    }
    // {
    //     mat4x4 matrix1 = mat4x4::identity();
    //     mat4x4 matrix2 = mat4x4::identity();
    //     auto   result = matrix1 * matrix2;
    //     auto   b = matrices_are_equal(result, mat4x4::identity());
    //     EXPECT_EQ(b, true);
    // }
}

TEST(Mat4x4_Arithmetic, Decomposition) {
    {
        mat4x4 identity = mat4x4::identity();
        auto [out_scale, out_rotation, out_translation] =
            identity.to_scale_rotation_translation();
        EXPECT_EQ(out_scale, v3::create(1, 1, 1));
        EXPECT_EQ(out_rotation, mat4x4::identity());
        EXPECT_EQ(out_translation, v3::create(0, 0, 0));

        mat4x4 matrix = mat4x4::from_scale_rotation_translation(
            out_scale, out_rotation, out_translation
        );
        EXPECT_EQ(matrix, identity);
    }
    // {
    //     v3     some_scale = v3::create(1, 2, 3);
    //     mat4x4 some_rotation = mat4x4::rotation_x_rh(to_radians(90));
    //     v3     some_translation = v3::create(1, 2, 3);
    //     mat4x4 matrix = mat4x4::from_scale_rotation_translation(
    //         some_scale, some_rotation, some_translation
    //     );
    //     auto [out_scale, out_rotation, out_translation] =
    //         matrix.to_scale_rotation_translation();
    //     EXPECT_EQ(out_scale, some_scale);
    //     std::cout << "out_rotation: " << to_string(out_rotation) << std::endl;
    //     std::cout << "some_rotation: " << to_string(some_rotation) << std::endl;
    //     EXPECT_EQ(out_rotation, some_rotation);
    //     EXPECT_EQ(out_translation, some_translation);
    // }
    // {
    //     mat4x4 matrix = mat4x4::from_rows(
    //         v4::create(1, 0, 0, 0),
    //         v4::create(0, 1, 0, 0),
    //         v4::create(0, 0, 1, 0),
    //         v4::create(1, 2, 3, 1)
    //     );

    //     auto [scale, rotation, translation] = matrix.to_scale_rotation_translation();
    //     EXPECT_EQ(scale, v3::create(1, 1, 1));
    //     EXPECT_EQ(rotation, mat4x4::identity());
    //     EXPECT_EQ(translation, v3::create(1, 2, 3));
    // }
}

TEST(Mat4x4_Arithmetic, MultiplyByZeroMatrix) {
    mat4x4 matrix =
        mat4x4::from_rows({1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16});
    mat4x4 zero = mat4x4::zero();
    EXPECT_EQ(matrix * zero, zero);
    EXPECT_EQ(zero * matrix, zero);
}

TEST(Mat4x4_Arithmetic, MatrixAssociativity) {
    mat4x4 A = mat4x4::from_rows(
        v4::create(1, 3, 5, 9),
        v4::create(1, 3, 1, 7),
        v4::create(4, 3, 9, 7),
        v4::create(5, 2, 0, 9)
    );
    mat4x4 B = mat4x4::from_rows(
        v4::create(2, 4, 6, 0),
        v4::create(7, 5, 3, 1),
        v4::create(8, 6, 4, 2),
        v4::create(0, 9, 7, 8)
    );
    mat4x4 C = mat4x4::from_rows(
        v4::create(9, 8, 7, 6),
        v4::create(0, 6, 4, 2),
        v4::create(3, 7, 5, 1),
        v4::create(1, 3, 5, 7)
    );

    // Check Associativity (A*B)*C ==
    // A*(B*C)
    EXPECT_EQ((A * B) * C, A * (B * C));
}

TEST(Mat4x4_Arithmetic, MultiplyByScalingMatrix) {
    mat4x4 matrix = mat4x4::identity();
    mat4x4 scale = mat4x4::scale(v3::create(2, 2, 2));
    mat4x4 expected =
        mat4x4::from_rows({2, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 2, 0}, {0, 0, 0, 1});
    EXPECT_EQ(matrix * scale, expected);
}

TEST(Mat4x4_Arithmetic, MultiplyByRotationMatrix) {
    {
        float  angle = M_PI / 4; // 45 degrees
        mat4x4 rotate = mat4x4::rotation_y_rh(angle);
        mat4x4 identity = mat4x4::identity();
        mat4x4 result = rotate * identity;
        EXPECT_NEAR(result[0][0], std::cos(angle), 1e-6);
        EXPECT_NEAR(result[0][2], -std::sin(angle), 1e-6);
    }
    // Test whether rotation around the y axis for 90 degrees, results in correct rotation
    // of a unit vector
    {
        v4 unit_vector_x = v4::X();
        {
            std::map<f32, v4> m_x = {
                {  90, v4::X()},
                { -90, v4::X()},
                { 180, v4::X()},
                {-180, v4::X()},
            };
            for (auto [angle_in_degrees, expected] : m_x) {
                mat4x4 rotate = mat4x4::rotation_x_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_x, expected);
            }
            std::map<f32, v4> m_y = {
                {  90, v4::NEG_Z()},
                { -90,     v4::Z()},
                { 180, v4::NEG_X()},
                {-180, v4::NEG_X()},
            };
            for (auto [angle_in_degrees, expected] : m_y) {
                mat4x4 rotate = mat4x4::rotation_y_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_x, expected);
            }

            std::map<f32, v4> m_z = {
                {  90,     v4::Y()},
                { -90, v4::NEG_Y()},
                { 180, v4::NEG_X()},
                {-180, v4::NEG_X()},
            };
            for (auto [angle_in_degrees, expected] : m_z) {
                mat4x4 rotate = mat4x4::rotation_z_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_x, expected);
            }
        }
        v4 unit_vector_y = v4::Y();
        {
            std::map<f32, v4> m_x = {
                {  90,     v4::Z()},
                { -90, v4::NEG_Z()},
                { 180, v4::NEG_Y()},
                {-180, v4::NEG_Y()},
            };
            for (auto [angle_in_degrees, expected] : m_x) {
                mat4x4 rotate = mat4x4::rotation_x_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_y, expected);
            }

            std::map<f32, v4> m_y = {
                {  90, v4::Y()},
                { -90, v4::Y()},
                { 180, v4::Y()},
                {-180, v4::Y()},
            };
            for (auto [angle_in_degrees, expected] : m_y) {
                mat4x4 rotate = mat4x4::rotation_y_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_y, expected);
            }

            std::map<f32, v4> m_z = {
                {  90, v4::NEG_X()},
                { -90,     v4::X()},
                { 180, v4::NEG_Y()},
                {-180, v4::NEG_Y()},
            };
            for (auto [angle_in_degrees, expected] : m_z) {
                mat4x4 rotate = mat4x4::rotation_z_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_y, expected);
            }
        }
        v4 unit_vector_z = v4::Z();
        {
            std::map<f32, v4> m_x = {
                {  90, v4::NEG_Y()},
                { -90,     v4::Y()},
                { 180, v4::NEG_Z()},
                {-180, v4::NEG_Z()},
            };
            for (auto [angle_in_degrees, expected] : m_x) {
                mat4x4 rotate = mat4x4::rotation_x_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_z, expected);
            }

            std::map<f32, v4> m_y = {
                {  90,     v4::X()},
                { -90, v4::NEG_X()},
                { 180, v4::NEG_Z()},
                {-180, v4::NEG_Z()},
            };
            for (auto [angle_in_degrees, expected] : m_y) {
                mat4x4 rotate = mat4x4::rotation_y_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_z, expected);
            }

            std::map<f32, v4> m_z = {
                {  90, v4::Z()},
                { -90, v4::Z()},
                { 180, v4::Z()},
                {-180, v4::Z()},
            };
            for (auto [angle_in_degrees, expected] : m_z) {
                mat4x4 rotate = mat4x4::rotation_z_rh(to_radians(angle_in_degrees));
                EXPECT_EQ(rotate * unit_vector_z, expected);
            }
        }
    }
}

TEST(Mat4x4_Arithmetic, MatrixVectorMultiplication) {
    mat4x4 matrix = mat4x4::identity();
    v4     vector{1, 2, 3, 1};
    v4     result = matrix * vector;

    auto re = result == vector;
    EXPECT_EQ(re, true);
}

TEST(Mat4x4Test, IdentityMatrix) {
    mat4x4 matrix = mat4x4::identity();
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(matrix[i][j], (i == j ? 1.0f : 0.0f));
        }
    }
}

TEST(Mat4x4_Transformation, TranslationMatrix) {
    v3     translation = v3::create(10, 20, 30);
    mat4x4 matrix = mat4x4::translation(translation);
    EXPECT_EQ(matrix[3][0], 10);
    EXPECT_EQ(matrix[3][1], 20);
    EXPECT_EQ(matrix[3][2], 30);
    EXPECT_EQ(matrix[3][3], 1);
}

TEST(Mat4x4_Transformation, Rotation) {
    const f32 degrees = to_degrees((f32)100);

    mat4x4 rot_x1 = mat4x4::rotation_x_rh(degrees);
    mat4x4 rox_x2 = mat4x4::rotation_rh(degrees, v3::X());
    EXPECT_EQ(rot_x1, rox_x2);

    mat4x4 rot_y1 = mat4x4::rotation_y_rh(degrees);
    mat4x4 rox_y2 = mat4x4::rotation_rh(degrees, v3::Y());
    EXPECT_EQ(rot_y1, rox_y2);

    mat4x4 rot_z1 = mat4x4::rotation_z_rh(degrees);
    mat4x4 rox_z2 = mat4x4::rotation_rh(degrees, v3::Z());
    EXPECT_EQ(rot_z1, rox_z2);
}

TEST(Mat4x4_Transformation, RotationXMatrix) {
    float  angle = M_PI / 2; // 90 degrees
    mat4x4 matrix = mat4x4::rotation_x_rh(angle);
    EXPECT_NEAR(matrix[1][1], 0, 1e-6);
    EXPECT_NEAR(matrix[1][2], 1, 1e-6);
    EXPECT_NEAR(matrix[2][1], -1, 1e-6);
    EXPECT_NEAR(matrix[2][2], 0, 1e-6);
}

TEST(Mat4x4_Transformation, ScaleMatrix) {
    {
        v3     scale_factor = v3::create(2, 3, 4);
        mat4x4 matrix = mat4x4::scale(scale_factor);
        EXPECT_EQ(matrix[0][0], 2);
        EXPECT_EQ(matrix[1][1], 3);
        EXPECT_EQ(matrix[2][2], 4);
    }
    {
        mat4x4 m = mat4x4::scale(v3::create(2, 4, 8));
        EXPECT_EQ(v4::X() * 2.0F, m.x_axis);
        EXPECT_EQ(v4::Y() * 4.0F, m.y_axis);
        EXPECT_EQ(v4::Z() * 8.0F, m.z_axis);
        EXPECT_EQ(v4::W(), m.w_axis);
        EXPECT_EQ(
            m.transform_point3(v3::create(1.0F, 1.0F, 1.0F)), v3::create(2.0F, 4.0F, 8.0F)
        );
    }
}

TEST(MISC, main_33) {

    mat4x4 ortho_rh_no =
        mat4x4::orthographic_rh_no(0.0F, 800.0F, 0.0F, 800.0F, -1.0F, 1.0F);
    mat4x4 ortho_rh_zo =
        mat4x4::orthographic_rh_zo(0.0F, 800.0F, 0.0F, 800.0F, -1.0F, 1.0F);
    mat4x4 ortho_lh_no =
        mat4x4::orthographic_lh_no(0.0F, 800.0F, 0.0F, 800.0F, -1.0F, 1.0F);
    mat4x4 ortho_lh_zo =
        mat4x4::orthographic_lh_zo(0.0F, 800.0F, 0.0F, 800.0F, -1.0F, 1.0F);

    v4 pos = v4::create(800.0F, 800.0F, 1.00F, 1.0F);

    // std::cout << "rh_no[-1,1]" << std::endl << to_string(ortho_rh_no * pos) <<
    // std::endl; std::cout << "rh_zo[0,1]" << std::endl << to_string(ortho_rh_zo * pos)
    // << std::endl; std::cout << "lh_no[-1,1]" << std::endl << to_string(ortho_lh_no *
    // pos) << std::endl; std::cout << "lh_zo[0,1]" << std::endl << to_string(ortho_lh_zo
    // * pos) << std::endl;

    // mat4x4 mat =
    // mat4x4::identity();
    // mat[1][0] = -2.0F;
    // mat[0][1] = +2.0F;

    // print(mat);

    // mat4x4 mat2 =
    // mat4x4::from_cols(
    //     v4::create(1.0F, 2.0F, 3.0F,
    //     4.0F),
    //     v4::create(5.0F, 6.0F, 7.0F,
    //     8.0F),
    //     v4::create(9.0F, 10.0F, 11.0F,
    //     12.0F),
    //     v4::create(13.0F, 14.0F, 15.0F,
    //     16.0F)
    // );

    // print(mat2);

    // std::cout << "Hello, World!
    // This is a test 33." <<
    // std::endl;
    EXPECT_EQ(1, 1);
}

inline constexpr auto
perspective_0(f32 left, f32 right, f32 top, f32 bottom, f32 far, f32 near) noexcept
    -> mat4x4 {

    auto map_to_c1 = [](f32 p, f32 q, f32 near, f32 far) -> f32 {
        return ((p - q) * near * far) / (far - near);
    };

    auto map_to_c2 = [](f32 p, f32 q, f32 near, f32 far) -> f32 {
        return -(p * near - q * far) / (far - near);
    };
    mat4x4 translate = mat4x4::identity();
    translate[3][0] = -(right + left) / 2.0F;
    translate[3][1] = -(top + bottom) / 2.0F;

    mat4x4 scale_depth = mat4x4::identity();
    auto   c1 = map_to_c1(-1, 1, near, far);
    auto   c2 = map_to_c2(-1, 1, near, far);
    scale_depth[2][2] = -c2;
    scale_depth[2][3] = -1.0F;
    scale_depth[3][2] = c1;
    scale_depth[3][3] = 0.0F;

    mat4x4 perspective = mat4x4::identity();
    perspective[0][0] = near;
    perspective[1][1] = near;

    mat4x4 scale = mat4x4::identity();
    scale[0][0] = 2.0F / (right - left);
    scale[1][1] = 2.0F / (top - bottom);

    // mat4x4 res = scale *
    // (perspective * (scale_depth
    // * translate));
    mat4x4 res = scale * perspective * scale_depth * translate;

    return res;
}

inline constexpr auto
perspective_1(f32 left, f32 right, f32 top, f32 bottom, f32 far, f32 near) noexcept
    -> mat4x4 {

    const f32 frustum_width = right - left;
    const f32 frustum_height = top - bottom;
    const f32 frustum_depth = far - near;

    return mat4x4::from_cols(
        v4::create(2.0F * near / frustum_width, 0.0F, 0.0F, 0.0F),
        v4::create(0.0F, 2.0F * near / frustum_height, 0.0F, 0.0F),
        v4::create(0.0F, 0.0F, -(far + near) / frustum_depth, -1.0F),
        v4::create(
            -near * (right + left) / frustum_width,
            -near * (top + bottom) / frustum_height,
            -2.0F * far * near / frustum_depth,
            0.0F
        )
    );
}

TEST(Mat4x4_PerspectiveProjection, CustomCompositePerspectiveMatrix) {
    f32 left = 0.0F;
    f32 right = 800.0F;

    f32 bottom = 0.0F;
    f32 top = 800.0F;

    f32 z_near = 0.1F;
    f32 z_far = 100.0F;

    const f32 width = right - left;
    const f32 height = top - bottom;
    const f32 aspect = width / height;
    const f32 fovy = 2.0F * std::atan(top / z_near);

    mat4x4 result_0 = perspective_0(left, right, top, bottom, z_far, z_near);
    mat4x4 result_1 = perspective_1(left, right, top, bottom, z_far, z_near);
    mat4x4 result_2 = mat4x4::perspective_rh_no(left, right, top, bottom, z_near, z_far);

    // std::cout << "Result_0:" << std::endl << to_string(result_0) << std::endl;
    // std::cout << "Result_1:" << std::endl << to_string(result_1) << std::endl;
    // std::cout << "Result_2:" << std::endl << to_string(result_2) << std::endl;

    ASSERT_TRUE(matrices_are_equal(result_0, result_1));
}

TEST(Mat4x4_PerspectiveProjection, NormalParameters_0) {

    f32 left = 0.0F;
    f32 right = 800.0F;

    f32 bottom = 0.0F;
    f32 top = 800.0F;

    f32 z_near = 0.1F;
    f32 z_far = 100.0F;

    const f32 width = right - left;
    const f32 height = top - bottom;
    const f32 aspect = width / height;
    const f32 fovy = 2.0F * std::atan(top / z_near);

    mat4x4 result_0 = mat4x4::perspective_rh_no(fovy, aspect, z_near, z_far);
    mat4x4 result_1 = mat4x4::perspective_rh_no(left, right, top, bottom, z_near, z_far);
    mat4x4 result_2 = perspective_0(left, right, top, bottom, z_far, z_near);

    // std::cout << "Result_0:" << std::endl << to_string(result_0) << std::endl;
    // std::cout << "Result_1:" << std::endl << to_string(result_1) << std::endl;
    // std::cout << "Result_2:" << std::endl << to_string(result_2) << std::endl;

    ASSERT_TRUE(matrices_are_equal(result_0, result_1));
}

// Test edge case with near and far
// planes very close
TEST(Mat4x4_PerspectiveProjection, NearFarEdgeCase) {
    float fovy = M_PI / 4; // 45 degrees
    float aspect = 1.0f;   // square aspect
                           // ratio
    float z_near = 1.0f;
    float z_far = 1.01f; // very close near
                         // and far plane

    mat4x4 result = mat4x4::perspective_rh_no(fovy, aspect, z_near, z_far);
    float  expected_z3 = -1.0f * ((z_far + z_near) / (z_far - z_near));
    float  expected_z4 = (-2.0f * z_far * z_near) / (z_far - z_near);

    // We can only test the third
    // column here due to the rest
    // being standard calculations
    ASSERT_NEAR(result[2][2], expected_z3, 1e-5);
    ASSERT_NEAR(result[3][2], expected_z4, 1e-5);
}

// Test extreme field of view
TEST(Mat4x4_PerspectiveProjection, ExtremeFieldOfView) {
    float fovy = M_PI / 100;     // very narrow field of view
    float aspect = 16.0f / 9.0f; // widescreen aspect ratio
    float z_near = 0.5f;
    float z_far = 50.0f;

    mat4x4 result = mat4x4::perspective_rh_no(fovy, aspect, z_near, z_far);
    float  expected_focal_length = 1.0f / math::tan(fovy / 2.0f);

    // Check if the focal length
    // calculation is correct
    ASSERT_NEAR(result[1][1], expected_focal_length, 1e-5);
    ASSERT_NEAR(result[0][0], expected_focal_length / aspect, 1e-5);
}

TEST(Mat4x4_PerspectiveProjection, Basic) {
    const f32 near = 0.1f;
    const f32 far = 100.0f;
    const f32 eps = 1e-6f;

    const f32 aspect = 4.0F / 3.0F;
    const f32 fovy = M_PI * 0.25F;

    mat4x4 projection_rh_no = mat4x4::perspective_rh_no(fovy, aspect, near, far);
    {
        v4 n = projection_rh_no * v4::create(0.0F, 0.0F, -near, 1.0F);
        v4 f = projection_rh_no * v4::create(0.0F, 0.0F, -far, 1.0F);
        n /= n.w;
        f /= f.w;
        EXPECT_NEAR(n.z, -1.0F, eps);
        EXPECT_NEAR(f.z, 1.0F, eps);
    }

    mat4x4 projection_rh_zo = mat4x4::perspective_rh_zo(fovy, aspect, near, far);
    {
        v4 n = projection_rh_zo * v4::create(0.0F, 0.0F, -near, 1.0F);
        v4 f = projection_rh_zo * v4::create(0.0F, 0.0F, -far, 1.0F);
        n /= n.w;
        f /= f.w;
        EXPECT_NEAR(n.z, 0.0F, eps);
        EXPECT_NEAR(f.z, 1.0F, eps);
    }
    mat4x4 projection_lh_no = mat4x4::perspective_lh_no(fovy, aspect, near, far);
    {
        v4 n = projection_lh_no * v4::create(0.0F, 0.0F, near, 1.0F);
        v4 f = projection_lh_no * v4::create(0.0F, 0.0F, far, 1.0F);
        n /= n.w;
        f /= f.w;
        EXPECT_NEAR(n.z, -1.0F, eps);
        EXPECT_NEAR(f.z, 1.0F, eps);
    }
    mat4x4 projection_lh_zo = mat4x4::perspective_lh_zo(fovy, aspect, near, far);
    {
        v4 n = projection_lh_zo * v4::create(0.0F, 0.0F, near, 1.0F);
        v4 f = projection_lh_zo * v4::create(0.0F, 0.0F, far, 1.0F);
        n /= n.w;
        f /= f.w;
        EXPECT_NEAR(n.z, 0.0F, eps);
        EXPECT_NEAR(f.z, 1.0F, eps);
    }
}

TEST(Mat4x4_PerspectiveProjection, Basic2) {
    mat4x4 projection = mat4x4::perspective_rh_no(to_radians(90.0F), 2.0F, 5.0F, 15.0F);
    {
        v3   original = v3::create(5.0, 5.0, -15.0);
        v4   projected = projection * v4::create(original, 1.0F);
        bool is_same = projected == v4::create(2.5, 5.0, 15.0, 15.0);
        EXPECT_EQ(is_same, true);
    }

    {
        v3   original = v3::create(5.0, 5.0, -5.0);
        v4   projected = projection * v4::create(original, 1.0F);
        bool is_same = projected == v4::create(2.5, 5.0, -5.0, 5.0);
        EXPECT_EQ(is_same, true);
    }
}

TEST(Mat4x4_OrthographicProjection, Basics) {
    v3 camera_position = v3::create(0.0F, 0.0F, -5.0F);
    v3 world_up = v3::Y();

    v3 point = v3::create(0.0F, 0.0F, 0.0F);

    mat4x4 look = mat4x4::look_at_rh(camera_position, v3::zero(), world_up);

    const f32 left = 0.0F;
    const f32 right = 800.0F;
    const f32 bottom = 0.0F;
    const f32 top = 600.0F;
    const f32 near = 1.0F;
    const f32 far = 100.0F;

    mat4x4 ortho = mat4x4::orthographic_rh_no(left, right, bottom, top, near, far);

    std::cout << "p 0: " << to_string(point) << std::endl;
    std::cout << "p 1: " << to_string(look.transform_point3(point)) << std::endl;
    std::cout << "p 2: " << to_string((ortho * look).transform_point3(point))
              << std::endl;

    EXPECT_EQ(true, true);
}

TEST(Mat4x4_LookAt, Basics) {
    v3 eye = v3::create(0.0F, 0.0F, -5.0F);
    v3 center = v3::create(0.0F, 0.0F, 0.0F);
    v3 up = v3::create(1.0F, 0.0F, 0.0F);

    v3 point = v3::create(1.0F, 0.0F, 0.0F);
    {
        mat4x4 lh = mat4x4::look_at_lh(eye, center, up);
        EXPECT_EQ(lh.transform_point3(point), v3::create(0.0F, 1.0, 5.0));

        mat4x4 rh = mat4x4::look_at_rh(eye, center, up);
        EXPECT_EQ(rh.transform_point3(point), v3::create(0.0F, 1.0, -5.0));
    }
    {
        v3 direction = center - eye;

        mat4x4 lh = mat4x4::look_to_lh(eye, direction, up);
        EXPECT_EQ(lh.transform_point3(point), v3::create(0.0F, 1.0, 5.0));
        mat4x4 rh = mat4x4::look_to_rh(eye, direction, up);
        EXPECT_EQ(rh.transform_point3(point), v3::create(0.0F, 1.0, -5.0));
    }
}
