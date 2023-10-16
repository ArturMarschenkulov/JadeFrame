#include "pch.h"
#include "vulkan_shader.h"
#include "vulkan_logical_device.h"
#include "vulkan_buffer.h"
#include "vulkan_renderer.h"

namespace JadeFrame {

Vulkan_Shader::Vulkan_Shader(
    const vulkan::LogicalDevice& device,
    const Vulkan_Renderer&       renderer,
    const Desc&                  desc
) {
    m_device = &device;
    Logger::info("Creating Vulkan shader");
    m_pipeline = vulkan::Pipeline(
        device,
        renderer.m_swapchain.m_extent,
        renderer.m_render_pass,
        desc.code,
        desc.vertex_format
    );
    Logger::info("Created Vulkan shader");
    m_reflected_code = m_pipeline.m_reflected_code;
    for (auto& module : m_reflected_code.m_modules) {
        for (auto& uniform_buffer : module.m_uniform_buffers) {
            Logger::info("Uniform buffer: {}", uniform_buffer.name);
            JF_ASSERT(
                uniform_buffer.size == sizeof(Matrix4x4),
                "Uniform buffer size is not 64 bytes"
            );
            m_device->create_buffer(
                vulkan::Buffer::TYPE::UNIFORM, nullptr, uniform_buffer.size
            );
        }
    }
}

auto Vulkan_Shader::bind_buffer(
    u32                   set,
    u32                   binding,
    const vulkan::Buffer& buffer,
    VkDeviceSize          offset,
    VkDeviceSize          range
) -> void {
    m_sets[set].bind_uniform_buffer(binding, buffer, offset, range);
    m_sets[set].update();
}

auto Vulkan_Shader::rebind_buffer(u32 set, u32 binding, const vulkan::Buffer& buffer)
    -> void {
    m_sets[set].rebind_uniform_buffer(binding, buffer);
    m_sets[set].update();
}

auto Vulkan_Shader::get_location(const std::string& name) -> std::tuple<u32, u32> {

    bool found = false;
    u32  set = 0;
    u32  binding = 0;
    for (auto& module : m_reflected_code.m_modules) {
        for (size_t j = 0; module.m_uniform_buffers.size(); j++) {
            auto& uniform_buffer = module.m_uniform_buffers[j];
            if (uniform_buffer.name == name) {
                found = true;
                set = uniform_buffer.set;
                binding = uniform_buffer.binding;
            }
        }
    }
    JF_ASSERT(found, "Uniform not found");

    return {set, binding};
}

} // namespace JadeFrame
