#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/types.h"
#include "vec.h"

#include <array>

JF_PRAGMA_PUSH

// To maintain the high convenience of anonymous structs and unions, we disable the
// warnings
#if defined(__clang__)
    #pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
    #pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wpedantic"
    #pragma GCC diagnostic ignored "-Wshadow"
#elif defined(_MSC_VER)
    #pragma warning(disable : 4201)
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
class mat4x4;

class Quaternion {
public:

public:
    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

class mat4x4 {
private:

public:
    constexpr mat4x4() noexcept = default;
    ~mat4x4() noexcept = default;

    constexpr mat4x4(const mat4x4&) noexcept = default;
    constexpr auto operator=(const mat4x4& mat) noexcept -> mat4x4&;
    constexpr mat4x4(mat4x4&&) noexcept = default;
    constexpr auto operator=(mat4x4&& mat) noexcept -> mat4x4& = default;

public:
    constexpr auto operator[](const u32 index) noexcept
        -> std::array<f32, 4>&; // for writing
    constexpr auto operator[](const u32 index) const noexcept
        -> const std::array<f32, 4>&; // for reading
    /*constexpr*/ auto operator*(const v4& vector) const noexcept -> v4;
    constexpr auto     operator*(const mat4x4& other) const noexcept -> mat4x4;

public: // static methods for matrices
    constexpr static auto
    from_cols(const v4& col1, const v4& col2, const v4& col3, const v4& col4) noexcept
        -> mat4x4 {
        mat4x4 result = {};
        result.col_vec[0] = col1;
        result.col_vec[1] = col2;
        result.col_vec[2] = col3;
        result.col_vec[3] = col4;
        return result;
    }

    constexpr static auto orthographic_rh_gl(
        f32 left,
        f32 right,
        f32 bottom,
        f32 top,
        f32 z_near,
        f32 z_far
    ) noexcept -> mat4x4;

    constexpr static auto
    orthographic_lh(f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far)
        -> mat4x4 {
        auto rcp_width = 1.0F / (right - left);
        auto rcp_height = 1.0F / (top - bottom);
        auto rcp_depth = 1.0F / (z_far - z_near);

        return mat4x4::from_cols(
            v4::create(2.0F * rcp_width, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, 2.0F * rcp_height, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, rcp_depth, 0.0F),
            v4::create(
                -(right + left) * rcp_width,
                -(top + bottom) * rcp_height,
                -z_near * rcp_depth,
                1.0F
            )
        );
    }

    constexpr static auto
    orthographic_rh(f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far)
        -> mat4x4 {
        auto rcp_width = 1.0F / (right - left);
        auto rcp_height = 1.0F / (top - bottom);
        auto rcp_depth = 1.0F / (z_near - z_far);

        return mat4x4::from_cols(
            v4::create(2.0F * rcp_width, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, 2.0F * rcp_height, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, rcp_depth, 0.0F),
            v4::create(
                -(right + left) * rcp_width,
                -(top + bottom) * rcp_height,
                z_near * rcp_depth,
                1.0F
            )
        );
    }

    /*constexpr*/ static auto
    perspective_rh_gl(f32 fovy, f32 aspect, f32 near, f32 far) noexcept -> mat4x4;

    /*constexpr*/ static auto
    perspective_rh(f32 fovy, f32 aspect, f32 z_near, f32 z_far) noexcept -> mat4x4 {
        auto sin_fov = static_cast<f32>(std::sin(fovy / 2.0F));
        auto cos_fov = static_cast<f32>(std::cos(fovy / 2.0F));
        auto h = cos_fov / sin_fov;
        auto w = h / aspect;
        auto r = z_far / (z_near - z_far);
        return mat4x4::from_cols(
            v4::create(w, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, h, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, r, -1.0F),
            v4::create(0.0F, 0.0F, r * z_near, 0.0F)
        );
    }

    /*constexpr*/ static auto
    perspective_lh(f32 fovy, f32 aspect, f32 z_near, f32 z_far) noexcept -> mat4x4 {
        auto sin_fov = static_cast<f32>(std::sin(fovy / 2.0F));
        auto cos_fov = static_cast<f32>(std::cos(fovy / 2.0F));
        auto h = cos_fov / sin_fov;
        auto w = h / aspect;
        auto r = z_far / (z_near - z_far);
        return mat4x4::from_cols(
            v4::create(w, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, h, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, r, 1.0F),
            v4::create(0.0F, 0.0F, -r * z_near, 0.0F)
        );
    }

    constexpr static auto identity() noexcept -> mat4x4;
    constexpr static auto zero() noexcept -> mat4x4;
    constexpr static auto one() noexcept -> mat4x4;
    constexpr static auto diagonal(f32 diag) noexcept -> mat4x4;
    constexpr static auto diagonal(const v4& diag) noexcept -> mat4x4;

    constexpr static auto translation(const v3& trans) noexcept -> mat4x4 {
        return mat4x4::from_cols(
            v4::X(), //
            v4::Y(),
            v4::Z(),
            v4::create(trans.x, trans.y, trans.z, 1.0F)
        );
    }

