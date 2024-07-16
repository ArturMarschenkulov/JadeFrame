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

    [[nodiscard]] auto get_projection() const -> mat4x4;
    [[nodiscard]] auto get_view() const -> mat4x4;
    [[nodiscard]] auto get_view_projection() const -> mat4x4;

public:
    MODE   m_mode;
    mat4x4 m_projection_matrix = mat4x4::zero();
    mat4x4 m_view_matrix = mat4x4::zero();

    v3 m_position = {};
    v3 m_forward = {}; // front
    v3 m_up = {};
    v3 m_world_up = {
    }; // TODO: Seems to be a global thing, maybe should to a more global scope?
    v3 m_right = {};

    f32 m_fov = {};
    f32 m_aspect = {};
    f32 m_near = {};
    f32 m_far = {};

    f32 m_yaw = {};
    f32 m_pitch = {};
};

class Camera0 {
    mat4x4 m_projection;
    mat4x4 m_view;

    v3 m_position;
    v3 m_forward;
    v3 m_up;
    v3 m_right;

    f32  m_FOV;
    f32  m_aspect;
    f32  m_near;
    f32  m_far;
    bool m_is_perspective;

    auto
    set_perspective(const f32 fov, const f32 aspect, const f32 t_near, const f32 t_far)
        -> void {
        m_is_perspective = true;
        m_projection = mat4x4::perspective_rh_gl(fov, aspect, t_near, t_far);
        m_FOV = fov;
        m_aspect = aspect;
        m_near = t_near;
        m_far = t_far;
    }

    auto set_orthographic(
        const f32 left,
        const f32 right,
        const f32 top,
        const f32 bottom,
        const f32 t_near,
        const f32 t_far
    ) -> void {
        m_is_perspective = false;
        m_projection =
            mat4x4::orthographic_rh_gl(left, right, top, bottom, t_near, t_far);
        m_near = t_near;
        m_far = t_far;
    }

    auto update_view() -> void {
        m_view = mat4x4::look_at_rh(m_position, m_position + m_forward, m_up);
    }
};

} // namespace JadeFrame