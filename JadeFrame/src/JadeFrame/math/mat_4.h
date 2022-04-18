#pragma once
#include "JadeFrame/defines.h"
#include "vec.h"

#include <array>
#include <vector>

JF_PRAGMA_PUSH
#if defined(_MSC_VER)
#pragma warning(disable : 4201)
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(__GNUC__)
// #pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
// #pragma GCC diagnostic ignored "-Wnested-anon-types"
#endif
namespace JadeFrame {
/*
        This matrix is column major

        mat[n][m]
        mat[col][row]
        n = columns
        m = rows

        n0 n1 n2 (rank)
         __ __ __
m0	|__|__|__|
m1	|__|__|__|
m2	|__|__|__|

*/
class Matrix4x4;
class Matrix4x4 {
  public:
    constexpr Matrix4x4() noexcept;
    constexpr Matrix4x4(const Matrix4x4&) noexcept = default;
    constexpr Matrix4x4(Matrix4x4&&) noexcept = default;
    ~Matrix4x4() noexcept = default;
    constexpr explicit Matrix4x4(f32 digo) noexcept;

    constexpr auto operator=(const Matrix4x4& mat) noexcept -> Matrix4x4&;
    constexpr auto operator[](const u32 index) noexcept -> std::array<f32, 4>&; // for writing
    constexpr auto operator[](const u32 index) const noexcept
        -> const std::array<f32, 4>&; // for reading
    /*constexpr*/ auto operator*(const v4& vector) const noexcept -> v4;
    constexpr auto     operator*(const Matrix4x4& other) const noexcept -> Matrix4x4;

  public: // static methods for matrices
    constexpr static auto orthogonal_projection_matrix(
        f32 left, f32 right, f32 buttom, f32 top, f32 near, f32 far) noexcept -> Matrix4x4;
    /*constexpr*/ static auto
    perspective_projection_matrix(f32 fovy, f32 aspect, f32 near, f32 far) noexcept -> Matrix4x4;
    constexpr static auto     translation_matrix(const v3& trans) noexcept -> Matrix4x4;
    /*constexpr*/ static auto rotation_matrix(f32 angle, const v3& axis) noexcept -> Matrix4x4;
    /*constexpr*/ static auto scale_matrix(const v3& scale) noexcept -> Matrix4x4;
    constexpr static auto     shear_matrix() noexcept -> Matrix4x4;
    constexpr static auto look_at_matrix(const v3& camera, const v3& object, const v3& up) noexcept
        -> Matrix4x4;

  public:
    constexpr auto get_determinant() const -> f32;
    constexpr auto get_echelon() const -> Matrix4x4;
    constexpr auto get_transpose() const -> Matrix4x4;
    constexpr auto get_inverted() const -> Matrix4x4;

    constexpr auto is_invertible() const -> bool;
    constexpr auto get_rank() const -> i32;

    constexpr auto make_echelon() -> Matrix4x4&;


  private:
    union {
        std::array<std::array<f32, 4>, 4> el;
        // std::array<f32, 4> colVec;
    };
};


// IMPLEMENTATION

inline constexpr Matrix4x4::Matrix4x4() noexcept
    : el() {

    // el[0][0] = 1.0f; el[1][0] = 0.0f; el[2][0] = 0.0f; el[3][0] = 0.0f;
    // el[0][1] = 0.0f; el[1][1] = 1.0f; el[2][1] = 0.0f; el[3][1] = 0.0f;
    // el[0][2] = 0.0f; el[1][2] = 0.0f; el[2][2] = 1.0f; el[3][2] = 0.0f;
    // el[0][3] = 0.0f; el[1][3] = 0.0f; el[2][3] = 0.0f; el[3][3] = 1.0f;
}
inline constexpr Matrix4x4::Matrix4x4(const f32 digo) noexcept
    : el() {
    for (u32 i = 0; i < 4; i++) { el[i][i] = digo; }
}

// inline constexpr Matrix4x4::Matrix4x4(const Matrix4x4& mat) noexcept {
//	*this = mat;
// }

inline constexpr auto Matrix4x4::operator=(const Matrix4x4& mat) noexcept -> Matrix4x4& {
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) { el[col][row] = mat[col][row]; }
    }
    return (*this);
}

inline constexpr auto Matrix4x4::operator[](const u32 index) noexcept -> std::array<f32, 4>& {
    return this->el[index];
}

inline constexpr auto Matrix4x4::operator[](const u32 index) const noexcept
    -> const std::array<f32, 4>& {
    return this->el[index];
}

