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
    using Col = std::array<f32, 4>;

    constexpr auto operator[](const u32 index) noexcept -> Col&; // for writing
    constexpr auto operator[](const u32 index
    ) const noexcept -> const Col&; // for reading
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

    constexpr auto operator==(const mat4x4& b) const noexcept -> bool {
        float tolerance = 1e-6f;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                if (std::fabs((*this)[row][col] - b[row][col]) > tolerance) {
                    return false;
                }
            }
        }
        return true;
    }

    constexpr auto operator!=(const mat4x4& other) const noexcept -> bool;

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
        f32 det = this->get_determinant();

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
        const f32 width = right - left;
        const f32 height = top - bottom;
        const f32 aspect = width / height;
        const f32 fovy = 2.0F * std::atan(top / near);
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
        v3 s = forward.cross(up).normalize();
        v3 u = s.cross(forward);

        return mat4x4::from_cols(
            v4::create(s.x, u.x, -forward.x, 0.0F),
            v4::create(s.y, u.y, -forward.y, 0.0F),
            v4::create(s.z, u.z, -forward.z, 0.0F),
            v4::create(-s.dot(eye), -u.dot(eye), forward.dot(eye), 1.0F)
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
        v4 res = v4::zero();
        res = (x_axis * vec.x) + res;
        res = (y_axis * vec.y) + res;
        res = (z_axis * vec.z) + res;
        return v3::create(res.x, res.y, res.z);
    }

    [[nodiscard]] constexpr auto transform_point3(const v3& point) const noexcept -> v3 {
        if (w_axis == v4::W()) {
            std::cout << w_axis << std::endl;
            assert(w_axis == v4::W() && "The w component of the w_axis must be 1.0F");
        }

        v4 res = v4::zero();
        res = (x_axis * point.x) + res;
        res = (y_axis * point.y) + res;
        res = (z_axis * point.z) + res;
        res = (w_axis * 1.0F) + res;
        return v3::create(res.x, res.y, res.z);
    }

    [[nodiscard]] constexpr auto project_point3(const v3& point) const noexcept -> v3 {

        v4 res = v4::zero();
        res = (x_axis * point.x) + res;
        res = (y_axis * point.y) + res;
        res = (z_axis * point.z) + res;
        res = (w_axis * 1.0F) + res;
        res = res * v4::create(res.w, res.w, res.w, res.w).reciprocal();
        return v3::create(res.x, res.y, res.z);
    }

public:
    [[nodiscard]] constexpr auto get_determinant() const -> f32;
    [[nodiscard]] constexpr auto get_echelon() const -> mat4x4;
    [[nodiscard]] constexpr auto get_transpose() const -> mat4x4;
    [[nodiscard]] constexpr auto get_inverted() const -> mat4x4;

    [[nodiscard]] constexpr auto is_invertible() const -> bool;
    [[nodiscard]] constexpr auto get_rank() const -> i32;

    constexpr auto make_echelon() -> mat4x4&;

public:
    union {
        std::array<std::array<f32, 4>, 4> el;
        std::array<v4, 4>                 col_vec;

        struct {
            v4 x_axis;
            v4 y_axis;
            v4 z_axis;
            v4 w_axis;
        };
    };
};

// IMPLEMENTATION

template<typename T>
auto operator<<(std::ostream& os, const mat4x4& v) -> std::ostream& {
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
    // const auto focal_length = math::cos(fovy / 2.0F) / math::sin(fovy / 2.0F);
    const f32 z_factor = -1.0F;

    f32 _1 = focal_length / aspect;
    f32 _2 = focal_length;
    f32 _3 = z_factor * ((z_far + z_near) / frustum_depth);
    f32 _4 = (-2.0F * z_far * z_near) / frustum_depth;
    f32 _5 = z_factor;

    return mat4x4::from_cols(
        v4::create(_1, 0.0F, 0.0F, 0.0F),
        v4::create(0.0F, _2, 0.0F, 0.0F),
        v4::create(0.0F, 0.0F, _3, _5),
        v4::create(0.0F, 0.0F, _4, 0.0F)
    );
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