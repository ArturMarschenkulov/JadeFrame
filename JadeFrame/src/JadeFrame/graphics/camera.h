#pragma once
#include "JadeFrame/math/mat_4.h"
#include "JadeFrame/math/vec.h"

namespace JadeFrame {

class Camera {
public:
    enum class MODE : u8 {
        ORTHOGRAPHIC,
        PERSPECTIVE,
    };

    struct Volume {
        f32 m_left = {};
        f32 m_right = {};
        f32 m_bottom = {};
        f32 m_top = {};
        f32 m_near = {};
        f32 m_far = {};

        [[nodiscard]] auto is_symmetric() const -> bool;

        static auto create(
            const f32 left,
            const f32 right,
            const f32 bottom,
            const f32 top,
            const f32 z_near,
            const f32 z_far
        ) -> Volume;
        static auto from_fovy_aspect(
            const f32 fovy,
            const f32 aspect,
            const f32 z_near,
            const f32 z_far
        ) -> Volume;

        [[nodiscard]] auto calc_fovy_aspect() const -> std::pair<f32, f32>;
    };

    struct Orientation {
        v3 m_forward;
        v3 m_right;
        v3 m_up;

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

        static auto create(const v3& forward, const v3& up) -> Orientation;

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

    [[nodiscard]] auto get_view_projection(const char* api) const -> mat4x4;

    auto calc_projection(const char* api) const -> mat4x4;

public:
    MODE        m_mode;
    v3          m_position;
    Orientation m_orientation;
    Volume      m_volume;
};

} // namespace JadeFrame