#include <gtest/gtest.h>

#include "JadeFrame/graphics/camera.h"

using namespace JadeFrame;

TEST(CameraVolume, PreservesFieldOfViewAspectAndSymmetry) {
    const Camera::Volume volume =
        Camera::Volume::from_fovy_aspect(60.0F, 16.0F / 9.0F, 0.1F, 100.0F);

    EXPECT_TRUE(volume.is_symmetric());

    const auto [fovy, aspect] = volume.calc_fovy_aspect();
    EXPECT_NEAR(fovy, 60.0F, 1e-5F);
    EXPECT_NEAR(aspect, 16.0F / 9.0F, 1e-5F);
}
