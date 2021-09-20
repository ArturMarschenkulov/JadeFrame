#pragma once
#include <vulkan/vulkan.h>

namespace JadeFrame {
class VulkanQueue {
public:
	auto submit(const VkSubmitInfo& submit_info) const -> void;
	auto wait_idle() const -> void;
	auto present(VkPresentInfoKHR info, VkResult& result) const -> void;
public:
	VkQueue m_handle = VK_NULL_HANDLE;
};
}