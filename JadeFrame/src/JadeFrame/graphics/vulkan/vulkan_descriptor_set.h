#pragma once
#include <vulkan/vulkan.h>
#include "JadeFrame/prelude.h"

#include <vector>

namespace JadeFrame {

namespace vulkan {
class LogicalDevice;
class Buffer;
class Vulkan_Texture;
class DescriptorSetLayout;
class DescriptorPool;


enum class FREQUENCY : u8 {
    PER_FRAME,
    PER_PASS,
    PER_MATERIAL,
    PER_OBJECT,
    MAX,
};


struct Descriptor {
    Descriptor() = default;
    ~Descriptor() = default;
    Descriptor(const Descriptor&) = delete;
    auto operator=(const Descriptor&) -> Descriptor& = delete;
    Descriptor(Descriptor&& other);
    auto operator=(Descriptor&& other) -> Descriptor&;

    Descriptor(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorSetLayoutBinding binding);


    union {
        VkDescriptorBufferInfo buffer_info;
        VkDescriptorImageInfo  image_info;
    };
    VkDescriptorType   type;
    VkShaderStageFlags stage_flags;
    u32                binding;
};


class DescriptorSet {
public:
    DescriptorSet() = default;
    ~DescriptorSet() = default;
    DescriptorSet(const DescriptorSet&) = delete;
    auto operator=(const DescriptorSet&) -> DescriptorSet& = delete;
    DescriptorSet(DescriptorSet&& other);
    auto operator=(DescriptorSet&& other) -> DescriptorSet&;

    DescriptorSet(const LogicalDevice& device, VkDescriptorSet handle, const DescriptorSetLayout& layout);

    auto update() -> void;
    auto bind_uniform_buffer(u32 binding, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range) -> void;
    auto bind_combined_image_sampler(u32 binding, const Vulkan_Texture& texture) -> void;
    auto rebind_uniform_buffer(u32 binding, const Buffer& buffer) -> void;

public:
    VkDescriptorSet            m_handle;
    const LogicalDevice*       m_device = nullptr;
    const DescriptorSetLayout* m_layout = nullptr;

    std::vector<Descriptor> m_descriptors;
};

class DescriptorSetLayout {
public:
    struct Binding {
        u32                binding;
        VkDescriptorType   type;
        u32                count;
        VkShaderStageFlags stage_flags;
        const VkSampler*   p_immutable_samplers = nullptr;
    };

public:
    DescriptorSetLayout() = default;
    ~DescriptorSetLayout() = default;
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    auto operator=(const DescriptorSetLayout&) -> DescriptorSetLayout& = delete;
    DescriptorSetLayout(DescriptorSetLayout&& other) = default;
    auto operator=(DescriptorSetLayout&& other) -> DescriptorSetLayout& = default;

    DescriptorSetLayout(const LogicalDevice& device, std::vector<Binding> bindings);
    auto deinit() -> void;

private:
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
    DescriptorPool() = default;
    ~DescriptorPool();
    DescriptorPool(const DescriptorPool&) = delete;
    auto operator=(const DescriptorPool&) -> DescriptorPool& = delete;
    DescriptorPool(DescriptorPool&& other);
    auto operator=(DescriptorPool&& other) -> DescriptorPool&;
public:
    DescriptorPool(const LogicalDevice& device, u32 max_sets, std::vector<VkDescriptorPoolSize>& pool_sizes);

public:
    auto add_pool_size(const VkDescriptorPoolSize& pool_size) -> void;

    auto allocate_sets(const DescriptorSetLayout& descriptor_set_layout, u32 image_amount)
        -> std::vector<DescriptorSet>;
    auto allocate_set(const DescriptorSetLayout& descriptor_set_layout) -> DescriptorSet;
    auto free_sets(const std::vector<DescriptorSet>& descriptor_sets) -> void;
    auto free_set(const DescriptorSet& descriptor_sets) -> void;

public:
    const LogicalDevice*              m_device = nullptr;
    VkDescriptorPool                  m_handle = VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> m_pool_sizes;
};
} // namespace vulkan
} // namespace JadeFrame