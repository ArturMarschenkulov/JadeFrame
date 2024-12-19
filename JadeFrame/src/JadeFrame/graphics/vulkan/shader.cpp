#include "pch.h"
#include "shader.h"
#include "logical_device.h"
#include "buffer.h"
#include "renderer.h"

namespace JadeFrame {

Vulkan_Shader::Vulkan_Shader(
    const vulkan::LogicalDevice& device,
    const Vulkan_Renderer&       renderer,
    const Desc&                  desc
)
    : m_device(&device) {

    Logger::info("Creating Vulkan shader");
    m_pipeline = vulkan::Pipeline(
        device, renderer.m_swapchain.m_extent, renderer.m_render_pass, desc.code
    );
    Logger::info("Created Vulkan shader");
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

static constexpr auto ceil_to_aligned(const u64 value, const u64 alignment) -> u64 {
#if 0 // more efficient
        const u64 new_val = (value + alignment - 1) & ~(alignment - 1);
        const u64 aligned_block_size = alignment > 0 ? new_val : value;
#else // more readable
    const u64 new_val = (value + alignment - (value % alignment));
    const u64 aligned_block_size = (value % alignment == 0) ? value : new_val;
#endif
    return aligned_block_size;
}

Vulkan_Material::Vulkan_Material(
    vulkan::LogicalDevice&  device,
    Vulkan_Shader&          shader,
    vulkan::Vulkan_Texture* texture
)
    : m_device(&device)
    , m_shader(&shader)
    , m_texture(texture) {



    const auto& pipeline = m_shader->m_pipeline;
    for (size_t i = 0; i < pipeline.m_set_layouts.size(); i++) {
        const auto& set_layout = pipeline.m_set_layouts[i];
        m_sets[i] = device.m_set_pool.allocate_set(set_layout);
    }

    for (const auto& uniform_buffer : pipeline.m_reflected_interface.m_uniform_buffers) {
        u32 set = uniform_buffer.set;
        u32 binding = uniform_buffer.binding;
        u32 size = uniform_buffer.size;

        JF_ASSERT(size == sizeof(mat4x4), "Uniform buffer size is not 64 bytes");
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
    vulkan::DescriptorSet& set = m_sets[vulkan::FREQUENCY::PER_MATERIAL];
    if (set.m_descriptors.empty()) {
        Logger::err("The shader does not support textures");
        assert(false);
    }
    set.bind_combined_image_sampler(0, *texture);
    set.update();
}

auto Vulkan_Material::bind_buffer(
    u32                   set,
    u32                   binding,
    const vulkan::Buffer& buffer,
    VkDeviceSize          offset,
    VkDeviceSize          range
) -> void {
    m_sets[set].bind_uniform_buffer(binding, buffer, offset, range);
    m_sets[set].update();
}

auto Vulkan_Material::rebind_buffer(u32 set, u32 binding, const vulkan::Buffer& buffer)
    -> void {
    m_sets[set].rebind_uniform_buffer(binding, buffer);
    m_sets[set].update();
}

auto Vulkan_Material::write_ub(
    vulkan::FREQUENCY frequency,
    u32               index,
    const void*       data,
    size_t            size,
    size_t            offset
) -> void {
    // TODO: Make it more solid

    if (frequency == vulkan::FREQUENCY::PER_OBJECT) {
        vulkan::Buffer* ub = m_uniform_buffers[frequency][index];
        ub->write(data, size, offset);
    } else {
        assert(offset == 0);
        vulkan::Buffer* ub = m_uniform_buffers[frequency][index];
        ub->write(data, size, offset);
    }
}

auto Vulkan_Material::set_dynamic_ub_num(u32 num) -> void {
    const vulkan::PhysicalDevice* pd = m_device->m_physical_device;

    auto type_size = sizeof(mat4x4);

    const u64 dyn_alignment =
        ceil_to_aligned(type_size, pd->limits().minUniformBufferOffsetAlignment);

    for (auto& [index, ub] : m_uniform_buffers[vulkan::FREQUENCY::PER_OBJECT]) {
        if (num * dyn_alignment == ub->m_size) { return; }
        ub->resize(num * dyn_alignment);
        this->rebind_buffer(vulkan::FREQUENCY::PER_OBJECT, index, *ub);
    }
}
} // namespace JadeFrame
