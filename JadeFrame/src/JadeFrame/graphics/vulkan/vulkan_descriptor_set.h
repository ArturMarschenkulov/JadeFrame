#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/prelude.h"

#include <vector>

namespace JadeFrame {

namespace vulkan {
class LogicalDevice;
class Buffer;
class DescriptorSetLayout;
class DescriptorPool;


enum class DESCRIPTOR_SET_FREQUENCY : u8 {
    PER_FRAME,
    PER_PASS,
    PER_MATERIAL,
    PER_OBJECT,
    MAX,
};


struct Descriptor {
    union {
        VkDescriptorBufferInfo bufer_info;
        VkDescriptorImageInfo  image_info;
    };
    VkDescriptorType   type;
    VkShaderStageFlags stage_flags;
    u32                binding;
};
inline auto is_image(Descriptor d) -> bool {
    switch (d.type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return true;
            break;
            // case VK_DESCRIPTOR_TYPE_SAMPLER:
        default: return false;
    }
}

class DescriptorSet {
public:
    auto update() -> void;
    auto add_uniform_buffer(u32 binding, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range) -> void;
    auto readd_uniform_buffer(u32 binding, const Buffer& buffer) -> void;

public:
    VkDescriptorSet            m_handle;
    const LogicalDevice*       m_device = nullptr;
    const DescriptorSetLayout* m_layout = nullptr;

    std::vector<Descriptor> m_descriptors;

    // su32 m_dynamic_count;
};

class DescriptorSetLayout {

public:
    auto init(const LogicalDevice& device) -> void;
    auto deinit() -> void;

    auto add_binding(
        u32 binding, VkDescriptorType descriptor_type, u32 descriptor_count, VkShaderStageFlags stage_flags,
        const VkSampler* p_immutable_samplers = nullptr) -> void;

public:
    VkDescriptorSetLayout                     m_handle = VK_NULL_HANDLE;
    const LogicalDevice*                      m_device = nullptr;
    std::vector<VkDescriptorSetLayoutBinding> m_bindings;

    u32 m_dynamic_count = 0;
};

class DescriptorPool {
public:
    auto init(const LogicalDevice& device, u32 max_sets) -> void;
    auto deinit() -> void;

    auto add_pool_size(const VkDescriptorPoolSize& pool_size) -> void;

    auto allocate_descriptor_sets(const DescriptorSetLayout& descriptor_set_layout, u32 image_amount)
        -> std::vector<DescriptorSet>;
    auto allocate_descriptor_set(const DescriptorSetLayout& descriptor_set_layout) -> DescriptorSet;
    auto free_descriptor_sets(const std::vector<DescriptorSet>& descriptor_sets) -> void;
    auto free_descriptor_set(const DescriptorSet& descriptor_sets) -> void;

public:
    const LogicalDevice*              m_device = nullptr;
    VkDescriptorPool                  m_handle = VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> m_pool_sizes;
};
} // namespace vulkan
} // namespace JadeFrame