inline /*constexpr*/ auto Matrix4x4::operator*(const v4& vector) const noexcept -> v4 {
    v4 result;
    result.x =
        el[0][0] * vector.x + el[1][0] * vector.y + el[2][0] * vector.z + el[3][0] * vector.w;
    result.y =
        el[0][1] * vector.x + el[1][1] * vector.y + el[2][1] * vector.z + el[3][1] * vector.w;
    result.z =
        el[0][2] * vector.x + el[1][2] * vector.y + el[2][2] * vector.z + el[3][2] * vector.w;
    result.w =
        el[0][3] * vector.x + el[1][3] * vector.y + el[2][3] * vector.z + el[3][3] * vector.w;
    return result;
}

inline constexpr auto Matrix4x4::operator*(const Matrix4x4& other) const noexcept -> Matrix4x4 {
    Matrix4x4 result(0.0f);
    for (u32 row = 0; row < 4; ++row) {
        for (u32 col = 0; col < 4; ++col) {
            for (u32 k = 0; k < 4; ++k) {
                // result.el[col][row] += el[k][row] * other.el[col][k];
                result.el[col][row] += el[col][k] * other.el[k][row];
            }
        }
    }
    return result;
}

inline constexpr auto Matrix4x4::orthogonal_projection_matrix(
    f32 left, f32 right, f32 bottom, f32 top, f32 zNear, f32 zFar) noexcept -> Matrix4x4 {
    Matrix4x4 result(1);
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = -2.0f / (zFar - zNear);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -(zFar + zNear) / (zFar - zNear);
    return result;
}

