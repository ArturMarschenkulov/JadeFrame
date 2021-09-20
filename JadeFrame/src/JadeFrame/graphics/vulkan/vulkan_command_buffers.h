#pragma once
#include <vulkan/vulkan.h>
#include <functional>


namespace JadeFrame {

class VulkanLogicalDevice;
class VulkanCommandPool;
class VulkanSwapchain;

class VulkanCommandBuffers {
public:
	auto init(
		const VulkanLogicalDevice& device,
		const VulkanCommandPool& command_pool,
		const size_t amount = 1
	) -> void;
	auto deinit() -> void;

	auto record(size_t index, std::function<void()> func) -> void;


	//auto bind_pipeline(size_t index) -> void;

public:
	std::vector<VkCommandBuffer> m_handles;
	const VulkanLogicalDevice* m_device = nullptr;
	const VulkanCommandPool* m_command_pool = nullptr;
};
}