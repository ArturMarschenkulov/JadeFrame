#pragma once
#include <vulkan/vulkan.h>


namespace JadeFrame {

class VulkanLogicalDevice;

class VulkanFence {
public:
	auto init(const VulkanLogicalDevice& device) -> void;
	auto deinit() -> void;
	auto wait_for_fences() -> void;
	auto reset() -> void;

	VkFence m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
};

class VulkanSemaphore {
public:
	auto init(const VulkanLogicalDevice& device) -> void;
	auto deinit() -> void;

	VkSemaphore m_handle = VK_NULL_HANDLE;
	const VulkanLogicalDevice* m_device = nullptr;
};

}