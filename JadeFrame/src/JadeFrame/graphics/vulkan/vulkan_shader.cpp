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
)
    : m_device(&device) {

    Logger::info("Creating Vulkan shader");
    m_pipeline = vulkan::Pipeline(
        device,
        renderer.m_swapchain.m_extent,
        renderer.m_render_pass,
        desc.code,
        desc.vertex_format
    );
    Logger::info("Created Vulkan shader");

    for (size_t i = 0; i < m_pipeline.m_set_layouts.size(); i++) {
        const auto& set_layout = m_pipeline.m_set_layouts[i];
        m_sets[i] = device.m_set_pool.allocate_set(set_layout);
    }

    for (auto& uniform_buffer : m_pipeline.m_reflected_interface.m_uniform_buffers) {
        auto set = uniform_buffer.set;
        auto binding = uniform_buffer.binding;
        auto size = uniform_buffer.size;

        JF_ASSERT(size == sizeof(Matrix4x4), "Uniform buffer size is not 64 bytes");
        vulkan::Buffer* buf =
            device.create_buffer(vulkan::Buffer::TYPE::UNIFORM, nullptr, size);
        this->bind_buffer(set, binding, *buf, 0, size);
        m_uniform_buffers[set][binding] = buf;

        // Logger::info("Uniform buffer: {}", uniform_buffer.name);
        // Logger::info(
        //     "reflected uniform buffers {}",
        //     m_pipeline.m_reflected_interface.m_uniform_buffers.size()
        // );
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
    for (auto& module : m_pipeline.m_reflected_code.m_modules) {
        for (size_t j = 0; j < module.m_uniform_buffers.size(); j++) {
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

static auto get_aligned_block_size(const u64 block_size, const u64 alignment) -> u64 {
#if 0 // more efficient
        const u64 new_val = (block_size + alignment - 1) & ~(alignment - 1);
        const u64 aligned_block_size = alignment > 0 ? new_val : block_size;
#else // more readable
    const u64 new_val = (block_size + alignment - (block_size % alignment));
    const u64 aligned_block_size = (block_size % alignment == 0) ? block_size : new_val;
#endif
    return aligned_block_size;
}

auto Vulkan_Shader::write_ub(
    vulkan::FREQUENCY frequency,
    u32               index,
    const void*       data,
    size_t            size,
    size_t            offset
) -> void {
    // TODO: Make it more solid

    if (frequency == vulkan::FREQUENCY::PER_OBJECT) {
        auto* ub = m_uniform_buffers[frequency][index];
        ub->write(data, size, offset);
    } else {
        assert(offset == 0);
        auto* ub = m_uniform_buffers[frequency][index];
        ub->write(data, size, offset);
    }
}

auto Vulkan_Shader::set_dynamic_ub_num(u32 num) -> void {
    const vulkan::PhysicalDevice* pd = m_device->m_physical_device;

    auto type_size = sizeof(Matrix4x4);

    const u64 dyn_alignment = get_aligned_block_size(
        type_size, pd->query_limits().minUniformBufferOffsetAlignment
    );

    for (auto& [index, ub] : m_uniform_buffers[vulkan::FREQUENCY::PER_OBJECT]) {
        if(num * dyn_alignment == ub->m_size) {
            return;
        }
        ub->resize(num * dyn_alignment);
        this->rebind_buffer(vulkan::FREQUENCY::PER_OBJECT, index, *ub);
    }
}
} // namespace JadeFrame
