#include "camera.h"
#include "JadeFrame/graphics/graphics_shared.h"
#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/math.h"

namespace JadeFrame {
// class Radian;
// class Degree;

// class Radian {
// public:
//     static auto create(const f32 radian) -> Radian { return Radian{.m_inner = radian};
//     }

//     static auto from_degree(const Degree& degree) -> Radian {
//         return Radian{.m_inner = to_radians(degree.m_inner)};
//     }

// public:
//     f32 m_inner;
// };

// class Degree {
// public:
//     static auto create(const f32 degree) -> Degree { return Degree{.m_inner = degree};
//     }

//     static auto from_radian(const Radian& radian) -> Degree {
//         return Degree{.m_inner = to_degrees(radian.m_inner)};
//     }

// public:
//     f32 m_inner;
// };
class Projection {
public:
    enum class TYPE {
        ORTHOGRAPHIC,
        PERSPECTIVE,
    };
    enum class HANDEDNESS {
        LEFT,
        RIGHT,
    };

    static auto
    from_box(const Camera::Volume& box, const TYPE type, const HANDEDNESS handedness)
        -> Projection {
        Projection proj;
        proj.m_type = type;
        proj.m_handedness = handedness;
        if (type == TYPE::ORTHOGRAPHIC) {
            if (handedness == HANDEDNESS::RIGHT) {
                proj.m_proj = mat4x4::orthographic_rh_no(
                    box.m_left,
                    box.m_right,
                    box.m_bottom,
                    box.m_top,
                    box.m_near,
                    box.m_far
                );
            } else if (handedness == HANDEDNESS::LEFT) {
                proj.m_proj = mat4x4::orthographic_lh_no(
                    box.m_left,
                    box.m_right,
                    box.m_bottom,
                    box.m_top,
                    box.m_near,
                    box.m_far
                );
            }
        } else if (type == TYPE::PERSPECTIVE) {

            if (handedness == HANDEDNESS::RIGHT) {
                proj.m_proj = mat4x4::perspective_rh_no(
                    box.m_left,
                    box.m_right,
                    box.m_bottom,
                    box.m_top,
                    box.m_near,
                    box.m_far
                );
            } else if (handedness == HANDEDNESS::LEFT) {
                auto [fovy, aspect] = get_fovy_aspect(
                    box.m_left, box.m_right, box.m_bottom, box.m_top, box.m_near
                );
                proj.m_proj =
                    mat4x4::perspective_lh_no(fovy, aspect, box.m_near, box.m_far);
            }
        }
        return proj;
    }

    // see:
    // https://lektiondestages.art.blog/2013/11/18/decompose-the-opengl-projection-matrix/
    [[nodiscard]] auto to_frustum_box() const -> Camera::Volume {
        if (m_type == TYPE::ORTHOGRAPHIC) {
            f32 near = (1 + m_proj[3][2]) / m_proj[2][2];
            f32 far = -(1 - m_proj[3][2]) / m_proj[2][2];
            f32 bottom = (1 - m_proj[3][1]) / m_proj[1][1];
            f32 top = -(1 + m_proj[3][1]) / m_proj[1][1];
            f32 left = -(1 + m_proj[3][0]) / m_proj[0][0];
            f32 right = (1 - m_proj[3][0]) / m_proj[0][0];
            return Camera::Volume{left, right, bottom, top, near, far};
        } else if (m_type == TYPE::PERSPECTIVE) {
            auto near = m_proj[3][2] / (m_proj[2][2] - 1);
            auto far = m_proj[3][2] / (m_proj[2][2] + 1);
            auto bottom = near * (m_proj[2][1] - 1) / m_proj[1][1];
            auto top = near * (m_proj[2][1] + 1) / m_proj[1][1];
            auto left = near * (m_proj[2][0] - 1) / m_proj[0][0];
            auto right = near * (m_proj[2][0] + 1) / m_proj[0][0];
            return Camera::Volume{left, right, bottom, top, near, far};
        }
        return Camera::Volume{};
    }

public:
    mat4x4     m_proj = mat4x4::zero();
    TYPE       m_type = TYPE::ORTHOGRAPHIC;
    HANDEDNESS m_handedness = HANDEDNESS::RIGHT;
};

/*
OpenGL's default "camera" is at (0, 0, 0) and looks forward into the negative
z-direction.

Vulkan's
*/

/// The coordinate system
///
/// Note:
/// The hand analogy: thumb == x-axis, index finger == y-axis, middle finger == z-axis
class CoordinateSystem {
public:
    enum class HANDED {
        LEFT,
        RIGHT,
    };

