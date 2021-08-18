#include "pch.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_logical_device.h"
#include "vulkan_swapchain.h"

#include "JadeFrame/defines.h"

namespace JadeFrame {

auto VulkanDescriptorPool::init(const VulkanLogicalDevice& device, const VulkanSwapchain& swapchain) -> void {
	VkResult result;
	const VkDescriptorPoolSize pool_size = {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = static_cast<u32>(swapchain.m_images.size()),
	};

	const VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.maxSets = static_cast<u32>(swapchain.m_images.size()),
		.poolSizeCount = 1,
		.pPoolSizes = &pool_size,
	};

	result = vkCreateDescriptorPool(device.m_handle, &pool_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();
}

}