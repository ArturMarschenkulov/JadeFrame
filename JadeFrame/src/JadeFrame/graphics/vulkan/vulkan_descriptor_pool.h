#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_descriptor_set.h"
#include "JadeFrame/defines.h"

namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanSwapchain;

class VulkanDescriptorPool {
public:
	auto init(const VulkanLogicalDevice& device, const VulkanSwapchain& swapchain) -> void;

	auto allocate_descriptor_sets(const VulkanDescriptorSetLayout& descriptor_set_layout, u32 image_amount) -> std::vector<VulkanDescriptorSet>;

public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkDescriptorPool m_handle = VK_NULL_HANDLE;
};

}