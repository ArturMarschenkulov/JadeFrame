#pragma once
#include "JadeFrame/prelude.h"
#include "JadeFrame/types.h"
#include "vec.h"
#include "JadeFrame/math/math.h"

#include <cassert>

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

constexpr static auto get_fovy_aspect(f32 left, f32 right, f32 top, f32 bottom, f32 near)
    -> std::tuple<f32, f32> {
    assert((left != right) && "left and right cannot be the same");
    assert((top != bottom) && "top and bottom cannot be the same");
    assert((left < right) && "left must be less than right");
    assert((bottom < top) && "bottom must be less than top");

    const f64 width = right - left;
    const f64 height = top - bottom;
    const f64 aspect = width / height;
    const f64 fovy = 2.0 * std::atan((f64)top / (f64)near);
    return {fovy, aspect};
}

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
    using Col = std::array<f32, 4>;

    constexpr auto operator[](const u32 index) noexcept -> Col&; // for writing
    constexpr auto operator[](const u32 index
    ) const noexcept -> const Col&; // for reading

    constexpr auto operator*(const f32& scalar) const noexcept -> mat4x4 {
        mat4x4 result = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) { result[i][j] = (*this)[i][j] * scalar; }
        }
        return result;
    }

    constexpr auto operator*(const v4& vector) const noexcept -> v4;
    constexpr auto operator*(const mat4x4& other) const noexcept -> mat4x4;

    constexpr auto operator+(const mat4x4& other) const noexcept -> mat4x4 {
        mat4x4 result = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) { result[i][j] = (*this)[i][j] + other[i][j]; }
        }
        return result;
    }

    constexpr auto operator-(const mat4x4& other) const noexcept -> mat4x4 {
        mat4x4 result = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) { result[i][j] = (*this)[i][j] - other[i][j]; }
        }
        return result;
    }

    [[nodiscard]] constexpr auto
    cmp(const mat4x4& other, f32 epsilon = 1e-6F) const noexcept -> bool {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (std::fabs((*this)[i][j] - other[i][j]) > epsilon) { return false; }
            }
        }
        return true;
    }

    constexpr auto operator==(const mat4x4& b) const noexcept -> bool {
        return this->cmp(b);
    }

    constexpr auto operator!=(const mat4x4& other) const noexcept -> bool {
        return !(*this == other);
    }

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

    constexpr static auto
    from_rows(const v4& row1, const v4& row2, const v4& row3, const v4& row4) noexcept
        -> mat4x4 {
        return mat4x4::from_cols(
            v4::create(row1.x, row2.x, row3.x, row4.x),
            v4::create(row1.y, row2.y, row3.y, row4.y),
            v4::create(row1.z, row2.z, row3.z, row4.z),
            v4::create(row1.w, row2.w, row3.w, row4.w)
        );
    }

    [[nodiscard]] constexpr auto
    to_scale_rotation_translation() const noexcept -> std::tuple<v3, mat4x4, v3> {
        f32 det = this->determinant();

        v3 scale = v3::create(
            x_axis.length() * (det < 0 ? -1 : 1), y_axis.length(), z_axis.length()

        );

        v3     inv_scale = scale.reciprocal();
        mat4x4 rotation = mat4x4::from_rows(
            x_axis * inv_scale.x, y_axis * inv_scale.y, z_axis * inv_scale.z, v4::W()
        );

        v3 translation = v3::create(w_axis.x, w_axis.y, w_axis.z);
        return {scale, rotation, translation};
    }

    constexpr static auto
    from_scale_rotation_translation(v3 scale, mat4x4 rotation, v3 translation) noexcept
        -> mat4x4 {
        return mat4x4::translation(translation) * rotation * mat4x4::scale(scale);
    }

    constexpr static auto orthographic_rh_no(
        f32 left,
        f32 right,
        f32 bottom,
        f32 top,
        f32 z_near,
        f32 z_far
    ) noexcept -> mat4x4;

    constexpr static auto orthographic_lh_no(
        f32 left,
        f32 right,
        f32 bottom,
        f32 top,
        f32 z_near,
        f32 z_far
    ) noexcept -> mat4x4 {
        const f32 frustum_width = right - left;
        const f32 frustum_height = top - bottom;
        const f32 frustum_depth = z_far - z_near;

        const f32 range = 2.0F;

        f32 a = range / frustum_width;
        f32 b = range / frustum_height;
        f32 c = range / frustum_depth;

        f32 tx = -(right + left) / frustum_width;
        f32 ty = -(top + bottom) / frustum_height;
        f32 tz = -(z_far + z_near) / frustum_depth;

        return mat4x4::from_cols(
            v4::create(a, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, b, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, c, 0.0F),
            v4::create(tx, ty, tz, 1.0F)
        );
    }

    constexpr static auto orthographic_lh_zo(
        f32 left,
        f32 right,
        f32 bottom,
        f32 top,
        f32 z_near,
        f32 z_far
    ) noexcept -> mat4x4 {

        const f32 frustum_width = right - left;
        const f32 frustum_height = top - bottom;
        const f32 frustum_depth = z_far - z_near;

        const f32 range = 2.0F;

        f32 a = range / frustum_width;
        f32 b = range / frustum_height;
        f32 c = 1.0F / frustum_depth;

        f32 tx = -(right + left) / frustum_width;
        f32 ty = -(top + bottom) / frustum_height;
        f32 tz = -(z_near) / frustum_depth;

        return mat4x4::from_cols(
            v4::create(a, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, b, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, c, 0.0F),
            v4::create(tx, ty, tz, 1.0F)
        );
    }

    constexpr static auto orthographic_rh_zo(
        f32 left,
        f32 right,
        f32 bottom,
        f32 top,
        f32 z_near,
        f32 z_far
    ) noexcept -> mat4x4 {
        const f32 frustum_width = right - left;
        const f32 frustum_height = top - bottom;
        const f32 frustum_depth = z_far - z_near;

        const f32 range = 2.0F;

        f32 a = range / frustum_width;
        f32 b = range / frustum_height;
        f32 z = -1.0F / frustum_depth;

        f32 tx = -(right + left) / frustum_width;
        f32 ty = -(top + bottom) / frustum_height;
        f32 tz = -(z_near) / frustum_depth;

        return mat4x4::from_cols(
            v4::create(a, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, b, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, z, 0.0F),
            v4::create(tx, ty, tz, 1.0F)
        );
    }

    constexpr static auto
    frustum(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) -> mat4x4 {
        const f32 frustum_width = right - left;
        const f32 frustum_height = top - bottom;
        const f32 frustum_depth = far - near;

        const f32 range = 2.0F * near;

        f32 a = (2.0F * near) / frustum_width;
        f32 b = (2.0F * near) / frustum_height;

        f32 tx = (right + left) / frustum_width;
        f32 ty = (top + bottom) / frustum_height;
        f32 tz = -(far + near) / frustum_depth;

        f32 d = -(2.0F * far * near) / frustum_depth;

        return mat4x4::from_cols(
            v4::create(a, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, b, 0.0F, 0.0F),
            v4::create(tx, ty, tz, -1.0F),
            v4::create(0.0F, 0.0F, d, 0.0F)
        );
    }

    constexpr static auto perspective_rh_no(
        f32 left,
        f32 right,
        f32 top,
        f32 bottom,
        f32 near,
        f32 far
    ) noexcept -> mat4x4 {
        auto [fovy, aspect] = get_fovy_aspect(left, right, top, bottom, near);
        return mat4x4::perspective_rh_no(fovy, aspect, near, far);
    }

    /// Creates a right-handed perspective projection matrix with depth range of [-1, 1].
    ///
    /// Mainly used in OpenGL.
    constexpr static auto
    perspective_rh_no(f32 fovy, f32 aspect, f32 near, f32 far) noexcept -> mat4x4;

    /// Creates a right-handed perspective projection matrix with depth range of [0, 1].
    ///
    /// Mainly used in Direct3D. In case of Vulkan, one has to do proj[1][1] *= -1;, aka
    /// flip the y-axis.
    constexpr static auto
    perspective_rh_zo(f32 fovy, f32 aspect, f32 z_near, f32 z_far) noexcept -> mat4x4 {

        const f32 frustum_depth = z_far - z_near;
        const f32 focal_length = 1.0F / math::tan(fovy / 2.0F);
        // const auto focal_length = math::cos(fovy / 2.0F) / math::sin(fovy / 2.0F);
        const f32 z_factor = -1.0F;

        f32 _1 = focal_length / aspect;
        f32 _2 = focal_length;
        f32 _3 = z_factor * (z_far / frustum_depth);
        f32 _4 = (-1.0F * z_near * z_far) / frustum_depth;
        f32 _5 = z_factor;

        return mat4x4::from_cols(
            v4::create(_1, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, _2, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, _3, _5),
            v4::create(0.0F, 0.0F, _4, 0.0F)
        );
    }

    constexpr static auto
    perspective_lh_no(f32 fovy, f32 aspect, f32 z_near, f32 z_far) noexcept -> mat4x4 {
        const f32 frustum_depth = z_far - z_near;
        const f32 focal_length = 1.0F / math::tan(fovy / 2.0F);
        // const f32 focal_length = math::cos(fovy / 2.0F) / math::sin(fovy / 2.0F);
        const f32 z_factor = 1.0F;

        f32 _1 = focal_length / aspect;
        f32 _2 = focal_length;
        f32 _3 = z_factor * ((z_far + z_near) / frustum_depth);
        f32 _4 = (-2.0F * z_near * z_far) / frustum_depth;
        f32 _5 = z_factor;

        return mat4x4::from_cols(
            v4::create(_1, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, _2, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, _3, _5),
            v4::create(0.0F, 0.0F, _4, 0.0F)
        );
    }

    constexpr static auto
    perspective_lh_zo(f32 fovy, f32 aspect, f32 z_near, f32 z_far) noexcept -> mat4x4 {
        const f32 frustum_depth = z_far - z_near;
        const f32 focal_length = 1.0F / math::tan(fovy / 2.0F);
        // const auto focal_length = math::cos(fovy / 2.0F) / math::sin(fovy / 2.0F);

        const f32 z_factor = 1.0F;

        const f32 z_range_min = 0.0F;
        const f32 z_range_max = 1.0F;
        const f32 c_0 = z_range_max * z_far - z_range_min * z_near;
        const f32 c_1 = (z_range_min - z_range_max) * z_near * z_far;

        const f32 _1 = focal_length / aspect;
        const f32 _2 = focal_length;
        const f32 _3 = z_factor * (c_0 / frustum_depth);
        const f32 _4 = c_1 / frustum_depth;
        const f32 _5 = z_factor;

        return mat4x4::from_cols(
            v4::create(_1, 0.0F, 0.0F, 0.0F),
            v4::create(0.0F, _2, 0.0F, 0.0F),
            v4::create(0.0F, 0.0F, _3, _5),
            v4::create(0.0F, 0.0F, _4, 0.0F)
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

    constexpr static auto rotation_rh(f32 angle, const v3& axis) noexcept -> mat4x4;

    static auto rotation_x_rh(f32 angle) noexcept -> mat4x4 {
        // return mat4x4::rotation_rh(angle, v3::X());
        auto sina = static_cast<f32>(math::sin(angle));
        auto cosa = static_cast<f32>(math::cos(angle));
        return mat4x4::from_cols(
            v4::X(),
            v4::create(0.0F, cosa, sina, 0.0F),
            v4::create(0.0F, -sina, cosa, 0.0F),
            v4::W()
        );
    }

    static auto rotation_y_rh(f32 angle) noexcept -> mat4x4 {
        // return mat4x4::rotation_rh(angle, v3::Y());
        auto sina = static_cast<f32>(math::sin(angle));
        auto cosa = static_cast<f32>(math::cos(angle));
        return mat4x4::from_cols(
            v4::create(cosa, 0.0F, -sina, 0.0F),
            v4::Y(),
            v4::create(sina, 0.0F, cosa, 0.0F),
            v4::W()
        );
    }

    static auto rotation_z_rh(f32 angle) noexcept -> mat4x4 {
        // return mat4x4::rotation_rh(angle, v3::Z());
        auto sina = static_cast<f32>(math::sin(angle));
        auto cosa = static_cast<f32>(math::cos(angle));
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

    /// Creates a right-handed look-at view matrix.
    ///
    /// The resulting matrix transforms a point from the world space to the view space.
    /// For coordinate system with +x=right, +y=up, +z=back.
    constexpr static auto
    look_to_rh(const v3& eye, const v3& direction, const v3& up) noexcept -> mat4x4 {
        v3 forward = direction.normalize();
        v3 new_right = forward.cross(up).normalize();
        v3 new_up = new_right.cross(forward);

        return mat4x4::from_cols(
            v4::create(new_right.x, new_up.x, -forward.x, 0.0F),
            v4::create(new_right.y, new_up.y, -forward.y, 0.0F),
            v4::create(new_right.z, new_up.z, -forward.z, 0.0F),
            v4::create(-new_right.dot(eye), -new_up.dot(eye), forward.dot(eye), 1.0F)
        );
    }

    /// Creates a left-handed look-at view matrix.
    ///
    /// The resulting matrix transforms a point from the world space to the view space.
    /// For coordinate system with +x=right, +y=up, +z=forward.
    constexpr static auto
    look_to_lh(const v3& eye, const v3& direction, const v3& up) -> mat4x4 {
        return mat4x4::look_to_rh(eye, -direction, up);
    }

    constexpr static auto
    look_at_lh(const v3& eye, const v3& target, const v3& up) -> mat4x4 {
        return mat4x4::look_to_lh(eye, target - eye, up);
    }

    constexpr static auto
    look_at_rh(const v3& eye, const v3& target, const v3& up) -> mat4x4 {
        return mat4x4::look_to_rh(eye, target - eye, up);
    }

public:
    [[nodiscard]] constexpr auto transform_vector3(const v3& vec) const noexcept -> v3 {
        v4 result = *this * v4::from_v3(vec, 0.0F);
        return v3::create(result.x, result.y, result.z);
    }

    [[nodiscard]] constexpr auto transform_point3(const v3& point) const noexcept -> v3 {
        if (w_axis == v4::W()) {
            assert(w_axis == v4::W() && "The w component of the w_axis must be 1.0F");
        }
        v4 result = *this * v4::from_v3(point, 1.0F);
        return v3::create(result.x, result.y, result.z);
    }

    [[nodiscard]] constexpr auto project_point3(const v3& point) const noexcept -> v3 {
        v4 result = *this * v4::from_v3(point, 1.0F);
        result = result * v4::create(result.w, result.w, result.w, result.w).reciprocal();
        return v3::create(result.x, result.y, result.z);
    }

public:
    [[nodiscard]] constexpr auto determinant() const -> f32;
    [[nodiscard]] constexpr auto get_echelon() const -> mat4x4;
    [[nodiscard]] constexpr auto get_transpose() const -> mat4x4;

    [[nodiscard]] constexpr auto inverse() const -> mat4x4 {
        f32 det = this->determinant();
        if (det == 0.0F) { return mat4x4::zero(); }
        f32 inv_det = 1.0F / det;

        f32 coeff00 = el[2][2] * el[3][3] - el[3][2] * el[2][3];
        f32 coeff02 = el[1][2] * el[3][3] - el[3][2] * el[1][3];
        f32 coeff03 = el[1][2] * el[2][3] - el[2][2] * el[1][3];

        f32 coeff04 = el[2][1] * el[3][3] - el[3][1] * el[2][3];
        f32 coeff06 = el[1][1] * el[3][3] - el[3][1] * el[1][3];
        f32 coeff07 = el[1][1] * el[2][3] - el[2][1] * el[1][3];

        f32 coeff08 = el[2][1] * el[3][2] - el[3][1] * el[2][2];
        f32 coeff10 = el[1][1] * el[3][2] - el[3][1] * el[1][2];
        f32 coeff11 = el[1][1] * el[2][2] - el[2][1] * el[1][2];

        f32 coeff12 = el[2][0] * el[3][3] - el[3][0] * el[2][3];
        f32 coeff14 = el[1][0] * el[3][3] - el[3][0] * el[1][3];
        f32 coeff15 = el[1][0] * el[2][3] - el[2][0] * el[1][3];

        f32 coeff16 = el[2][0] * el[3][2] - el[3][0] * el[2][2];
        f32 coeff18 = el[1][0] * el[3][2] - el[3][0] * el[1][2];
        f32 coeff19 = el[1][0] * el[2][2] - el[2][0] * el[1][2];

        f32 coeff20 = el[2][0] * el[3][1] - el[3][0] * el[2][1];
        f32 coeff22 = el[1][0] * el[3][1] - el[3][0] * el[1][1];
        f32 coeff23 = el[1][0] * el[2][1] - el[2][0] * el[1][1];

        v4 fac0 = v4::create(coeff00, coeff00, coeff02, coeff03);
        v4 fac1 = v4::create(coeff04, coeff04, coeff06, coeff07);
        v4 fac2 = v4::create(coeff08, coeff08, coeff10, coeff11);
        v4 fac3 = v4::create(coeff12, coeff12, coeff14, coeff15);
        v4 fac4 = v4::create(coeff16, coeff16, coeff18, coeff19);
        v4 fac5 = v4::create(coeff20, coeff20, coeff22, coeff23);

        v4 vec0 = v4::create(el[1][0], el[0][0], el[0][0], el[0][0]);
        v4 vec1 = v4::create(el[1][1], el[0][1], el[0][1], el[0][1]);
        v4 vec2 = v4::create(el[1][2], el[0][2], el[0][2], el[0][2]);
        v4 vec3 = v4::create(el[1][3], el[0][3], el[0][3], el[0][3]);

        v4 inv0 = vec1 * fac0 - vec2 * fac1 + vec3 * fac2;
        v4 inv1 = vec0 * fac0 - vec2 * fac3 + vec3 * fac4;
        v4 inv2 = vec0 * fac1 - vec1 * fac3 + vec3 * fac5;
        v4 inv3 = vec0 * fac2 - vec1 * fac4 + vec2 * fac5;

        v4 sign_a = v4::create(1.0F, -1.0F, 1.0F, -1.0F);
        v4 sign_b = v4::create(-1.0F, 1.0F, -1.0F, 1.0F);

        mat4x4 inverse =
            mat4x4::from_cols(inv0 * sign_a, inv1 * sign_b, inv2 * sign_a, inv3 * sign_b);

        v4  col0 = v4::create(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);
        v4  dot0 = this->x_axis * col0;
        f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

        f32 rcp_det = 1.0F / dot1;
        return inverse * rcp_det;
    }

    [[nodiscard]] constexpr auto is_invertible() const -> bool;
    [[nodiscard]] constexpr auto get_rank() const -> i32;

    [[nodiscard]] constexpr auto make_echelon() const -> mat4x4;

public:
    union {
        std::array<Col, 4> el;
        std::array<v4, 4>  col_vec;

        struct {
            v4 x_axis;
            v4 y_axis;
            v4 z_axis;
            v4 w_axis;
        };
    };
};

// IMPLEMENTATION

static auto operator<<(std::ostream& os, const mat4x4& v) -> std::ostream& {
    os << '{';
    os << '{' << v[0][0] << ", " << v[0][1] << ", " << v[0][2] << ", " << v[0][3] << '}';
    os << ',';
    os << '{' << v[1][0] << ", " << v[1][1] << ", " << v[1][2] << ", " << v[1][3] << '}';
    os << ',';
    os << '{' << v[2][0] << ", " << v[2][1] << ", " << v[2][2] << ", " << v[2][3] << '}';
    os << ',';
    os << '{' << v[3][0] << ", " << v[3][1] << ", " << v[3][2] << ", " << v[3][3] << '}';
    return os;
}

inline constexpr auto mat4x4::operator=(const mat4x4& mat) noexcept -> mat4x4& {
    if (this == &mat) { return *this; }
    for (u32 col = 0; col < 4; col++) {
        for (u32 row = 0; row < 4; row++) { el[col][row] = mat[col][row]; }
    }
    return (*this);
}

inline constexpr auto mat4x4::operator[](const u32 index
) noexcept -> std::array<f32, 4>& {
    return this->el[index];
}

inline constexpr auto mat4x4::operator[](const u32 index
) const noexcept -> const std::array<f32, 4>& {
    return this->el[index];
}

inline constexpr auto mat4x4::operator*(const v4& vector) const noexcept -> v4 {
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
                // result.el[col][row] += el[col][k] * other.el[k][row];
                result.el[col][row] += el[k][row] * other.el[col][k];
            }
        }
    }
    return result;
}

