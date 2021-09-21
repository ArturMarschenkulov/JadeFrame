#pragma once 
#include "../shared.h"
#include "../opengl/opengl_shader_loader.h"
#include "vulkan_pipeline.h"

namespace JadeFrame {
class VulkanLogicalDevice;
class Vulkan_Shader : public IShader {
//private:
//	Vulkan_Shader(const VulkanLogicalDevice& device, const GLSLCode& code);
public:
	Vulkan_Shader(const VulkanLogicalDevice& device, const DESC& desc);

	Vulkan_Shader() = default;
	Vulkan_Shader(Vulkan_Shader&&) noexcept = default;

	Vulkan_Shader(const Vulkan_Shader&) = delete;
	auto operator=(const Vulkan_Shader&)->Vulkan_Shader & = delete;
	auto operator=(Vulkan_Shader&&)->Vulkan_Shader & = delete;
public:
	VulkanPipeline m_pipeline;
	const VulkanLogicalDevice* m_device;

};
}