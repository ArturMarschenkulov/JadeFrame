#pragma once

#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/vec.h"

namespace JadeFrame {
class OrthographicProjection {
public:
    f32 m_near = 0;
    f32 m_far = 1000.0F;
};

class PerspectiveProjection {};

/// Corresponds to the frustum of the perspective projection or the view volume of the
/// orthographic projection.
struct FrustumBox {
    f32 m_left;
    f32 m_right;
    f32 m_bottom;
    f32 m_top;
    f32 m_near;
    f32 m_far;

    [[nodiscard]] auto calc_fovy_aspect() const -> std::pair<f32, f32> {
        // f32 fovy = 2.0F * std::atan(m_top / m_near);
        // f32 aspect = (m_right - m_left) / (m_top - m_bottom);
        // return {fovy, aspect};
        const f32 width = m_right - m_left;
        const f32 height = m_top - m_bottom;
        const f32 aspect = width / height;
        const f32 field_of_view = 2.0F * std::atan(m_top / 1.0F);
        return {field_of_view, aspect};
    }
};

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
    from_box(const FrustumBox& box, const TYPE type, const HANDEDNESS handedness)
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
    [[nodiscard]] auto to_frustum_box() const -> FrustumBox {
        if (m_type == TYPE::ORTHOGRAPHIC) {
            f32 near = (1 + m_proj[3][2]) / m_proj[2][2];
            f32 far = -(1 - m_proj[3][2]) / m_proj[2][2];
            f32 bottom = (1 - m_proj[3][1]) / m_proj[1][1];
            f32 top = -(1 + m_proj[3][1]) / m_proj[1][1];
            f32 left = -(1 + m_proj[3][0]) / m_proj[0][0];
            f32 right = (1 - m_proj[3][0]) / m_proj[0][0];
            return FrustumBox{left, right, bottom, top, near, far};
        } else if (m_type == TYPE::PERSPECTIVE) {
            auto near = m_proj[3][2] / (m_proj[2][2] - 1);
            auto far = m_proj[3][2] / (m_proj[2][2] + 1);
            auto bottom = near * (m_proj[2][1] - 1) / m_proj[1][1];
            auto top = near * (m_proj[2][1] + 1) / m_proj[1][1];
            auto left = near * (m_proj[2][0] - 1) / m_proj[0][0];
            auto right = near * (m_proj[2][0] + 1) / m_proj[0][0];
            return FrustumBox{left, right, bottom, top, near, far};
        }
        return FrustumBox{};
    }

public:
    mat4x4     m_proj = mat4x4::zero();
    TYPE       m_type = TYPE::ORTHOGRAPHIC;
    HANDEDNESS m_handedness = HANDEDNESS::RIGHT;
};

class Camera {
public:
    enum class MODE {
        ORTHOGRAPHIC,
        PERSPECTIVE,
    };

    struct Orientation {
        v3 m_forward;
        v3 m_right;
        v3 m_up;
        // TODO: Seems to be a global thing, maybe should to a more global scope?
        v3 m_world_up;

        // NOTE: pitch == rotation around x, the y of the forward vector
        // NOTE: yaw == rotation around y, the x of the forward vector
        // NOTE: roll == rotation around z, the z of the forward vector

        // TODO(artur): The goal is to completely remove them from the camera class
        // and only store the vectors, if they are needed they would to calculated on
        // the fly based on the vectors. However, I need to test whether this is the
        // best approach. For now, I will keep them here.
        f32 m_yaw = {};
        f32 m_pitch = {};
        f32 m_roll = {};

        auto               set_pitch_yaw(const f32 pitch, const f32 yaw) -> void;
        [[nodiscard]] auto get_pitch_yaw() const -> std::pair<f32, f32>;
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

public:
    mat4x4      m_view_matrix = mat4x4::zero();
    v3          m_position;
    Orientation m_orientation;

    MODE   m_mode;
    mat4x4 m_projection_matrix = mat4x4::zero();
    f32    m_fov = {};
    f32    m_aspect = {};
    f32    m_near = {};
    f32    m_far = {};
};

} // namespace JadeFrame