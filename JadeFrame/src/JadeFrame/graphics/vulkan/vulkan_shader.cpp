#include "pch.h"
#include "vulkan_shader.h"
#include "vulkan_logical_device.h"

namespace JadeFrame {

Vulkan_Shader::Vulkan_Shader(const VulkanLogicalDevice& device, const GLSLCode& code) {
	m_device = &device;
	m_pipeline.init(
		*m_device, 
		m_device->m_swapchain, 
		m_device->m_descriptor_set_layout,
		m_device->m_render_pass, 
		code
	);

}

}
