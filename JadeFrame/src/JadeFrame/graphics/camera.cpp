#include "JadeFrame/math/mat_4.h"
#include "pch.h"
#include "camera.h"

namespace JadeFrame {

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

auto Camera::get_view_projection() const -> mat4x4 {
    mat4x4 look_at =
        mat4x4::look_to_rh(m_position, m_orientation.m_forward, m_orientation.m_up);
    mat4x4 proj = m_projection_matrix;
    mat4x4 result = proj * look_at;
    return result;
}

static const CoordinateSystem g_coordinate_system = CoordinateSystem::z_up_right_handed();

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
    Orientation& orient = cam.m_orientation;
    orient.m_forward = g_coordinate_system.m_forward;
    orient.m_world_up = g_coordinate_system.m_up;
    orient.m_up = orient.m_world_up;

    orient.m_pitch = to_degrees(std::asin(orient.m_forward.y));
    orient.m_yaw = to_degrees(std::atan2(orient.m_forward.z, orient.m_forward.x));
    cam.m_fov = fov;
    cam.m_aspect = aspect;
    cam.m_near = z_near;
    cam.m_far = z_far;
    cam.m_projection_matrix =
        mat4x4::perspective_rh_no(cam.m_fov, cam.m_aspect, cam.m_near, cam.m_far);

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
    const CoordinateSystem g_coordinate_system = CoordinateSystem::z_up_right_handed();

    Camera camera;
    camera.m_mode = MODE::ORTHOGRAPHIC;

    camera.m_position = v3::zero();
    Orientation& orient = camera.m_orientation;
    orient.m_world_up = g_coordinate_system.m_up;
    orient.m_forward = -g_coordinate_system.m_up;
    orient.m_up = g_coordinate_system.m_forward;
    orient.m_right = g_coordinate_system.m_right;

    camera.m_projection_matrix =
        mat4x4::orthographic_rh_no(left, right, bottom, top, near_, far_);

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
} // namespace JadeFrame