    constexpr static auto create(v3 up, HANDED handed) noexcept -> CoordinateSystem {
        assert((up == v3::Y() || up == v3::Z()) && "up must be either Y or Z");
        if (up == v3::Y() && handed == HANDED::LEFT) {
            // Unity, LightWave, ZBrush, Cinema4D, OpenGL
            return CoordinateSystem{
                .m_up = v3::Y(),
                .m_right = v3::X(),
                .m_forward = v3::NEG_Z(),
            };
        } else if (up == v3::Z() && handed == HANDED::LEFT) {
            // Unreal Engine
            return CoordinateSystem{
                .m_up = v3::Z(),
                .m_right = v3::Y(),
                .m_forward = v3::X(),
            };
        } else if (up == v3::Y() && handed == HANDED::RIGHT) {
            // Bevy, Maya, Modo, Godot, Substance Painter, Houdini, Minecraft
            return CoordinateSystem{
                .m_up = v3::Y(),
                .m_right = v3::X(),
                .m_forward = v3::NEG_Z(),
            };
        } else if (up == v3::Z() && handed == HANDED::RIGHT) {
            // Blender, 3DSMax, SketchUp, Source, Autodesk AutoCAD

            return CoordinateSystem{
                .m_up = v3::Z(),
                .m_right = v3::X(),
                .m_forward = v3::Y(),
            };
        }
        assert(false && "Invalid Coordinate System");
    }

    // Blender, 3DSMax, SketchUp, Source, Autodesk AutoCAD, CryEngine
    consteval static auto z_up_right_handed() noexcept -> CoordinateSystem {
        return CoordinateSystem::create(v3::Z(), HANDED::RIGHT);
    }

    // Unreal Engine
    consteval static auto z_up_left_handed() noexcept -> CoordinateSystem {
        return CoordinateSystem::create(v3::Z(), HANDED::LEFT);
    }

    // Bevy, Maya, Modo, Godot, Substance Painter, Houdini, Minecraft
    consteval static auto y_up_right_handed() noexcept -> CoordinateSystem {
        return CoordinateSystem::create(v3::Y(), HANDED::RIGHT);
    }

    // Unity, LightWave, ZBrush, Cinema4D, OpenGL
    consteval static auto y_up_left_handed() noexcept -> CoordinateSystem {
        return CoordinateSystem::create(v3::Y(), HANDED::LEFT);
    }

