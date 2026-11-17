#include <gtest/gtest.h>

#include <cmath>

#include "JadeFrame/base_app.h"
#include "JadeFrame/graphics/camera.h"
#include "JadeFrame/platform/window.h"
#include "JadeFrame/platform/window_event.h"

using namespace JadeFrame;

TEST(CameraVolume, PreservesFieldOfViewAspectAndSymmetry) {
    const Camera::Volume volume =
        Camera::Volume::from_fovy_aspect(60.0F, 16.0F / 9.0F, 0.1F, 100.0F);

    EXPECT_TRUE(volume.is_symmetric());

    const auto [fovy, aspect] = volume.calc_fovy_aspect();
    EXPECT_NEAR(fovy, 60.0F, 1e-5F);
    EXPECT_NEAR(aspect, 16.0F / 9.0F, 1e-5F);
}

TEST(CameraControls, PreserveOrthographicOrientationWithoutRotationInput) {
    auto camera = Camera::orthographic(0, 800, 0, 800, -1, 1);
    const auto original = camera.m_orientation;
    InputState input;

    for (int frame = 0; frame < 3; ++frame) {
        control_camera(camera, input, 1.0 / 60.0);
    }
    EXPECT_EQ(camera.m_orientation.m_forward, original.m_forward);
    EXPECT_EQ(camera.m_orientation.m_up, original.m_up);
    EXPECT_EQ(camera.m_orientation.m_right, original.m_right);

    // Translation should still work without rebuilding the orientation.
    ASSERT_TRUE(input.process_event(
        WindowEvent::make_key_event(KEY::D, INPUT_STATE::PRESSED)
    ));
    control_camera(camera, input, 1.0);
    EXPECT_EQ(camera.m_position, original.m_right);
    EXPECT_EQ(camera.m_orientation.m_forward, original.m_forward);
    EXPECT_EQ(camera.m_orientation.m_up, original.m_up);
    EXPECT_EQ(camera.m_orientation.m_right, original.m_right);
}

namespace {

auto expect_orthonormal(const Camera::Orientation& orientation) -> void {
    const auto& forward = orientation.m_forward;
    const auto& right = orientation.m_right;
    const auto& up = orientation.m_up;
    EXPECT_NEAR(forward.length(), 1.0F, 1e-5F);
    EXPECT_NEAR(right.length(), 1.0F, 1e-5F);
    EXPECT_NEAR(up.length(), 1.0F, 1e-5F);
    EXPECT_NEAR(forward.dot(right), 0.0F, 1e-5F);
    EXPECT_NEAR(forward.dot(up), 0.0F, 1e-5F);
    EXPECT_NEAR(right.dot(up), 0.0F, 1e-5F);
    EXPECT_NEAR(forward.cross(up).dot(right), 1.0F, 1e-5F);
}

} // namespace

TEST(CameraControls, ArrowKeysRotateAroundLocalAxesForBothProjections) {
    for (const auto& initial : {
             Camera::orthographic(0, 800, 0, 800, -1, 1),
             Camera::perspective(v3::zero(), 60.0F, 1.0F, 0.1F, 100.0F)}) {
        for (const KEY key : {KEY::UP, KEY::DOWN, KEY::LEFT, KEY::RIGHT}) {
            auto camera = initial;
            const auto original = camera.m_orientation;
            InputState input;
            ASSERT_TRUE(input.process_event(
                WindowEvent::make_key_event(key, INPUT_STATE::PRESSED)
            ));

            control_camera(camera, input, 1.0);

            // One second turns by one degree, with no sudden change of basis.
            const auto& rotated = camera.m_orientation;
            EXPECT_NEAR(rotated.m_forward.dot(original.m_forward),
                        std::cos(to_radians(1.0F)), 1e-6F);
            const v3 direction = key == KEY::UP ? original.m_up
                               : key == KEY::DOWN ? -original.m_up
                               : key == KEY::LEFT ? -original.m_right
                                                  : original.m_right;
            EXPECT_NEAR(rotated.m_forward.dot(direction),
                        std::sin(to_radians(1.0F)), 1e-6F);
            if (key == KEY::UP || key == KEY::DOWN) {
                EXPECT_NEAR(rotated.m_right.dot(original.m_right), 1.0F, 1e-6F);
            } else {
                EXPECT_NEAR(rotated.m_up.dot(original.m_up), 1.0F, 1e-6F);
            }
            expect_orthonormal(rotated);
            EXPECT_EQ(camera.m_position, initial.m_position);
        }
    }
}

TEST(CameraControls, RepeatedLocalRotationStaysOrthonormalAcrossVerticalDirections) {
    for (auto camera : {
             Camera::orthographic(0, 800, 0, 800, -1, 1),
             Camera::perspective(v3::zero(), 60.0F, 1.0F, 0.1F, 100.0F)}) {
        const auto original = camera.m_orientation;
        InputState input;
        ASSERT_TRUE(input.process_event(
            WindowEvent::make_key_event(KEY::UP, INPUT_STATE::PRESSED)
        ));
        for (int frame = 0; frame < 3600; ++frame) {
            control_camera(camera, input, 0.1);
            expect_orthonormal(camera.m_orientation);
        }
        // A full turn passes through the old pitch limits and returns to the start.
        EXPECT_NEAR(camera.m_orientation.m_forward.dot(original.m_forward), 1.0F, 1e-5F);
        EXPECT_NEAR(camera.m_orientation.m_up.dot(original.m_up), 1.0F, 1e-5F);
    }
}
