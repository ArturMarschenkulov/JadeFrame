#pragma once
#include "../graphics_shared.h"
#include "../shader_loader.h"
#include "vulkan_pipeline.h"
#include <tuple>

namespace JadeFrame {
namespace vulkan {
class LogicalDevice;
class Buffer;
} // namespace vulkan
class Vulkan_Renderer;

class Vulkan_Shader : public IShader {
public:
    Vulkan_Shader() = default;
    ~Vulkan_Shader() = default;
    Vulkan_Shader(const Vulkan_Shader&) = delete;
    auto operator=(const Vulkan_Shader&) -> Vulkan_Shader& = delete;
    Vulkan_Shader(Vulkan_Shader&&) noexcept = default;
    auto operator=(Vulkan_Shader&&) -> Vulkan_Shader& = default;

    Vulkan_Shader(
        const vulkan::LogicalDevice& device,
        const Vulkan_Renderer&       renderer,
        const Desc&                  desc
    );

    auto bind_buffer(
        u32                   set,
        u32                   binding,
        const vulkan::Buffer& buffer,
        VkDeviceSize          offset,
        VkDeviceSize          range
    ) -> void;
    auto rebind_buffer(u32 set, u32 binding, const vulkan::Buffer& buffer) -> void;
    auto get_location(const std::string& name) -> std::tuple<u32, u32>;

public:
    const vulkan::LogicalDevice* m_device;
    vulkan::Pipeline             m_pipeline;
    ReflectedCode                m_reflected_code;

    std::array<vulkan::DescriptorSet, static_cast<u8>(vulkan::FREQUENCY::MAX)> m_sets;

    std::vector<vulkan::Buffer> m_uniforms;
};
} // namespace JadeFrame