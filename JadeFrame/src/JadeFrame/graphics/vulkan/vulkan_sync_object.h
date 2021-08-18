#pragma once
#include <vulkan/vulkan.h>


namespace JadeFrame {

class VulkanFence {
public:
	auto init(VkDevice m_device) -> void;
	auto deinit() -> void;

	VkFence m_handle = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
};

class VulkanSemaphore {
public:
	auto init(VkDevice m_device) -> void;
	auto deinit() -> void;

	VkSemaphore m_handle = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
};

}