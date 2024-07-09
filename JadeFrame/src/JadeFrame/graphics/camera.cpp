#include "JadeFrame/graphics/graphics_shared.h"
#include "pch.h"
#include "camera.h"

namespace JadeFrame {

/*
OpenGL's default "camera" is at (0, 0, 0) and looks forward into the negative
z-direction.

Vulkan's
*/

class CoordinateSystem {
public:
    enum class HANDED {
        LEFT,
        RIGHT,
    };

    constexpr static auto create(v3 up, HANDED handed) -> CoordinateSystem {
        assert(up == v3::Y() || up == v3::Z() && "up must be either Y or Z");
        if (up == v3::Y() && handed == HANDED::LEFT) {
            // Unity, LightWave, ZBrush, Cinema4D, OpenGL
            return CoordinateSystem{
                .m_up = v3::Y(),
                .m_right = v3::X(),
                .m_forward = v3::NEG_Z(),
            };
        } else if (up == v3::Y() && handed == HANDED::RIGHT) {
            // Bevy, Maya, Modo, Godot, Substance Painter, Houdini, Minecraft
            return CoordinateSystem{
                .m_up = v3::Y(),
                .m_right = v3::X(),
                .m_forward = v3::Z(),
            };
        } else if (up == v3::Z() && handed == HANDED::LEFT) {
            // Unreal Engine
            return CoordinateSystem{
                .m_up = v3::Z(),
                .m_right = v3::X(),
                .m_forward = v3::NEG_Y(),
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
    consteval static auto z_up_right_handed() -> CoordinateSystem {
        return CoordinateSystem::create(v3::Z(), HANDED::RIGHT);
    }

    // Unreal Engine
    consteval static auto z_up_left_handed() -> CoordinateSystem {
        return CoordinateSystem::create(v3::Z(), HANDED::LEFT);
    }

    // Bevy, Maya, Modo, Godot, Substance Painter, Houdini, Minecraft
    consteval static auto y_up_right_handed() -> CoordinateSystem {
        return CoordinateSystem::create(v3::Y(), HANDED::RIGHT);
    }

    // Unity, LightWave, ZBrush, Cinema4D
    consteval static auto y_up_left_handed() -> CoordinateSystem {
        return CoordinateSystem::create(v3::Y(), HANDED::LEFT);
    }

    v3 m_up;
    v3 m_right;
    v3 m_forward;
};

struct PerspectiveProjection {
    f32 m_fov;
    f32 m_aspect;
    f32 m_near;
    f32 m_far;
};

struct OrthographicProjection {
    f32 m_near;
    f32 m_far;
};

auto Camera::get_projection() const -> mat4x4 { return m_projection_matrix; }

auto Camera::get_view() const -> mat4x4 {
    return mat4x4::look_at_rh(m_position, m_position + m_forward, m_up);
}

auto Camera::get_view_projection() const -> mat4x4 {
    return this->get_view() * this->get_projection();
}

auto Camera::perspective(
    const v3& position,
    const f32 fov,
    const f32 aspect,
    const f32 near,
    const f32 far
) -> Camera {
    const auto coordinate_system = CoordinateSystem::z_up_right_handed();

    Camera camera;
    camera.m_forward = coordinate_system.m_forward;
    camera.m_world_up = coordinate_system.m_up;

    camera.m_mode = MODE::PERSPECTIVE;
    camera.m_position = position;

    camera.m_projection_matrix = mat4x4::perspective_rh(fov, aspect, near, far);

    camera.m_fov = fov;
    camera.m_aspect = aspect;
    camera.m_near = near;
    camera.m_far = far;
    return camera;
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
    const auto coordinate_system = CoordinateSystem::y_up_left_handed();

    Camera camera;
    camera.m_forward = coordinate_system.m_forward;
    camera.m_up = coordinate_system.m_up;

    camera.m_mode = MODE::ORTHOGRAPHIC;
    camera.m_projection_matrix =
        mat4x4::orthographic_rh(left, right, bottom, top, near_, far_);
    camera.m_position = {0.0F, 0.0F, 0.0F};

    return camera;
}
} // namespace JadeFrame