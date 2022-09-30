#include "pch.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_buffer.h"
#include "JadeFrame/utils/utils.h"
#include "JadeFrame/utils/assert.h"

#include <vector>
#include <cassert>

namespace JadeFrame {



namespace vulkan {
class LogicalDevice;
/*---------------------------
        Descriptor Set
---------------------------*/
auto DescriptorSet::add_uniform_buffer(u32 binding, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range)
    -> void {
    JF_ASSERT(buffer.m_size < from_kibibyte(64), "Guaranteed only between 16K and 64K");
    // Descriptor d = {
    //	.bufer_info = {
    //		.buffer = buffer.m_handle,
    //		.offset = offset,
    //		.range = range,
    //	},
    //	.binding = binding
    // };
    Descriptor d;
    d.bufer_info.buffer = buffer.m_handle;
    d.bufer_info.offset = offset;
    d.bufer_info.range = range;
    d.binding = binding;


    JF_ASSERT(d.bufer_info.offset < buffer.m_size, "");
    JF_ASSERT(d.bufer_info.range != VK_WHOLE_SIZE && d.bufer_info.range > 0, "");
    JF_ASSERT(d.bufer_info.range != VK_WHOLE_SIZE && d.bufer_info.range <= buffer.m_size - d.bufer_info.offset, "");

    // Find according to binding.
    // TODO: Maybe find a better way
    bool found = false;
    for (u32 i = 0; i < m_descriptors.size(); i++) {
        if (m_descriptors[i].binding == d.binding) {
            found = true;
            m_descriptors[i].bufer_info = d.bufer_info;
            // infos[i] = d.info;
        }
    }
    JF_ASSERT(found == true, "");
}
auto DescriptorSet::readd_uniform_buffer(u32 binding, const Buffer& buffer) -> void {

    for (u32 i = 0;; i++) {
        if (m_descriptors[i].binding == binding) {
            m_descriptors[binding].bufer_info.buffer = buffer.m_handle;
            return;
        }
    }
    assert(false);
    return;
}
auto DescriptorSet::update() -> void {

    std::vector<VkDescriptorBufferInfo> infos;
    infos.resize(m_descriptors.size());
    for (u32 i = 0; i < m_descriptors.size(); i++) { infos[i] = m_descriptors[i].bufer_info; }

    std::vector<VkWriteDescriptorSet> wdss;
    wdss.reserve(m_descriptors.size());
    for (u32 i = 0; i < m_descriptors.size(); i++) {

        const VkWriteDescriptorSet wds = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_handle,
            .dstBinding = m_descriptors[i].binding,
            .dstArrayElement = 0,
            .descriptorCount = 1 /*static_cast<u32>(m_descriptors.size())*/,
            .descriptorType =
                m_descriptors[i]
                    .type, // m_binding_map.at(m_descriptors[i].binding).descriptorType,//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &infos[i],
            .pTexelBufferView = nullptr,
        };
        wdss.push_back(wds);
    }

    vkUpdateDescriptorSets(m_device->m_handle, static_cast<u32>(wdss.size()), wdss.data(), 0, nullptr);
}



/*---------------------------
        Descriptor Set Layout
---------------------------*/

auto DescriptorSetLayout::add_binding(
    u32 binding, VkDescriptorType descriptor_type, u32 descriptor_count, VkShaderStageFlags stage_flags,
    const VkSampler* p_immutable_samplers) -> void {
    JF_ASSERT(m_handle == VK_NULL_HANDLE, "");
    const VkDescriptorSetLayoutBinding dslb = {
        .binding = binding,
        .descriptorType = descriptor_type,
        .descriptorCount = descriptor_count,
        .stageFlags = stage_flags,
        .pImmutableSamplers = p_immutable_samplers};
    if (dslb.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT && !(dslb.descriptorCount % 4 == 0)) {
        assert(false);
    }
    m_bindings.push_back(dslb);

    switch (descriptor_type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: m_dynamic_count++; break;
        default:;
    }
    {
        // Logger::info()
    }
}

auto DescriptorSetLayout::init(const LogicalDevice& device) -> void {

    m_device = &device;
    VkResult result;

    const VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<u32>(m_bindings.size()),
        .pBindings = m_bindings.data(),
    };