    /*constexpr*/ static auto rotation(f32 angle, const v3& axis) noexcept -> mat4x4;
    static auto               rotation_x(f32 angle) noexcept -> mat4x4 {
        // return mat4x4::rotation(angle, v3::X());
        auto sina = static_cast<f32>(sin(angle));
        auto cosa = static_cast<f32>(cos(angle));
        return mat4x4::from_cols(
            v4::X(),
            v4::create(0.0F, cosa, sina, 0.0F),
            v4::create(0.0F, -sina, cosa, 0.0F),
            v4::W()
        );
    }

    static auto rotation_y(f32 angle) noexcept -> mat4x4 {
        // return mat4x4::rotation(angle, v3::Y());
        auto sina = static_cast<f32>(sin(angle));
        auto cosa = static_cast<f32>(cos(angle));
        return mat4x4::from_cols(
            v4::create(cosa, 0.0F, -sina, 0.0F),
            v4::Y(),
            v4::create(sina, 0.0F, cosa, 0.0F),
            v4::W()
        );
    }

    static auto rotation_z(f32 angle) noexcept -> mat4x4 {
        // return mat4x4::rotation(angle, v3::Z());
        auto sina = static_cast<f32>(sin(angle));
        auto cosa = static_cast<f32>(cos(angle));
        return mat4x4::from_cols(
            v4::create(cosa, sina, 0.0F, 0.0F),
            v4::create(-sina, cosa, 0.0F, 0.0F),
            v4::Z(),
            v4::W()
        );
    }

    constexpr static auto scale(const v3& scale) noexcept -> mat4x4 {
        return mat4x4::from_cols(
            v4::create(scale.x, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, scale.y, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, scale.z, 0.0F),
            v4::W()

        );
    }

    constexpr static auto
    look_to_rh(const v3& eye, const v3& direction, const v3& up) noexcept -> mat4x4 {
        auto forward = direction.get_normal();
        auto s = forward.cross(up).get_normal();
        auto u = s.cross(forward);

        return mat4x4::from_cols(
            v4(s.x, u.x, -forward.x, 0.0F),
            v4(s.y, u.y, -forward.y, 0.0F),
            v4(s.z, u.z, -forward.z, 0.0F),
            v4(-s.dot(eye), -u.dot(eye), forward.dot(eye), 1.0F)
        );
    }

    constexpr static auto look_to_lh(const v3& eye, const v3& direction, const v3& up)
        -> mat4x4 {
        return mat4x4::look_to_rh(eye, -direction, up);
    }

    constexpr static auto look_at_lh(const v3& eye, const v3& target, const v3& up)
        -> mat4x4 {
        return mat4x4::look_to_lh(eye, target - eye, up);
    }

    constexpr static auto look_at_rh(const v3& eye, const v3& target, const v3& up)
        -> mat4x4 {
        return mat4x4::look_to_rh(eye, target - eye, up);
    }

public:
    [[nodiscard]] constexpr auto get_determinant() const -> f32;
    [[nodiscard]] constexpr auto get_echelon() const -> mat4x4;
    [[nodiscard]] constexpr auto get_transpose() const -> mat4x4;
    [[nodiscard]] constexpr auto get_inverted() const -> mat4x4;

    [[nodiscard]] constexpr auto is_invertible() const -> bool;
    [[nodiscard]] constexpr auto get_rank() const -> i32;

    constexpr auto make_echelon() -> mat4x4&;

private:
    union {
        std::array<std::array<f32, 4>, 4> el;
        std::array<v4, 4>                 col_vec;
    };
};

// IMPLEMENTATION

inline constexpr auto mat4x4::operator=(const mat4x4& mat) noexcept -> mat4x4& {
    if (this == &mat) { return *this; }
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) { el[col][row] = mat[col][row]; }
    }
    return (*this);
}

inline constexpr auto mat4x4::operator[](const u32 index) noexcept
    -> std::array<f32, 4>& {
    return this->el[index];
}

inline constexpr auto mat4x4::operator[](const u32 index) const noexcept
    -> const std::array<f32, 4>& {
    return this->el[index];
}

inline /*constexpr*/ auto mat4x4::operator*(const v4& vector) const noexcept -> v4 {
    v4          result;
    const auto& v = vector;
    result.x = el[0][0] * v.x + el[1][0] * v.y + el[2][0] * v.z + el[3][0] * v.w;
    result.y = el[0][1] * v.x + el[1][1] * v.y + el[2][1] * v.z + el[3][1] * v.w;
    result.z = el[0][2] * v.x + el[1][2] * v.y + el[2][2] * v.z + el[3][2] * v.w;
    result.w = el[0][3] * v.x + el[1][3] * v.y + el[2][3] * v.z + el[3][3] * v.w;
    return result;
}

