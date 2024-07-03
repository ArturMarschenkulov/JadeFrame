#include "pch.h"
#include "sync_object.h"
#include "logical_device.h"
#include "context.h"

#include "JadeFrame/utils/assert.h"

namespace JadeFrame {
namespace vulkan {

Fence::Fence(Fence&& other) noexcept
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_device(std::exchange(other.m_device, nullptr)) {}

auto Fence::operator=(Fence&& other) noexcept -> Fence& {
    if (this != &other) {
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_device = std::exchange(other.m_device, nullptr);
    }
    return *this;
}

Fence::Fence(LogicalDevice& device, bool signaled)
    : m_device(&device) {

    const VkFenceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : (VkFenceCreateFlags)0,
    };

    VkResult result =
        vkCreateFence(device.m_handle, &info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

Fence::~Fence() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyFence(m_device->m_handle, m_handle, nullptr);
    }
}

auto Fence::wait_for_fences() -> void {
    VkResult result =
        vkWaitForFences(m_device->m_handle, 1, &m_handle, VK_TRUE, UINT64_MAX);
    JF_ASSERT(result == VK_SUCCESS, "");
}

auto Fence::reset() -> void {
    VkResult result = vkResetFences(m_device->m_handle, 1, &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

auto Fence::is_signaled() const -> bool {
    VkResult result = vkGetFenceStatus(m_device->m_handle, m_handle);
    switch (result) {
        case VK_SUCCESS: return true;
        case VK_NOT_READY: return false;
        default: JF_ASSERT(false, ""); return false;
    }
}

Semaphore::Semaphore(Semaphore&& other) noexcept
    : m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
    , m_device(std::exchange(other.m_device, nullptr)) {}

auto Semaphore::operator=(Semaphore&& other) noexcept -> Semaphore& {
    if (this != &other) {
        m_handle = std::exchange(other.m_handle, VK_NULL_HANDLE);
        m_device = std::exchange(other.m_device, nullptr);
    }
    return *this;
}

Semaphore::~Semaphore() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device->m_handle, m_handle, nullptr);
    }
}

Semaphore::Semaphore(LogicalDevice& device)
    : m_device(&device) {

    const VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkResult result =
        vkCreateSemaphore(device.m_handle, &info, Instance::allocator(), &m_handle);
    JF_ASSERT(result == VK_SUCCESS, "");
}

} // namespace vulkan
} // namespace JadeFrame
