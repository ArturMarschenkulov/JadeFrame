#include "pch.h"
#include "vulkan_shader.h"
#include "vulkan_logical_device.h"

namespace JadeFrame {


Vulkan_Shader::Vulkan_Shader(const VulkanLogicalDevice& device, const DESC& desc) {
	m_device = &device;
	m_pipeline.init(
		device,
		device.m_swapchain.m_extent,
		device.m_descriptor_set_layout_0,
		device.m_swapchain.m_render_pass,
		desc.code,
		desc.vertex_format
	);
}

}
