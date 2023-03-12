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
    auto perspective_mode(const v3& position, const f32 fov, const f32 aspect, const f32 zNear, const f32 zFar) -> void;
    auto
    othographic_mode(const f32 left, const f32 right, const f32 buttom, const f32 top, const f32 near_, const f32 far_)
        -> void;

    auto perspective_mode(const v3& position, const f32 fov, const f32 aspect, const v2f32 nf) -> void;
    auto othographic_mode(const v2f32 lr, const v2f32 bt, const v2f32 nf) -> void;
    auto get_projection_matrix() const -> Matrix4x4;
    auto get_view_matrix() const -> Matrix4x4;
    auto get_view_projection_matrix() const -> Matrix4x4;
    auto control() -> void;

    // private:
public:
    MODE      m_mode;
    Matrix4x4 m_projection_matrix = Matrix4x4::zero();
    Matrix4x4 m_view_matrix = Matrix4x4::zero();

    v3 m_position = {};
    v3 m_forward = {}; // front
    v3 m_up = {};
    v3 m_world_up = {}; // TODO: Seems to be a global thing, maybe should to a more global scope?
    v3 m_right = {};

    f32 m_fov = {};
    f32 m_aspect = {};
    f32 m_near = {};
    f32 m_far = {};

    f32 m_yaw = 0;
    f32 m_pitch = {};
};

class Camera0 {
    Matrix4x4 m_projection;
    Matrix4x4 m_view;

    v3 m_position;
    v3 m_forward;
    v3 m_up;
    v3 m_right;

    f32  m_FOV;
    f32  m_aspect;
    f32  m_near;
    f32  m_far;
    bool m_is_perspective;

    auto set_perspective(const f32 fov, const f32 aspect, const f32 t_near, const f32 t_far) -> void {
        m_is_perspective = true;
        m_projection = Matrix4x4::perspective_projection(fov, aspect, t_near, t_far);
        m_FOV = fov;
        m_aspect = aspect;
        m_near = t_near;
        m_far = t_far;
    }
    auto set_orthographic(
        const f32 left, const f32 right, const f32 top, const f32 bottom, const f32 t_near, const f32 t_far) -> void {
        m_is_perspective = false;
        m_projection = Matrix4x4::orthogonal_projection(left, right, top, bottom, t_near, t_far);
        m_near = t_near;
        m_far = t_far;
    }
    auto update_view() -> void { m_view = Matrix4x4::look_at(m_position, m_position + m_forward, m_up); }
};

} // namespace JadeFrame