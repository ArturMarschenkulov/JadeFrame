#include "shader.h"
#include "JadeFrame/utils/logger.h"
#include "logical_device.h"
#include "buffer.h"
#include "renderer.h"

#include <algorithm>

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
    for (const auto& uniform_buffer : m_pipeline.m_reflected_interface.m_uniform_buffers) {
        if (uniform_buffer.name == name) {
            return {uniform_buffer.set, uniform_buffer.binding};
        }
    }
    Logger::err("Uniform '{}' was not found in the reflected pipeline", name);
    return {0, 0};
}

static constexpr auto ceil_to_aligned(const u64 value, const u64 alignment) -> u64 {
    if (alignment == 0) { return value; }
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
    if (!pipeline.m_is_compiled) {
        Logger::err("Cannot create a Vulkan material for an uncompiled pipeline");
        return;
    }

    m_sets.resize(pipeline.m_set_layouts.size());
    for (size_t i = 0; i < pipeline.m_set_layouts.size(); i++) {
        const auto& set_layout = pipeline.m_set_layouts[i];
        m_sets[i] = device.m_set_pool.allocate_set(set_layout);
    }

    for (const auto& uniform_buffer : pipeline.m_reflected_interface.m_uniform_buffers) {
        u32 set = uniform_buffer.set;
        u32 binding = uniform_buffer.binding;
        u32 size = uniform_buffer.size;

        if (set >= m_sets.size()) {
            Logger::err(
                "Uniform buffer {} references descriptor set {} outside the pipeline layout",
                uniform_buffer.name,
                set
            );
            continue;
        }
        if (uniform_buffer.descriptor_count != 1) {
            Logger::err(
                "Uniform buffer {} uses descriptor array count {}; only scalar bindings are "
                "supported",
                uniform_buffer.name,
                uniform_buffer.descriptor_count
            );
            continue;
        }
        if (size == 0) {
            Logger::err("Uniform buffer {} has an empty layout", uniform_buffer.name);
            continue;
        }

        const u64 alignment = device.m_physical_device->limits().minUniformBufferOffsetAlignment;
        const VkDeviceSize dynamic_stride =
            set == static_cast<u32>(vulkan::FREQUENCY::PER_OBJECT)
                ? ceil_to_aligned(size, alignment)
                : size;
        vulkan::Buffer* buf =
            device.create_buffer(vulkan::Buffer::TYPE::UNIFORM, nullptr, size);
        this->bind_buffer(set, binding, *buf, 0, size);
        m_uniform_buffers[set][binding] = UniformBufferAllocation{
            .m_buffer = buf,
            .m_size = size,
            .m_dynamic_stride = dynamic_stride,
        };
    }
    for (const auto& sampled_image : pipeline.m_reflected_interface.m_sampled_images) {
        u32 set_index = sampled_image.set;
        u32 binding_index = sampled_image.binding;
        if (set_index >= m_sets.size()) {
            Logger::err(
                "Sampled image {} references descriptor set {} outside the pipeline layout",
                sampled_image.name,
                set_index
            );
            continue;
        }
        if (sampled_image.descriptor_count != 1) {
            Logger::err(
                "Sampled image {} uses descriptor array count {}; only scalar bindings are "
                "supported",
                sampled_image.name,
                sampled_image.descriptor_count
            );
            continue;
        }
        if (m_texture != nullptr) {
            vulkan::DescriptorSet& set = m_sets[set_index];
            if (set.m_descriptors.empty()) {
                Logger::err("The shader does not support textures");
                continue;
            }
            set.bind_combined_image_sampler(binding_index, *m_texture);
            set.update();
        }
    }
}

Vulkan_Material::~Vulkan_Material() {
    if (m_device == nullptr) { return; }

    for (auto& [set, buffers] : m_uniform_buffers) {
        (void)set;
        for (auto& [binding, allocation] : buffers) {
            (void)binding;
            if (allocation.m_buffer != nullptr) {
                m_device->destroy_buffer(allocation.m_buffer);
                allocation.m_buffer = nullptr;
            }
        }
    }
    m_uniform_buffers.clear();
}

auto Vulkan_Material::bind_buffer(
    u32                   set,
    u32                   binding,
    const vulkan::Buffer& buffer,
    VkDeviceSize          offset,
    VkDeviceSize          range
) -> void {
    if (set >= m_sets.size()) {
        Logger::err("Descriptor set {} is not present in the Vulkan pipeline", set);
        return;
    }
    m_sets[set].bind_uniform_buffer(binding, buffer, offset, range);
    m_sets[set].update();
}

auto Vulkan_Material::rebind_buffer(u32 set, u32 binding, const vulkan::Buffer& buffer)
    -> void {
    if (set >= m_sets.size()) {
        Logger::err("Descriptor set {} is not present in the Vulkan pipeline", set);
        return;
    }
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
    const u32 set = static_cast<u32>(frequency);
    const auto set_it = m_uniform_buffers.find(set);
    if (set_it == m_uniform_buffers.end()) {
        Logger::err("No uniform buffers exist for descriptor set {}", set);
        return;
    }
    const auto buffer_it = set_it->second.find(index);
    if (buffer_it == set_it->second.end() || buffer_it->second.m_buffer == nullptr) {
        Logger::err(
            "Uniform buffer at descriptor set {} binding {} was not reflected",
            set,
            index
        );
        return;
    }
    if (data == nullptr || size == 0) {
        Logger::err("Cannot write an empty uniform buffer update at set {} binding {}", set, index);
        return;
    }
    if (frequency != vulkan::FREQUENCY::PER_OBJECT && offset != 0) {
        Logger::err("Non-dynamic uniform buffer updates must use offset zero");
        return;
    }

    const vulkan::Buffer* buffer = buffer_it->second.m_buffer;
    if (offset > buffer->m_size || size > buffer->m_size - offset) {
        Logger::err(
            "Uniform buffer update at set {} binding {} exceeds its allocation",
            set,
            index
        );
        return;
    }
    buffer->write(data, size, offset);
}

auto Vulkan_Material::set_dynamic_ub_num(u32 num) -> void {
    const auto set_it = m_uniform_buffers.find(vulkan::FREQUENCY::PER_OBJECT);
    if (set_it == m_uniform_buffers.end()) { return; }

    const u64 element_count = std::max<u64>(num, 1);
    for (auto& [index, allocation] : set_it->second) {
        if (allocation.m_buffer == nullptr || allocation.m_dynamic_stride == 0) { continue; }
        const u64 required_size = element_count * allocation.m_dynamic_stride;
        if (required_size == allocation.m_buffer->m_size) { continue; }
        allocation.m_buffer->resize(required_size);
        this->rebind_buffer(
            static_cast<u32>(vulkan::FREQUENCY::PER_OBJECT),
            index,
            *allocation.m_buffer
        );
    }
}

auto Vulkan_Material::get_dynamic_uniform_stride(u32 set, u32 binding) const
    -> VkDeviceSize {
    const auto set_it = m_uniform_buffers.find(set);
    if (set_it == m_uniform_buffers.end()) { return 0; }
    const auto buffer_it = set_it->second.find(binding);
    if (buffer_it == set_it->second.end()) { return 0; }
    return buffer_it->second.m_dynamic_stride;
}
} // namespace JadeFrame
