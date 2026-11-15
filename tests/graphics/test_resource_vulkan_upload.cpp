// These Vulkan entry points replace the driver only in this executable. Exercise
// upload recording and rollback even where no device can present to a test window.
#include <gtest/gtest.h>

#include "JadeFrame/graphics/vulkan/command_buffer.h"
#include "JadeFrame/graphics/vulkan/logical_device.h"

#include <string>
#include <vector>

namespace {
enum class Failure {
    NONE,
    ALLOCATION,
    BEGIN,
    END,
    SUBMIT,
    WAIT
};

struct Driver {
    Failure                  failure = Failure::NONE;
    bool                     allocated = false;
    bool                     recording = false;
    bool                     submitted = false;
    std::vector<std::string> calls;
} driver;

int native_command_buffer;

auto command_handle() -> VkCommandBuffer {
    return reinterpret_cast<VkCommandBuffer>(&native_command_buffer);
}
} // namespace

extern "C" {
VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(
    VkDevice,
    const VkCommandBufferAllocateInfo* info,
    VkCommandBuffer*                   handles
) {
    driver.calls.emplace_back("allocate");
    EXPECT_EQ(info->commandBufferCount, 1U);
    EXPECT_FALSE(driver.allocated);
    if (driver.failure == Failure::ALLOCATION) { return VK_ERROR_OUT_OF_HOST_MEMORY; }
    handles[0] = command_handle();
    driver.allocated = true;
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(
    VkDevice,
    VkCommandPool,
    uint32_t               count,
    const VkCommandBuffer* handles
) {
    driver.calls.emplace_back("free");
    EXPECT_EQ(count, 1U);
    EXPECT_EQ(handles[0], command_handle());
    EXPECT_TRUE(driver.allocated) << "command buffer freed twice";
    driver.allocated = false;
}

VKAPI_ATTR VkResult VKAPI_CALL
vkBeginCommandBuffer(VkCommandBuffer handle, const VkCommandBufferBeginInfo* info) {
    driver.calls.emplace_back("begin");
    EXPECT_EQ(handle, command_handle());
    EXPECT_EQ(info->flags, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    EXPECT_TRUE(driver.allocated);
    if (driver.failure == Failure::BEGIN) { return VK_ERROR_OUT_OF_HOST_MEMORY; }
    driver.recording = true;
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(VkCommandBuffer handle) {
    driver.calls.emplace_back("end");
    EXPECT_EQ(handle, command_handle());
    EXPECT_TRUE(driver.recording);
    driver.recording = false;
    return driver.failure == Failure::END ? VK_ERROR_OUT_OF_HOST_MEMORY : VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
vkQueueSubmit(VkQueue, uint32_t count, const VkSubmitInfo* info, VkFence) {
    driver.calls.emplace_back("submit");
    EXPECT_FALSE(driver.recording);
    EXPECT_EQ(count, 1U);
    EXPECT_EQ(info->commandBufferCount, 1U);
    EXPECT_EQ(info->pCommandBuffers[0], command_handle());
    if (driver.failure == Failure::SUBMIT) { return VK_ERROR_OUT_OF_HOST_MEMORY; }
    driver.submitted = true;
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueWaitIdle(VkQueue) {
    driver.calls.emplace_back("wait");
    EXPECT_TRUE(driver.submitted);
    return driver.failure == Failure::WAIT ? VK_ERROR_DEVICE_LOST : VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(
    VkCommandBuffer handle,
    VkBuffer,
    VkBuffer,
    uint32_t            count,
    const VkBufferCopy* region
) {
    driver.calls.emplace_back("copy buffer");
    EXPECT_EQ(handle, command_handle());
    EXPECT_TRUE(driver.recording);
    EXPECT_EQ(count, 1U);
    EXPECT_EQ(region->size, 16U);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage(
    VkCommandBuffer handle,
    VkBuffer,
    VkImage,
    VkImageLayout            layout,
    uint32_t                 count,
    const VkBufferImageCopy* region
) {
    driver.calls.emplace_back("copy image");
    EXPECT_EQ(handle, command_handle());
    EXPECT_TRUE(driver.recording);
    EXPECT_EQ(layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    EXPECT_EQ(count, 1U);
    EXPECT_EQ(region->imageExtent.width, 2U);
    EXPECT_EQ(region->imageExtent.height, 2U);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(
    VkCommandBuffer handle,
    VkPipelineStageFlags,
    VkPipelineStageFlags,
    VkDependencyFlags,
    uint32_t,
    const VkMemoryBarrier*,
    uint32_t,
    const VkBufferMemoryBarrier*,
    uint32_t                    count,
    const VkImageMemoryBarrier* barrier
) {
    driver.calls.emplace_back("transition");
    EXPECT_EQ(handle, command_handle());
    EXPECT_TRUE(driver.recording);
    EXPECT_EQ(count, 1U);
    EXPECT_EQ(barrier->oldLayout, VK_IMAGE_LAYOUT_UNDEFINED);
    EXPECT_EQ(barrier->newLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}
}

namespace {
using namespace JadeFrame;
enum class Upload {
    BUFFER,
    IMAGE,
    TRANSITION
};

class VulkanUpload : public testing::TestWithParam<Upload> {
protected:
    auto SetUp() -> void override { driver = {}; }

    auto TearDown() -> void override { EXPECT_FALSE(driver.allocated); }

    auto upload() -> bool {
        vulkan::LogicalDevice device;
        vulkan::CommandPool   pool;
        pool.m_device = &device;
        // Zero-size buffers allocate no native memory. The intercepted driver only
        // needs their bounds, so no VMA allocator or actual GPU resources are needed.
        vulkan::Buffer src(device, vulkan::Buffer::STAGING, nullptr, 0);
        vulkan::Buffer dst(device, vulkan::Buffer::STAGING, nullptr, 0);
        src.m_size = dst.m_size = 16;
        vulkan::Image image;
        switch (GetParam()) {
            case Upload::BUFFER: return pool.copy_buffer(src, dst, 16);
            case Upload::IMAGE:
                return pool.copy_buffer_to_image(src, image, v2u32::create(2, 2));
            case Upload::TRANSITION:
                return pool.transition_layout(
                    image,
                    VK_FORMAT_R8G8B8A8_SRGB,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                );
        }
        return false;
    }

    auto operation() -> std::string {
        switch (GetParam()) {
            case Upload::BUFFER: return "copy buffer";
            case Upload::IMAGE: return "copy image";
            case Upload::TRANSITION: return "transition";
        }
        return {};
    }
};

TEST_P(VulkanUpload, RecordsSubmitsAndFreesExactlyOnce) {
    // Buffer and image copy methods assert JadeFrame's RECORDING state in Debug,
    // independently of the intercepted driver's native recording state.
    ASSERT_TRUE(upload());
    EXPECT_EQ(
        driver.calls,
        (std::vector<std::string>{
            "allocate",
            "begin",
            operation(),
            "end",
            "submit",
            "wait",
            "free",
        })
    );
}

TEST_P(VulkanUpload, EachFailureReleasesTheCommandBufferAndStopsSubsequentWork) {
    for (Failure failure :
         {Failure::ALLOCATION,
          Failure::BEGIN,
          Failure::END,
          Failure::SUBMIT,
          Failure::WAIT}) {
        SCOPED_TRACE(static_cast<int>(failure));
        driver = {};
        driver.failure = failure;
        EXPECT_FALSE(upload());
        std::vector<std::string> expected{"allocate"};
        if (failure != Failure::ALLOCATION) {
            expected.emplace_back("begin");
            if (failure != Failure::BEGIN) {
                expected.push_back(operation());
                expected.emplace_back("end");
                if (failure != Failure::END) {
                    expected.emplace_back("submit");
                    if (failure != Failure::SUBMIT) { expected.emplace_back("wait"); }
                }
            }
            expected.emplace_back("free");
        }
        EXPECT_EQ(driver.calls, expected);
        EXPECT_FALSE(driver.allocated);
    }
    driver = {};
    EXPECT_TRUE(upload()) << "failed uploads must not break subsequent uploads";
}

INSTANTIATE_TEST_SUITE_P(
    Operations,
    VulkanUpload,
    testing::Values(Upload::BUFFER, Upload::IMAGE, Upload::TRANSITION)
);
} // namespace