inline constexpr auto mat4x4::orthographic_rh_no(
    f32 left,
    f32 right,
    f32 bottom,
    f32 top,
    f32 z_near,
    f32 z_far
) noexcept -> mat4x4 {
    const f32 frustum_width = right - left;
    const f32 frustum_height = top - bottom;
    const f32 frustum_depth = z_far - z_near;

    const f32 range = 2.0F;

    const f32 a = range / frustum_width;
    const f32 b = range / frustum_height;
    const f32 c = -range / frustum_depth;

    const f32 tx = -(right + left) / frustum_width;
    const f32 ty = -(top + bottom) / frustum_height;
    const f32 tz = -(z_far + z_near) / frustum_depth;

    return mat4x4::from_cols(
        v4::create(a, 0.0F, 0.0F, 0.0F),
        v4::create(0.0F, b, 0.0F, 0.0F),
        v4::create(0.0F, 0.0F, c, 0.0F),
        v4::create(tx, ty, tz, 1.0F)
    );
}

inline constexpr auto
perspe_0(f32 left, f32 right, f32 top, f32 bottom, f32 far, f32 near) noexcept -> mat4x4 {

    enum API {
        DEFAULT,
        VULKAN,
    };

    f32  y_dir = {};
    auto api = API::VULKAN;
    if (api == API::DEFAULT) {
        y_dir = 1;
    } else if (api == API::VULKAN) {
        y_dir = -1;
    }

    auto map_to_c1 = [](f32 to_near, f32 to_far, f32 from_near, f32 from_far) -> f32 {
        return ((to_near - to_far) * from_near * from_far) / (from_far - from_near);
    };

    auto map_to_c2 = [](f32 to_near, f32 to_far, f32 from_near, f32 from_far) -> f32 {
        return -(to_near * from_near - to_far * from_far) / (from_far - from_near);
    };

    auto map_to_c1_ = [](f32 near_to, f32 far_to, f32 near_from, f32 far_from) -> f32 {
        f32 depth_from = far_from - near_from;
        f32 depth_to = far_to - near_to;
        f32 scale = depth_to / depth_from;
        return -near_from * far_from * scale;
    };
    auto map_to_c2_ = [](f32 near_to, f32 far_to, f32 near_from, f32 far_from) -> f32 {
        f32 depth_from = far_from - near_from;
        return -(near_to * near_from - far_to * far_from) / depth_from;
    };
    mat4x4 translate = mat4x4::identity();
    translate[3][0] = -(right + left) / 2.0F;
    translate[3][1] = -(top + bottom) / 2.0F;

    mat4x4 scale_depth = mat4x4::identity();
    auto   c1 = map_to_c1(-1, 1, near, far);
    auto   c2 = map_to_c2(-1, 1, near, far);
    scale_depth[2][2] = c1;
    scale_depth[2][3] = -1.0F;
    scale_depth[3][2] = c2;

    mat4x4 perspective = mat4x4::identity();
    perspective[0][0] = near;
    perspective[1][1] = near;

    mat4x4 scale = mat4x4::identity();
    scale[0][0] = 2.0F / (right - left);
    scale[1][1] = 2.0F / (top - bottom);
    scale[1][1] *= y_dir;

    mat4x4 res = scale * perspective * scale_depth * translate;
    return res;
}