inline constexpr auto mat4x4::operator*(const mat4x4& other) const noexcept -> mat4x4 {
    mat4x4 result = mat4x4::zero();
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

inline constexpr auto mat4x4::orthographic_rh_gl(
    f32 left,
    f32 right,
    f32 bottom,
    f32 top,
    f32 z_near,
    f32 z_far
) noexcept -> mat4x4 {
    const auto range = 2.0F;
    const auto a = range / (right - left);
    const auto b = range / (top - bottom);
    const auto c = -range / (z_far - z_near);
    const auto tx = -(right + left) / (right - left);
    const auto ty = -(top + bottom) / (top - bottom);
    const auto tz = -(z_far + z_near) / (z_far - z_near);

    return mat4x4::from_cols(
        v4::create(a, 0.0F, 0.0F, 0.0F),
        v4::create(0.0F, b, 0.0F, 0.0F),
        v4::create(0.0F, 0.0F, c, 0.0F),
        v4::create(tx, ty, tz, 1.0F)
    );
}

inline /*constexpr*/ auto
mat4x4::perspective_rh_gl(f32 fovy, f32 aspect, f32 z_near, f32 z_far) noexcept
    -> mat4x4 {
    auto tan_half_fovy = static_cast<f32>(std::tan(fovy / 2.0F));
    auto zoom = 1.0F / tan_half_fovy;

    auto a = 1.0F / (aspect * tan_half_fovy);
    auto b = zoom;
    auto c = -(z_far + z_near) / (z_far - z_near);
    auto d = -(2.0F * z_far * z_near) / (z_far - z_near);
    return mat4x4::from_cols(
        v4::create(a, 0.0F, 0.0F, 0.0F),
        v4::create(0.0F, b, 0.0F, 0.0F),
        v4::create(0.0F, 0.0F, c, -1.0F),
        v4::create(0.0F, 0.0F, d, 0.0F)
    );

    {
        auto inv_length = 1.0F / (z_near - z_far);
        auto zoom = 1.0F / static_cast<f32>(std::tan(0.5F * fovy));
        auto a = zoom / aspect;
        auto b = zoom;
        auto c = (z_near + z_far) * inv_length;
        auto d = (2.0F * z_near * z_far) * inv_length;
        return mat4x4::from_cols(
            v4::create(a, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, b, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, c, -1.0F),
            v4::create(0.0F, 0.0F, d, 0.0F)
        );
    }
}

inline constexpr auto mat4x4::identity() noexcept -> mat4x4 {
    return mat4x4::diagonal(1.0F);
}

inline constexpr auto mat4x4::zero() noexcept -> mat4x4 {
    return mat4x4::from_cols(v4::zero(), v4::zero(), v4::zero(), v4::zero());
}

inline constexpr auto mat4x4::one() noexcept -> mat4x4 { return mat4x4::diagonal(1.0F); }

inline constexpr auto mat4x4::diagonal(f32 diag) noexcept -> mat4x4 {
    return mat4x4::diagonal(v4::splat(diag));
}

inline constexpr auto mat4x4::diagonal(const v4& diag) noexcept -> mat4x4 {
    mat4x4 m = mat4x4::zero();
    m.el[0][0] = diag.x;
    m.el[1][1] = diag.y;
    m.el[2][2] = diag.z;
    m.el[3][3] = diag.w;
    return m;
}

inline /*constexpr*/ auto mat4x4::rotation(f32 angle, const v3& axis) noexcept -> mat4x4 {
    const f32 c = static_cast<f32>(cos(angle));
    const f32 omc = 1 - c;
    const f32 s = static_cast<f32>(sin(angle));

    mat4x4 result = mat4x4::identity();
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

inline constexpr auto mat4x4::get_determinant() const -> f32 {
    const mat4x4& m = *this;

    f32 t00 = m[0][0] * m[1][1] * m[2][2] * m[3][3];
    f32 t01 = m[0][0] * m[1][1] * m[3][2] * m[2][3];
    f32 t02 = m[0][0] * m[2][1] * m[1][2] * m[3][3];
    f32 t03 = m[0][0] * m[2][1] * m[3][2] * m[1][3];
    f32 t04 = m[0][0] * m[3][1] * m[1][2] * m[2][3];
    f32 t05 = m[0][0] * m[3][1] * m[2][2] * m[1][3];

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

inline constexpr auto mat4x4::get_echelon() const -> mat4x4 {
    mat4x4 m = *this;
    u32    col_count = 4;
    u32    row_count = 4;
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

inline constexpr auto mat4x4::is_invertible() const -> bool {
    return this->get_determinant() != 0;
}

inline constexpr auto mat4x4::get_rank() const -> i32 {
    i32  result = 0;
    auto e = this->get_echelon();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) { result += (e[col][row] > 0) ? 1 : 0; }
    }
    return result;
}

inline constexpr auto mat4x4::get_transpose() const -> mat4x4 {
    mat4x4 result = mat4x4::zero();
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) { result[col][row] = el[row][col]; }
    }
    return result;
}

inline constexpr auto mat4x4::make_echelon() -> mat4x4& {
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