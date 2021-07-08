#pragma once
#include <vulkan/vulkan.h>

#include <vector>
namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanDescriptorSetLayout {

public:
	auto init(const VulkanLogicalDevice& device) -> void;
	auto deinit() -> void;

	//auto add_binding() -> void;
public:
	const VulkanLogicalDevice* m_device = nullptr;
	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;
	//std::vector<VkDescriptorSetLayoutBinding> m_bindings;
};

}