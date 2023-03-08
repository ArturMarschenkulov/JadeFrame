#include "pch.h"
#include "vulkan_shader.h"
#include "vulkan_logical_device.h"

namespace JadeFrame {


Vulkan_Shader::Vulkan_Shader(const vulkan::LogicalDevice& device, const Desc& desc) {
    m_device = &device;
    Logger::info("Creating Vulkan shader");
    m_pipeline.init(
        device, device.m_swapchain.m_extent, device.m_swapchain.m_render_pass,
        desc.code, desc.vertex_format);
    Logger::info("Created Vulkan shader");
    m_reflected_code = m_pipeline.m_reflected_code;
}

} // namespace JadeFrame
