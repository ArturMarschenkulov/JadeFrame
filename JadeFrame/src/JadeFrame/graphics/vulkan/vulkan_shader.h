#pragma once
#include "../graphics_shared.h"
#include "../shader_loader.h"
#include "vulkan_pipeline.h"

namespace JadeFrame {
namespace vulkan {
class LogicalDevice;
}
class Vulkan_Shader : public IShader {
    // private:
    //	Vulkan_Shader(const vulkan::LogicalDevice& device, const GLSLCode& code);
public:
    Vulkan_Shader(const vulkan::LogicalDevice& device, const DESC& desc);

    Vulkan_Shader() = default;
    Vulkan_Shader(Vulkan_Shader&&) noexcept = default;

    Vulkan_Shader(const Vulkan_Shader&) = delete;
    auto operator=(const Vulkan_Shader&) -> Vulkan_Shader& = delete;
    auto operator=(Vulkan_Shader&&) -> Vulkan_Shader& = delete;

public:
    vulkan::Pipeline             m_pipeline;
    const vulkan::LogicalDevice* m_device;
};
} // namespace JadeFrame