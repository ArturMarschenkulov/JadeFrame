#include "pch.h"
#include "vulkan_shader.h"
#include "vulkan_logical_device.h"

namespace JadeFrame {

//Vulkan_Shader::Vulkan_Shader(const VulkanLogicalDevice& device, const GLSLCode& code) {
//	m_device = &device;
//	m_pipeline.init(
//		device, 
//		device.m_swapchain.m_extent, 
//		device.m_descriptor_set_layout,
//		device.m_render_pass, 
//		code
//	);
//
//}

Vulkan_Shader::Vulkan_Shader(const VulkanLogicalDevice& device, const DESC& desc) {
	m_device = &device;
	m_pipeline.init(
		device,
		device.m_swapchain.m_extent,
		device.m_descriptor_set_layout,
		device.m_render_pass,
		desc.code,
		desc.buffer_layout
	);
}

}