    result = vkCreateDescriptorSetLayout(device.m_handle, &layout_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);
    {
        Logger::info("Created descriptor set layout {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
        Logger::info("\tBindings: {}", m_bindings.size());
        for (const auto& b : m_bindings) {
            Logger::info("\t\tBinding: {}", b.binding);
            Logger::info("\t\t-Descriptor Type: {}", to_string(b.descriptorType));
            Logger::info("\t\t-Descriptor Count: {}", b.descriptorCount);
            Logger::info("\t\t-Stage Flags: {}", to_string_from_shader_stage_flags(b.stageFlags));
        }
    }
}
auto DescriptorSetLayout::deinit() -> void {
    vkDestroyDescriptorSetLayout(m_device->m_handle, m_handle, nullptr);
    { Logger::info("Destroyed descriptor set layout {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
}

/*---------------------------
        Descriptor Pool
---------------------------*/
auto DescriptorPool::add_pool_size(const VkDescriptorPoolSize& pool_size) -> void {
    JF_ASSERT(m_handle == VK_NULL_HANDLE, "");
    JF_ASSERT(pool_size.descriptorCount > 0, "");

    m_pool_sizes.push_back(pool_size);
    {
        Logger::info(
            "Added to descriptor pool {} a pool size {} of type {}", fmt::ptr(this), pool_size.descriptorCount,
            to_string(pool_size.type));
    }
}


auto DescriptorPool::init(const LogicalDevice& device, u32 max_sets) -> void {
    m_device = &device;
    VkResult                         result;
    const VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0 /* | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT*/,
        .maxSets = max_sets,
        .poolSizeCount = static_cast<u32>(m_pool_sizes.size()),
        .pPoolSizes = m_pool_sizes.data(),
    };
    JF_ASSERT(pool_info.maxSets > 0, "");
    JF_ASSERT(pool_info.poolSizeCount > 0, "");

    result = vkCreateDescriptorPool(device.m_handle, &pool_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) assert(false);
    {
        Logger::info("Created descriptor pool {} at {}", fmt::ptr(this), fmt::ptr(m_handle));
        Logger::info("\tmax sets: {}", max_sets);
        Logger::info("\tpools: {}", m_pool_sizes.size());
        for (const auto& pool_size : m_pool_sizes) {
            Logger::info("\t-size, type: {} {}", pool_size.descriptorCount, to_string(pool_size.type));
        }
    }
}

auto DescriptorPool::deinit() -> void {
    vkDestroyDescriptorPool(m_device->m_handle, m_handle, nullptr);
    { Logger::info("Destroyed descriptor pool {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
}

auto DescriptorPool::allocate_descriptor_sets(const DescriptorSetLayout& descriptor_set_layout, u32 amount)
    -> std::vector<DescriptorSet> {
    VkResult                           result;
    std::vector<VkDescriptorSetLayout> layouts(amount, descriptor_set_layout.m_handle);

    const VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_handle,
        .descriptorSetCount = static_cast<u32>(amount),
        .pSetLayouts = layouts.data(),
    };
    std::vector<VkDescriptorSet> handles(amount);
    result = vkAllocateDescriptorSets(m_device->m_handle, &alloc_info, handles.data());
    if (result != VK_SUCCESS) assert(false);
    {
        Logger::info(
            "Allocated {} descriptor sets from pool {} at {}", amount, fmt::ptr(this), fmt::ptr(*handles.data()));
    }

    std::vector<DescriptorSet> sets;
    sets.resize(handles.size());
    for (u32 i = 0; i < sets.size(); i++) {
        DescriptorSet& set = sets[i];
        set.m_handle = handles[i];
        set.m_device = m_device;

        set.m_layout = &descriptor_set_layout;
        set.m_descriptors.resize(descriptor_set_layout.m_bindings.size());
        for (u32 j = 0; j < descriptor_set_layout.m_bindings.size(); j++) {
            Descriptor& descr = set.m_descriptors[j];
            descr.binding = descriptor_set_layout.m_bindings[j].binding;
            descr.stage_flags = descriptor_set_layout.m_bindings[j].stageFlags;
            descr.type = descriptor_set_layout.m_bindings[j].descriptorType;
        }
    }
    {
        Logger::info(
            "Allocated {} descriptor sets from pool {} at {}", amount, fmt::ptr(this), fmt::ptr(*handles.data()));
        i32 i = 0;
        for (const auto& set : sets) {
            Logger::info("\tset {} at {}", i, fmt::ptr(set.m_handle));
            Logger::info("\tlayout at: {}", fmt::ptr(set.m_layout->m_handle));
            Logger::info("\tdescriptors: {}", set.m_descriptors.size());
            for (const auto& descr : set.m_descriptors) {
                Logger::info("\t\tbinding: {}", descr.binding);
                Logger::info("\t\t-type: {}", to_string(descr.type));
                Logger::info("\t\t-stage flags: {}", to_string_from_shader_stage_flags(descr.stage_flags));
            }
            i++;
        }
    }
    return sets;
}

auto DescriptorPool::allocate_descriptor_set(const DescriptorSetLayout& descriptor_set_layout) -> DescriptorSet {
    return this->allocate_descriptor_sets(descriptor_set_layout, 1)[0];
}

auto DescriptorPool::free_descriptor_sets(const std::vector<DescriptorSet>& /*descriptor_sets*/) -> void {
    // for(u32 i = 0; i < descriptor_sets.size(); i++) {
    //	VkResult result;
    //	result = vkFreeDescriptorSets(m_device->m_handle, m_handle, 1, &descriptor_sets[i].m_handle);
    //	if (result != VK_SUCCESS) __debugbreak();
    // }
    // VkResult result;
    // result = vkResetDescriptorPool(m_device->m_handle, m_handle, 0);
    // if (result != VK_SUCCESS) __debugbreak();
    vkDestroyDescriptorPool(m_device->m_handle, m_handle, nullptr);
    { Logger::info("Destroyed descriptor pool {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
}

auto DescriptorPool::free_descriptor_set(const DescriptorSet& descriptor_set) -> void {
    VkResult result;
    result = vkFreeDescriptorSets(m_device->m_handle, m_handle, 1, &descriptor_set.m_handle);
    if (result != VK_SUCCESS) assert(false);
    { Logger::info("Freed descriptor set {} from pool {}", fmt::ptr(&descriptor_set), fmt::ptr(this)); }
}
} // namespace vulkan
} // namespace JadeFrame