    v3 m_up;
    v3 m_right;
    v3 m_forward;
};

auto Camera::get_view_projection(const char* api) const -> mat4x4 {
    mat4x4 look_at =
        mat4x4::look_to_rh(m_position, m_orientation.m_forward, m_orientation.m_up);

    mat4x4 proj = this->calc_projection(api);

    // m_projection_matrix = proj;
    mat4x4 result = proj * look_at;
    return result;
}

auto Camera::calc_projection(const char* api) const -> mat4x4 {
    auto   ss = GRAPHICS_API::OPENGL;
    mat4x4 proj = mat4x4::identity();

    auto [fov, aspect] = m_volume.calc_fovy_aspect();
    if (m_mode == MODE::PERSPECTIVE) {
        if (api == "OpenGL") {
            proj =
                mat4x4::perspective_rh_no(fov, aspect, m_volume.m_near, m_volume.m_far);
        } else if (api == "Vulkan") {
            proj =
                mat4x4::perspective_rh_zo(fov, aspect, m_volume.m_near, m_volume.m_far);
            // proj[1][1] *= -1;
        } else {
            assert(false && "Invalid API");
        }
    } else if (m_mode == MODE::ORTHOGRAPHIC) {
        if (api == "OpenGL") {
            proj = mat4x4::orthographic_rh_no(
                m_volume.m_near,
                m_volume.m_far,
                m_volume.m_near,
                m_volume.m_far,
                m_volume.m_near,
                m_volume.m_far
            );
        } else if (api == "Vulkan") {
            proj = mat4x4::orthographic_rh_zo(
                m_volume.m_near,
                m_volume.m_far,
                m_volume.m_near,
                m_volume.m_far,
                m_volume.m_near,
                m_volume.m_far
            );
            // proj[1][1] *= -1;
        } else {
            assert(false && "Invalid API");
        }
    }
    return proj;
}

static const CoordinateSystem g_coordinate_system = CoordinateSystem::z_up_right_handed();

auto Camera::perspective(
    const v3& position,
    const f32 left,
    const f32 right,
    const f32 bottom,
    const f32 top,
    const f32 z_near,
    const f32 z_far
) -> Camera {

    Camera cam;
    cam.m_mode = MODE::PERSPECTIVE;
    cam.m_position = position;
    cam.m_orientation =
        Orientation::create(g_coordinate_system.m_forward, g_coordinate_system.m_up);
    cam.m_volume = Volume::create(left, right, bottom, top, z_near, z_far);

    return cam;
}

auto Camera::perspective(
    const v3& position,
    const f32 fov,
    const f32 aspect,
    const f32 z_near,
    const f32 z_far
) -> Camera {

    Camera cam;
    cam.m_mode = MODE::PERSPECTIVE;
    cam.m_position = position;

    cam.m_orientation =
        Orientation::create(g_coordinate_system.m_forward, g_coordinate_system.m_up);

    cam.m_volume = Volume::from_fovy_aspect(fov, aspect, z_near, z_far);
    Volume& vol = cam.m_volume;

    assert(vol.is_symmetric() && "The volume must be symmetric");

    return cam;
}

auto Camera::orthographic(
    const f32 left,
    const f32 right,
    const f32 bottom,
    const f32 top,
    const f32 near_,
    const f32 far_
) -> Camera {
    assert(left != right);
    assert(bottom != top);
    assert(near_ != far_);
    Camera camera;
    camera.m_mode = MODE::ORTHOGRAPHIC;
    camera.m_position = v3::zero();
    camera.m_orientation =
        Orientation::create(-g_coordinate_system.m_up, g_coordinate_system.m_forward);
    camera.m_volume = Volume::create(left, right, bottom, top, near_, far_);
    return camera;
}

enum class RotationOrder {
    XYZ,
    XZY,
    YXZ,
    YZX,
    ZXY,
    ZYX,
};

// auto Camera::Orientation::set_pitch_yaw(const f32 pitch, const f32 yaw) -> void {
//     m_pitch = pitch;
//     m_yaw = yaw;

//     m_forward.x = std::cos(to_radians(yaw)) * std::cos(to_radians(pitch));
//     m_forward.y = std::sin(to_radians(pitch));
//     m_forward.z = std::sin(to_radians(yaw)) * std::cos(to_radians(pitch));

//     m_right = m_forward.cross(m_world_up).normalize();
//     m_up = m_right.cross(m_forward).normalize();
// }

auto Camera::Orientation::create(const v3& forward, const v3& up) -> Orientation {
    Orientation orient;
    orient.m_forward = forward;
    orient.m_up = up;
    // TODO: This seems iffy. Check out whether the logic here is correct.
    // I assume that the global right would always (at least in most cases) be the right
    // initial right vector of the camera. But that might also not always be correct.
    orient.m_right = g_coordinate_system.m_right;

    orient.m_pitch = to_degrees(std::asin(orient.m_forward.y));
    orient.m_yaw = to_degrees(std::atan2(orient.m_forward.z, orient.m_forward.x));
    return orient;
}

/// pitch: positive pitch means looking down, negative pitch means looking up
/// yaw: positive yaw means looking to the right, negative yaw means looking to the left
/// if pitch and yaw are both 0, the camera is looking toward +x
auto Camera::Orientation::set_pitch_yaw(const f32 pitch, const f32 yaw) -> void {
    m_pitch = pitch;
    m_yaw = yaw;

    mat4x4 pitch_matrix = mat4x4::rotation_y_rh(to_radians(pitch));
    mat4x4 yaw_matrix = mat4x4::rotation_z_rh(to_radians(yaw));
    mat4x4 rotation_matrix = mat4x4::identity();
    rotation_matrix = yaw_matrix * pitch_matrix;

    v4 forward_ = rotation_matrix.x_axis;
    v4 right_ = rotation_matrix.y_axis;
    v4 up_ = rotation_matrix.z_axis;

    v3 forward = v3::create(forward_.x, forward_.y, forward_.z);
    v3 right = v3::create(right_.x, right_.y, right_.z);
    v3 up = v3::create(up_.x, up_.y, up_.z);

    m_forward = forward.normalize();
    m_right = -right.normalize();
    m_up = up.normalize();
}

[[nodiscard]] auto Camera::Orientation::get_pitch_yaw() const -> std::pair<f32, f32> {
    // return {m_pitch, m_yaw};

    // Ensure the forward vector is normalized
    v3 forward_normalized = m_forward.normalize();

    // Clamp the forward.z to the range [-1.0, 1.0] to avoid domain errors in asin
    float clamped_forward_z = std::fmax(-1.0f, std::fmin(1.0f, forward_normalized.z));

    // Compute pitch in radians
    float pitch_rad = std::asin(-clamped_forward_z);

    // Compute yaw in radians using atan2 to handle the correct quadrant
    float yaw_rad = std::atan2(forward_normalized.y, forward_normalized.x);

    // Convert radians to degrees
    float pitch = to_degrees(pitch_rad);
    float yaw = to_degrees(yaw_rad);

    return {pitch, yaw};
}

auto Camera::Volume::is_symmetric() const -> bool {
    return m_left == -m_right && m_bottom == -m_top;
}

auto Camera::Volume::create(
    const f32 left,
    const f32 right,
    const f32 bottom,
    const f32 top,
    const f32 z_near,
    const f32 z_far
) -> Volume {
    return Volume{
        .m_left = left,
        .m_right = right,
        .m_bottom = bottom,
        .m_top = top,
        .m_near = z_near,
        .m_far = z_far,
    };
}

auto Camera::Volume::from_fovy_aspect(
    const f32 fovy,
    const f32 aspect,
    const f32 z_near,
    const f32 z_far
) -> Volume {
    Volume vol;

    vol.m_near = z_near;
    vol.m_far = z_far;

    f32 tan_half_fovy = std::tan(to_radians(fovy) / 2);
    f32 half_height = z_near * tan_half_fovy;
    f32 half_width = half_height * aspect;

    vol.m_left = -half_width;
    vol.m_right = half_width;
    vol.m_bottom = -half_height;
    vol.m_top = half_height;

    return vol;
}

auto Camera::Volume::calc_fovy_aspect() const -> std::pair<f32, f32> {
    f32 width = m_right - m_left;
    f32 height = m_top - m_bottom;
    f32 aspect = width / height;
    f32 field_of_view = to_degrees(2.0F * std::atan(m_top / m_near));
    return {field_of_view, aspect};
}

} // namespace JadeFrame