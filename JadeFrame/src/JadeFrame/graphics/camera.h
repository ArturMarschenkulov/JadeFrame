#pragma once

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/vec.h"

namespace JadeFrame {

class Camera {
public:
    enum class MODE {
        ORTHOGRAPHIC,
        PERSPECTIVE,
    };

public:
    static auto orthographic(
        const f32 left,
        const f32 right,
        const f32 bottom,
        const f32 top,
        const f32 z_near,
        const f32 z_far
    ) -> Camera;

    static auto perspective(
        const v3& position,
        const f32 fov,
        const f32 aspect,
        const f32 z_near,
        const f32 z_far
    ) -> Camera;

    [[nodiscard]] auto get_view_projection() const -> mat4x4;

    auto set_pitch_yaw(const f32 pitch, const f32 yaw) -> void;

public:
    MODE   m_mode;
    mat4x4 m_projection_matrix = mat4x4::zero();
    mat4x4 m_view_matrix = mat4x4::zero();

    v3 m_position;
    v3 m_forward;
    v3 m_up;

    // TODO: Seems to be a global thing, maybe should to a more global scope?
    v3 m_world_up;

    v3 m_right;

    f32 m_fov = {};
    f32 m_aspect = {};
    f32 m_near = {};
    f32 m_far = {};

    f32 m_yaw = {};
    f32 m_pitch = {};
    f32 m_roll = {};
};

} // namespace JadeFrame