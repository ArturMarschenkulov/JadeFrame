#include "pch.h"
#include "camera.h"

namespace JadeFrame {

auto Camera::perspective_mode(const v3& position, const f32 fov, const f32 aspect, const f32 zNear, const f32 zFar)
    -> void {
    m_mode = MODE::PERSPECTIVE;
    // probably move to constructor
    m_position = position;
    m_forward = {0.0f, 1.0f, 0.0f};
    m_world_up = {0.0f, 0.0f, 1.0f};

    m_projection_matrix = Matrix4x4::perspective_projection_matrix(fov, aspect, zNear, zFar);

    m_fov = fov;
    m_aspect = aspect;
    m_near = zNear;
    m_far = zFar;
}

auto Camera::othographic_mode(f32 left, f32 right, f32 bottom, f32 top, f32 near_, f32 far_) -> void {
    assert(left != right);
    assert(bottom != top);
    assert(near_ != far_);


    m_mode = MODE::ORTHOGRAPHIC;
    m_projection_matrix = Matrix4x4::orthogonal_projection_matrix(left, right, bottom, top, near_, far_);

    m_position = {0.0f, 0.0f, 0.0f};

    // m_forward = { 1.0f, 0.0f, 0.0f };
    // m_up = { 0.0f, 1.0f, 0.0f };
    m_forward = {0.0f, 0.0f, -1.0f};
    m_up = {0.0f, 1.0f, 0.0f};
}

auto Camera::get_projection_matrix() const -> Matrix4x4 { return m_projection_matrix; }
auto Camera::get_view_matrix() const -> Matrix4x4 {
    return Matrix4x4::look_at_matrix(m_position, m_position + m_forward, m_up);
}

auto Camera::get_view_projection_matrix() const -> Matrix4x4 {
    return this->get_view_matrix() * this->get_projection_matrix();
}
} // namespace JadeFrame