inline /*constexpr*/ auto
Matrix4x4::perspective_projection_matrix(f32 fovy, f32 aspect, f32 zNear, f32 zFar) noexcept
    -> Matrix4x4 {
    const f32 tan_half_fovy = static_cast<f32>(tan(fovy / 2.0f));
    Matrix4x4 result(0.0f);
    result.el[0][0] = 1.0f / (aspect * tan_half_fovy);
    result.el[1][1] = 1.0f / tan_half_fovy;
    result.el[2][2] = -(zFar + zNear) / (zFar - zNear);
    result.el[2][3] = -1.0f;
    result.el[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
    return result;
}
inline constexpr auto Matrix4x4::translation_matrix(const v3& trans) noexcept -> Matrix4x4 {
    Matrix4x4 result(1.0f);
    result.el[3][0] = trans.x;
    result.el[3][1] = trans.y;
    result.el[3][2] = trans.z;
    return result;
}
inline /*constexpr*/ auto Matrix4x4::rotation_matrix(f32 angle, const v3& axis) noexcept
    -> Matrix4x4 {
    const f32 c = static_cast<f32>(cos(angle));
    const f32 omc = 1 - c;
    const f32 s = static_cast<f32>(sin(angle));

    Matrix4x4 result(1.0f);
    result.el[0][0] = axis.x * axis.x * omc + c;
    result.el[0][1] = axis.y * axis.x * omc + axis.z * s;
    result.el[0][2] = axis.z * axis.x * omc - axis.y * s;

    result.el[1][0] = axis.x * axis.y * omc - axis.z * s;
    result.el[1][1] = axis.y * axis.y * omc + c;
    result.el[1][2] = axis.z * axis.y * omc + axis.x * s;

    result.el[2][0] = axis.x * axis.z * omc + axis.y * s;
    result.el[2][1] = axis.y * axis.z * omc - axis.x * s;
    result.el[2][2] = axis.z * axis.z * omc + c;
    return result;
}
inline /*constexpr*/ auto Matrix4x4::scale_matrix(const v3& scale) noexcept -> Matrix4x4 {
    Matrix4x4 result(1.0f);
    result.el[0][0] = scale.x;
    result.el[1][1] = scale.y;
    result.el[2][2] = scale.z;
    return result;
}

inline constexpr auto
Matrix4x4::look_at_matrix(const v3& eye, const v3& center, const v3& up) noexcept -> Matrix4x4 {
    v3 const f((center - eye).get_normal());
    v3 const s(f.cross(up).get_normal());
    v3 const u(s.cross(f));

    Matrix4x4 result(1);
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;

    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;

    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;

    result[3][0] = -s.dot(eye);
    result[3][1] = -u.dot(eye);
    result[3][2] = f.dot(eye);
    return result;
}

inline constexpr auto Matrix4x4::get_determinant() const -> f32 {
    const Matrix4x4& m = *this;
    f32              t00 = m[0][0] * m[1][1] * m[2][2] * m[3][3];
    f32              t01 = m[0][0] * m[1][1] * m[3][2] * m[2][3];
    f32              t02 = m[0][0] * m[2][1] * m[1][2] * m[3][3];
    f32              t03 = m[0][0] * m[2][1] * m[3][2] * m[1][3];
    f32              t04 = m[0][0] * m[3][1] * m[1][2] * m[2][3];
    f32              t05 = m[0][0] * m[3][1] * m[2][2] * m[1][3];

    f32 t10 = m[1][0] * m[0][1] * m[2][2] * m[3][3];
    f32 t11 = m[1][0] * m[0][1] * m[3][2] * m[2][3];
    f32 t12 = m[1][0] * m[2][1] * m[0][2] * m[3][3];
    f32 t13 = m[1][0] * m[2][1] * m[3][2] * m[0][3];
    f32 t14 = m[1][0] * m[3][1] * m[0][2] * m[2][3];
    f32 t15 = m[1][0] * m[3][1] * m[2][2] * m[0][3];

    f32 t20 = m[2][0] * m[0][1] * m[1][2] * m[3][3];
    f32 t21 = m[2][0] * m[0][1] * m[3][2] * m[1][3];
    f32 t22 = m[2][0] * m[1][1] * m[0][2] * m[3][3];
    f32 t23 = m[2][0] * m[1][1] * m[3][2] * m[0][3];
    f32 t24 = m[2][0] * m[3][1] * m[0][2] * m[1][3];
    f32 t25 = m[2][0] * m[3][1] * m[1][2] * m[0][3];

    f32 t30 = m[3][0] * m[0][1] * m[1][2] * m[2][3];
    f32 t31 = m[3][0] * m[0][1] * m[2][2] * m[1][3];
    f32 t32 = m[3][0] * m[1][1] * m[0][2] * m[2][3];
    f32 t33 = m[3][0] * m[1][1] * m[2][2] * m[0][3];
    f32 t34 = m[3][0] * m[2][1] * m[0][2] * m[1][3];
    f32 t35 = m[3][0] * m[2][1] * m[1][2] * m[0][3];

    f32 t0 = +t00 - t01 - t02 + t03 + t04 - t05;
    f32 t1 = -t10 + t11 + t12 - t13 - t14 + t15;
    f32 t2 = +t20 - t21 - t22 + t23 + t24 - t25;
    f32 t3 = -t30 + t31 + t32 - t33 - t34 + t35;

    f32 t = t0 + t1 + t2 + t3;

    return t;
}

inline constexpr auto Matrix4x4::get_echelon() const -> Matrix4x4 {
    Matrix4x4 m = *this;
    u32       col_count = 4;
    u32       row_count = 4;
    // go through every column
    for (u32 col = 0; col < col_count; col++) {
        for (u32 row = col + 1; row < row_count; row++) {
            if (m[col][row] != 0) {
                f32 factor = m[col][row] / m[col][col];
                for (u32 col2 = 0; col2 < col_count; col2++) {
                    m[col2][row] -= factor * m[col2][col];
                }
            }
        }
    }
    return m;
}

inline constexpr auto Matrix4x4::is_invertible() const -> bool {
    return (this->get_determinant() == 0) ? false : true;
}

inline constexpr auto Matrix4x4::get_rank() const -> i32 {
    i32  result = 0;
    auto e = this->get_echelon();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) { result += (e[col][row] > 0) ? 1 : 0; }
    }
    return result;
}

inline constexpr auto Matrix4x4::get_transpose() const -> Matrix4x4 {
    Matrix4x4 result;
    for (i32 col = 0; col < 4; col++) {
        for (i32 row = 0; row < 4; row++) { result[col][row] = el[row][col]; }
    }
    return result;
}

inline constexpr auto Matrix4x4::make_echelon() -> Matrix4x4& {
    u32 col_count = 4;
    u32 row_count = 4;
    // go through every column
    for (u32 col = 0; col < col_count; col++) {
        for (u32 row = col + 1; row < row_count; row++) {
            if (el[col][row] != 0) {
                f32 factor = el[col][row] / el[col][col];
                for (u32 col2 = 0; col2 < col_count; col2++) {
                    el[col2][row] -= factor * el[col2][col];
                }
            }
        }
    }
    return *this;
}
} // namespace JadeFrame
JF_PRAGMA_POP