inline constexpr auto
mat4x4::perspective_rh_no(f32 fovy, f32 aspect, f32 z_near, f32 z_far) noexcept
    -> mat4x4 {

    const f32 frustum_depth = z_far - z_near;
    const f32 focal_length = 1.0F / math::tan(fovy / 2.0F);
    const f32 z_factor = -1.0F;

    f32 _1 = focal_length / aspect;
    f32 _2 = focal_length;
    f32 _3 = z_factor * ((z_far + z_near) / frustum_depth);
    f32 _4 = (-2.0F * z_far * z_near) / frustum_depth;
    f32 _5 = z_factor;

    mat4x4 result = mat4x4::from_cols(
        v4::create(_1, 0.0F, 0.0F, 0.0F),
        v4::create(0.0F, _2, 0.0F, 0.0F),
        v4::create(0.0F, 0.0F, _3, _5),
        v4::create(0.0F, 0.0F, _4, 0.0F)
    );

    return result;
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

inline constexpr auto mat4x4::rotation_rh(f32 angle, const v3& axis) noexcept -> mat4x4 {
    const f32 c = static_cast<f32>(math::cos(angle));
    const f32 s = static_cast<f32>(math::sin(angle));

    const f32 omc = 1 - c;

    const v3 axis_sin = axis * s;
    const v3 axis_sq = axis * axis;

    const f32 xy_omc = axis.x * axis.y * omc;
    const f32 xz_omc = axis.x * axis.z * omc;
    const f32 yz_omc = axis.y * axis.z * omc;

    mat4x4 result = mat4x4::identity();
    result.el[0][0] = axis_sq.x * omc + c;
    result.el[0][1] = xy_omc + axis_sin.z;
    result.el[0][2] = xz_omc - axis_sin.y;
    result.el[0][3] = 0.0F;

    result.el[1][0] = xy_omc - axis_sin.z;
    result.el[1][1] = axis_sq.y * omc + c;
    result.el[1][2] = yz_omc + axis_sin.x;
    result.el[1][3] = 0.0F;

    result.el[2][0] = xz_omc + axis_sin.y;
    result.el[2][1] = yz_omc - axis_sin.x;
    result.el[2][2] = axis_sq.z * omc + c;
    result.el[2][3] = 0.0F;

    result.el[3][0] = 0.0F;
    result.el[3][1] = 0.0F;
    result.el[3][2] = 0.0F;
    result.el[3][3] = 1.0F;

    return result;
}

inline constexpr auto mat4x4::determinant() const -> f32 {
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
    return this->determinant() != 0;
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

inline constexpr auto mat4x4::make_echelon() const -> mat4x4 {

    mat4x4 result = *this;
    u32    col_count = 4;
    u32    row_count = 4;
    // go through every column
    for (u32 col = 0; col < col_count; col++) {
        for (u32 row = col + 1; row < row_count; row++) {
            if (result[col][row] != 0) {
                f32 factor = result[col][row] / result[col][col];
                for (u32 col2 = 0; col2 < col_count; col2++) {
                    result[col2][row] -= factor * result[col2][col];
                }
            }
        }
    }
    return result;
}

class Quaternion {
public:
    constexpr Quaternion() noexcept = default;
    ~Quaternion() noexcept = default;

    constexpr Quaternion(const Quaternion&) noexcept = default;
    constexpr auto operator=(const Quaternion& mat) noexcept -> Quaternion&;
    constexpr Quaternion(Quaternion&&) noexcept = default;
    constexpr auto operator=(Quaternion&& mat) noexcept -> Quaternion& = default;

public: // Creation / named constructors
    /// Creates a new quaternion from raw components.
    static inline constexpr auto
    create(f32 x, f32 y, f32 z, f32 w) noexcept -> Quaternion {
        Quaternion quat = {};
        quat.x = x;
        quat.y = y;
        quat.z = z;
        quat.w = w;
        return quat;
    }

    /// Returns the identity quaternion.
    static inline constexpr auto identity() noexcept -> Quaternion {
        return Quaternion::create(0.0F, 0.0F, 0.0F, 1.0F);
    }

    /// Creates a quaternion from an axis and angle (in radians).
    ///
    /// \param axis Must be normalized (or will be normalized internally).
    /// \param angle Rotation in radians.
    static inline constexpr auto
    from_axis_angle(const v3& axis_in, f32 angle) noexcept -> Quaternion {
        v3  axis = axis_in.normalize(); // or do your own normalization
        f32 half_angle = angle * 0.5F;
        f32 s = static_cast<f32>(math::sin(half_angle));
        f32 c = static_cast<f32>(math::cos(half_angle));
        return Quaternion::create(axis.x * s, axis.y * s, axis.z * s, c);
    }

    static inline constexpr auto
    from_euler(f32 pitch, f32 yaw, f32 roll) noexcept -> Quaternion {
        // For a standard Y-up, R_z(roll)*R_y(yaw)*R_x(pitch) approach:
        f32 half_x = pitch * 0.5F;
        f32 half_y = yaw * 0.5F;
        f32 half_z = roll * 0.5F;

        f32 sx = static_cast<f32>(math::sin(half_x));
        f32 cx = static_cast<f32>(math::cos(half_x));
        f32 sy = static_cast<f32>(math::sin(half_y));
        f32 cy = static_cast<f32>(math::cos(half_y));
        f32 sz = static_cast<f32>(math::sin(half_z));
        f32 cz = static_cast<f32>(math::cos(half_z));

        // This matches a common Z-Y-X convention for Euler angles
        // (roll around Z, yaw around Y, pitch around X).
        Quaternion q;
        q.x = sx * cy * cz + cx * sy * sz;
        q.y = cx * sy * cz - sx * cy * sz;
        q.z = cx * cy * sz - sx * sy * cz;
        q.w = cx * cy * cz + sx * sy * sz;
        return q;
    }

    /// Converts a (normalized) quaternion to a 4x4 rotation matrix (RH).
    static inline auto to_mat4x4(const Quaternion& q_in) noexcept -> mat4x4 {
        Quaternion q = q_in.normalize();

        const f32 xx = q.x * q.x;
        const f32 yy = q.y * q.y;
        const f32 zz = q.z * q.z;
        const f32 xy = q.x * q.y;
        const f32 xz = q.x * q.z;
        const f32 yz = q.y * q.z;
        const f32 wx = q.w * q.x;
        const f32 wy = q.w * q.y;
        const f32 wz = q.w * q.z;

        mat4x4 m = mat4x4::identity();
        // Row-major (if you store row-wise). But recall mat4x4 is column-major:
        //   so each `m[i][0..3]` is the i-th column. Adjust carefully.
        // We'll fill it out in a column-major way:

        m[0][0] = 1.0F - 2.0F * (yy + zz);
        m[1][0] = 2.0F * (xy + wz);
        m[2][0] = 2.0F * (xz - wy);

        m[0][1] = 2.0F * (xy - wz);
        m[1][1] = 1.0F - 2.0F * (xx + zz);
        m[2][1] = 2.0F * (yz + wx);

        m[0][2] = 2.0F * (xz + wy);
        m[1][2] = 2.0F * (yz - wx);
        m[2][2] = 1.0F - 2.0F * (xx + yy);

        // translation row
        m[3][0] = 0.0F;
        m[3][1] = 0.0F;
        m[3][2] = 0.0F;
        // last column
        m[0][3] = 0.0F;
        m[1][3] = 0.0F;
        m[2][3] = 0.0F;
        m[3][3] = 1.0F;

        return m;
    }

public: // Basic quaternion operations
    /// Quaternion addition.
    inline constexpr auto operator+(const Quaternion& other
    ) const noexcept -> Quaternion {
        return Quaternion::create(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    /// Quaternion subtraction.
    inline constexpr auto operator-(const Quaternion& other
    ) const noexcept -> Quaternion {
        return Quaternion::create(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    /// Negate the quaternion (unary minus).
    inline constexpr auto operator-() const noexcept -> Quaternion {
        return Quaternion::create(-x, -y, -z, -w);
    }

    /// Scalar multiplication.
    inline constexpr auto operator*(f32 scalar) const noexcept -> Quaternion {
        return Quaternion::create(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    /// Quaternion multiplication (combines rotations).
    ///
    /// \note Order matters: q1 * q2 means "apply q2, then q1".
    inline constexpr auto operator*(const Quaternion& other
    ) const noexcept -> Quaternion {
        // Hamilton product:
        // (w1*x2 + x1*w2 + y1*z2 - z1*y2, ...)
        // A common reference formula:
        // q = (w1*x2 + x1*w2 + y1*z2 - z1*y2,
        //      w1*y2 - x1*z2 + y1*w2 + z1*x2,
        //      w1*z2 + x1*y2 - y1*x2 + z1*w2,
        //      w1*w2 - x1*x2 - y1*y2 - z1*z2)
        Quaternion result;
        result.x = w * other.x + x * other.w + y * other.z - z * other.y;
        result.y = w * other.y - x * other.z + y * other.w + z * other.x;
        result.z = w * other.z + x * other.y - y * other.x + z * other.w;
        result.w = w * other.w - x * other.x - y * other.y - z * other.z;
        return result;
    }

    /// Dot product of two quaternions.
    inline constexpr static auto
    dot(const Quaternion& a, const Quaternion& b) noexcept -> f32 {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    /// Equality operator with small tolerance for floating-point comparisons.
    inline constexpr auto operator==(const Quaternion& rhs) const noexcept -> bool {
        f32 tolerance = 1e-6F;
        return (
            std::abs(x - rhs.x) < tolerance && std::abs(y - rhs.y) < tolerance &&
            std::abs(z - rhs.z) < tolerance && std::abs(w - rhs.w) < tolerance
        );
    }

    inline constexpr auto operator!=(const Quaternion& rhs) const noexcept -> bool {
        return !(*this == rhs);
    }

public: // Quaternion-specific operations
    /// Conjugate of the quaternion: ( -x, -y, -z, w ).
    inline constexpr auto conjugate() const noexcept -> Quaternion {
        return Quaternion::create(-x, -y, -z, w);
    }

    /// Length (norm) of the quaternion.
    inline constexpr auto length() const noexcept -> f32 {
        auto d = Quaternion::dot(*this, *this);
        return static_cast<f32>(std::sqrt(d));
    }

    /// Returns whether this quaternion is a unit quaternion (within some tolerance).
    inline constexpr auto is_normalized(f32 eps = 1e-6F) const noexcept -> bool {
        return std::abs(Quaternion::dot(*this, *this) - 1.0F) < eps;
    }

    /// Returns a normalized version of this quaternion (does not modify the original).
    inline constexpr auto normalize() const noexcept -> Quaternion {
        f32 len = this->length();
        if (len <= 1e-8F) {
            // Degenerate quaternion, fallback to identity or zero?
            return Quaternion::create(0.0F, 0.0F, 0.0F, 1.0F);
        }
        f32 inv_len = 1.0F / len;
        return (*this) * inv_len;
    }

    /// Normalizes this quaternion in-place.
    inline constexpr auto normalize() noexcept -> Quaternion& {
        f32 len = this->length();
        if (len > 1e-8F) {
            f32 inv_len = 1.0F / len;
            x *= inv_len;
            y *= inv_len;
            z *= inv_len;
            w *= inv_len;
        } else {
            // If length is extremely small, default to identity or zero?
            x = 0.0F;
            y = 0.0F;
            z = 0.0F;
            w = 1.0F;
        }
        return *this;
    }

    /// Inverse of the quaternion = conjugate / (length^2).
    inline constexpr auto inverse() const noexcept -> Quaternion {
        f32 len_sq = Quaternion::dot(*this, *this);
        ;
        if (len_sq <= 1e-8F) {
            // Degenerate; can't invert. Return identity or something safe
            return Quaternion::identity();
        }
        f32        inv_len_sq = 1.0F / len_sq;
        Quaternion conj = this->conjugate();
        return conj * inv_len_sq;
    }

public: // 3D vector transform
    /// Rotates a 3D vector by this quaternion.
    ///
    /// Note: This can be done in multiple ways; this is a straightforward approach:
    ///   v' = q * (0, x, y, z) * q^-1
    [[nodiscard]] inline constexpr auto rotate_vector(const v3& v) const noexcept -> v3 {
        // Convert to quaternion
        Quaternion vec_q = Quaternion::create(v.x, v.y, v.z, 0.0F);
        Quaternion inv_q = this->inverse();
        // q * vec * q^-1
        Quaternion res = (*this) * vec_q * inv_q;
        return v3::create(res.x, res.y, res.z);
    }

public: // Spherical Linear Interpolation
    /// Spherical linear interpolation between two quaternions (t in [0,1]).
    ///
    /// Both quaternions should be normalized. If not, slerp might still work, but
    /// typically they're expected to be unit quaternions.
    static inline auto
    slerp(const Quaternion& a, const Quaternion& b, f32 t) noexcept -> Quaternion {
        // Clamp t
        if (t < 0.0F) { t = 0.0F; }
        if (t > 1.0F) { t = 1.0F; }

        // Compute the cosine of the angle between the two quaternions
        f32 cos_theta = Quaternion::dot(a, b);
        // If rotating in the wrong "direction", flip one quaternion
        Quaternion b_cpy = b;
        if (cos_theta < 0.0F) {
            b_cpy = -b_cpy; // flip
            cos_theta = -cos_theta;
        }
        // If close enough, use linear interpolation
        const f32 eps = 1e-5F;
        if (cos_theta > 1.0F - eps) {
            // Lerp
            Quaternion result = a * (1.0F - t) + b_cpy * t;
            return result.normalize();
        }

        // Perform the actual slerp
        f32        angle = static_cast<f32>(std::acos(cos_theta));
        f32        inv_sin = 1.0F / math::sin(angle);
        f32        t1 = math::sin((1.0F - t) * angle) * inv_sin;
        f32        t2 = math::sin(t * angle) * inv_sin;
        Quaternion result = a * t1 + b_cpy * t2;
        return result;
    }

public:
    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

/// Optional stream operator for convenient logging:
inline auto operator<<(std::ostream& os, const Quaternion& q) -> std::ostream& {
    os << "{ x=" << q.x << ", y=" << q.y << ", z=" << q.z << ", w=" << q.w << " }";
    return os;
}
} // namespace JadeFrame

JF_PRAGMA_POP