#include "pch.h"
#include "vulkan_descriptor_set.h"
#include "vulkan_logical_device.h"
#include "vulkan_context.h"
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
        Descriptor
---------------------------*/

Descriptor::Descriptor(Descriptor&& other) {
    this->buffer_info = other.buffer_info;
    this->image_info = other.image_info;
    this->type = other.type;
    this->stage_flags = other.stage_flags;
    this->binding = other.binding;

    other.buffer_info = {};
    other.image_info = {};
    other.type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    other.stage_flags = 0;
    other.binding = 0;
}
auto Descriptor::operator=(Descriptor&& other) -> Descriptor& {
    this->buffer_info = other.buffer_info;
    this->image_info = other.image_info;
    this->type = other.type;
    this->stage_flags = other.stage_flags;
    this->binding = other.binding;

    other.buffer_info = {};
    other.image_info = {};
    other.type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    other.stage_flags = 0;
    other.binding = 0;
    return *this;
}

Descriptor::Descriptor(
    const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorSetLayoutBinding t_binding) {

    binding = t_binding.binding;
    stage_flags = t_binding.stageFlags;
    type = t_binding.descriptorType;

    buffer_info.buffer = buffer.m_handle;
    buffer_info.offset = offset;
    buffer_info.range = range;
}



/*---------------------------
        Descriptor Set
---------------------------*/

DescriptorSet::~DescriptorSet() {
    // if (m_handle != VK_NULL_HANDLE) {
    //     vkFreeDescriptorSets(m_device->m_handle, m_layout->m_pool->m_handle, 1, &m_handle);
    // }
}

DescriptorSet::DescriptorSet(DescriptorSet&& other) {
    this->m_handle = other.m_handle;
    this->m_device = other.m_device;
    this->m_layout = other.m_layout;
    this->m_descriptors = std::move(other.m_descriptors);

    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_layout = nullptr;
}
auto DescriptorSet::operator=(DescriptorSet&& other) -> DescriptorSet& {
    this->m_handle = other.m_handle;
    this->m_device = other.m_device;
    this->m_layout = other.m_layout;
    this->m_descriptors = std::move(other.m_descriptors);

    other.m_handle = VK_NULL_HANDLE;
    other.m_device = nullptr;
    other.m_layout = nullptr;
    return *this;
}


DescriptorSet::DescriptorSet(const LogicalDevice& device, VkDescriptorSet handle, const DescriptorSetLayout& layout) {
    m_handle = handle;
    m_device = &device;

    m_layout = &layout;

    m_descriptors.resize(layout.m_bindings.size());
}



static auto is_image(VkDescriptorType type) -> bool {
    bool result = false;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            result = true;
            break;
            // case VK_DESCRIPTOR_TYPE_SAMPLER:
        default: result = false;
    }
    return result;
}
static auto is_sampler(VkDescriptorType type) -> bool {
    bool result = false;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER: result = true; break;
        default: result = false;
    }
    return result;
}
static auto is_uniform(VkDescriptorType type) -> bool {
    bool result = false;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: result = true; break;
        default: result = false;
    }
    return result;
}
static auto is_storage(VkDescriptorType type) -> bool {
    bool result = false;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: result = true; break;
        default: result = false;
    }
    return result;
}
static auto is_buffer(VkDescriptorType type) -> bool { return is_uniform(type) || is_storage(type); }
static auto is_dynamic(VkDescriptorType type) -> bool {
    bool result = false;
    switch (type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: result = true; break;
        default: result = false;
    }
    return result;
}

auto DescriptorSet::bind_uniform_buffer(u32 binding, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range)
    -> void {

    JF_ASSERT(buffer.m_size < from_kibibyte(64), "Guaranteed only between 16K and 64K");
    JF_ASSERT(offset < buffer.m_size, "offset mustn't be greater than buffer size");
    JF_ASSERT(range != VK_WHOLE_SIZE && range > 0, "range mustn't be 0 or VK_WHOLE_SIZE");
    JF_ASSERT(range != VK_WHOLE_SIZE && range <= buffer.m_size - offset, "range mustn't be greater than buffer size");


    for (u32 i = 0; i < m_descriptors.size(); i++) {
        if (m_layout->m_bindings[i].binding == binding) {
            JF_ASSERT(true == is_uniform(m_layout->m_bindings[i].descriptorType), "type mismatch");
            m_descriptors[i] = Descriptor(buffer, offset, range, m_layout->m_bindings[i]);
            return;
        }
    }
    JF_ASSERT(false, "");
}

auto DescriptorSet::rebind_uniform_buffer(u32 binding, const Buffer& buffer) -> void {

    for (u32 i = 0; i < m_descriptors.size(); i++) {
        if (m_descriptors[i].binding == binding) {
            m_descriptors[binding].buffer_info.buffer = buffer.m_handle;
            return;
        }
    }
    assert(false);
    return;
}

auto DescriptorSet::bind_combined_image_sampler(u32 binding, const Vulkan_Texture& texture) -> void {


    // Find according to binding.
    bool found = false;
    for (u32 i = 0; i < m_descriptors.size(); i++) {
        if (m_descriptors[i].binding == binding) {
            Descriptor d;
            d.image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            d.image_info.imageView = texture.m_image_view.m_handle;
            d.image_info.sampler = texture.m_sampler.m_handle;
            d.binding = binding;
            d.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            m_descriptors[i] = std::move(d);
            // m_descriptors[i].image_info = d.image_info;
            return;
        }
    }
    JF_ASSERT(found == true, "");
}

auto get_infos(const std::vector<Descriptor>& descriptors)
    -> std::pair<std::vector<VkDescriptorBufferInfo>, std::vector<VkDescriptorImageInfo>> {
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorImageInfo>  image_infos;
    for (u32 i = 0; i < descriptors.size(); i++) {
        auto& d = descriptors[i];
        if (is_image(d.type)) {
            image_infos.push_back(d.image_info);
        } else {
            buffer_infos.push_back(d.buffer_info);
        }
    }
    return {buffer_infos, image_infos};
}

auto DescriptorSet::update() -> void {
    auto [buffer_infos, image_infos] = get_infos(m_descriptors);

    std::vector<VkWriteDescriptorSet> sets;
    sets.reserve(m_descriptors.size());
    for (u32 i = 0; i < m_descriptors.size(); i++) {
        auto& d = m_descriptors[i];

        const VkWriteDescriptorSet set = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_handle,
            .dstBinding = d.binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = d.type,
            .pImageInfo = image_infos.data(),
            .pBufferInfo = buffer_infos.data(),
            .pTexelBufferView = nullptr,
        };
        sets.push_back(set);
    }

    vkUpdateDescriptorSets(
        m_device->m_handle,            // device
        static_cast<u32>(sets.size()), // descriptorWriteCount
        sets.data(),                   // pDescriptorWrites
        0,                             // descriptorCopyCount
        nullptr                        // pDescriptorCopies
    );
}



/*---------------------------
    Descriptor Set Layout
---------------------------*/

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other)
    : m_device(other.m_device)
    , m_handle(other.m_handle)
    , m_bindings(std::move(other.m_bindings))
    , m_dynamic_count(other.m_dynamic_count) {

    other.m_device = nullptr;
    other.m_handle = VK_NULL_HANDLE;
    other.m_bindings = {};
    other.m_dynamic_count = 0;
}

auto DescriptorSetLayout::operator=(DescriptorSetLayout&& other) -> DescriptorSetLayout& {
    if (this != &other) {
        this->m_device = other.m_device;
        this->m_handle = other.m_handle;
        this->m_bindings = std::move(other.m_bindings);
        this->m_dynamic_count = other.m_dynamic_count;

        other.m_device = nullptr;
        other.m_handle = VK_NULL_HANDLE;
        other.m_bindings = {};
        other.m_dynamic_count = 0;
    }
    return *this;
}

DescriptorSetLayout::~DescriptorSetLayout() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device->m_handle, m_handle, Instance::allocator());
        { Logger::info("Destroyed descriptor set layout {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
    }
}

DescriptorSetLayout::DescriptorSetLayout(const LogicalDevice& device, const std::vector<Binding>& bindings) {
    m_device = &device;
    VkResult result;

    for (int i = 0; i < bindings.size(); i++) {
        this->add_binding(
            bindings[i].binding, bindings[i].type, bindings[i].count, bindings[i].stage_flags,
            bindings[i].p_immutable_samplers);
    }

    const VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<u32>(m_bindings.size()),
        .pBindings = m_bindings.data(),
    };

    result = vkCreateDescriptorSetLayout(device.m_handle, &layout_info, Instance::allocator(), &m_handle);
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

auto DescriptorSetLayout::add_binding(
    u32 binding, VkDescriptorType descriptor_type, u32 descriptor_count, VkShaderStageFlags stage_flags,
    const VkSampler* p_immutable_samplers) -> void {
    JF_ASSERT(m_handle == VK_NULL_HANDLE, "");
    const VkDescriptorSetLayoutBinding layout = {
        .binding = binding,
        .descriptorType = descriptor_type,
        .descriptorCount = descriptor_count,
        .stageFlags = stage_flags,
        .pImmutableSamplers = p_immutable_samplers};

    if (layout.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT && !(layout.descriptorCount % 4 == 0)) {
        Logger::err("Inline uniform block must be a multiple of 4");
        assert(false);
    }
    m_bindings.push_back(layout);

    if (is_dynamic(descriptor_type)) { m_dynamic_count++; }

    {
        // Logger::info()
    }
}

/*---------------------------
        Descriptor Pool
---------------------------*/

DescriptorPool::DescriptorPool(DescriptorPool&& other) {
    this->m_device = other.m_device;
    this->m_handle = other.m_handle;
    this->m_pool_sizes = std::move(other.m_pool_sizes);

    other.m_device = nullptr;
    other.m_handle = VK_NULL_HANDLE;
}
auto DescriptorPool::operator=(DescriptorPool&& other) -> DescriptorPool& {
    if (this != &other) {
        this->m_device = other.m_device;
        this->m_handle = other.m_handle;
        this->m_pool_sizes = std::move(other.m_pool_sizes);

        other.m_device = nullptr;
        other.m_handle = VK_NULL_HANDLE;
    }
    return *this;
}

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

DescriptorPool::DescriptorPool(
    const LogicalDevice& device, u32 max_sets, std::vector<VkDescriptorPoolSize>& pool_sizes) {

    m_device = &device;
    VkResult result;

    for (int i = 0; i < pool_sizes.size(); i++) { this->add_pool_size(pool_sizes[i]); }

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

    result = vkCreateDescriptorPool(device.m_handle, &pool_info, Instance::allocator(), &m_handle);
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

DescriptorPool::~DescriptorPool() {
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->m_handle, m_handle, Instance::allocator());
        { Logger::info("Destroyed descriptor pool {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
    }
}

auto DescriptorPool::allocate_sets(const DescriptorSetLayout& layout, u32 amount) -> std::vector<DescriptorSet> {
    VkResult                           result;
    std::vector<VkDescriptorSetLayout> layouts(amount, layout.m_handle);

    const VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_handle,
        .descriptorSetCount = static_cast<u32>(amount),
        .pSetLayouts = layouts.data(),
    };
    std::vector<VkDescriptorSet> handles(amount);
    result = vkAllocateDescriptorSets(m_device->m_handle, &alloc_info, handles.data());
    if (result != VK_SUCCESS) {
        Logger::err("Failed to allocate descriptor sets {}", to_string(result));
        assert(false);
    }
    {
        Logger::info(
            "Allocated {} descriptor sets from pool {} at {}", amount, fmt::ptr(this), fmt::ptr(*handles.data()));
    }
    std::vector<DescriptorSet> sets;
    sets.resize(handles.size());
    for (u32 i = 0; i < sets.size(); i++) { sets[i] = DescriptorSet(*m_device, handles[i], layout); }

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

auto DescriptorPool::allocate_set(const DescriptorSetLayout& descriptor_set_layout) -> DescriptorSet {
    return std::move(this->allocate_sets(descriptor_set_layout, 1)[0]);
}

auto DescriptorPool::free_sets(const std::vector<DescriptorSet>& /*descriptor_sets*/) -> void {
    // for(u32 i = 0; i < descriptor_sets.size(); i++) {
    //	VkResult result;
    //	result = vkFreeDescriptorSets(m_device->m_handle, m_handle, 1, &descriptor_sets[i].m_handle);
    //	if (result != VK_SUCCESS) assert(false);
    // }
    // VkResult result;
    // result = vkResetDescriptorPool(m_device->m_handle, m_handle, 0);
    // if (result != VK_SUCCESS) assert(false);
    vkDestroyDescriptorPool(m_device->m_handle, m_handle, Instance::allocator());
    { Logger::info("Destroyed descriptor pool {} at {}", fmt::ptr(this), fmt::ptr(m_handle)); }
}

auto DescriptorPool::free_set(const DescriptorSet& descriptor_set) -> void {
    VkResult result;
    result = vkFreeDescriptorSets(m_device->m_handle, m_handle, 1, &descriptor_set.m_handle);
    if (result != VK_SUCCESS) assert(false);
    { Logger::info("Freed descriptor set {} from pool {}", fmt::ptr(&descriptor_set), fmt::ptr(this)); }
}
} // namespace vulkan
} // namespace JadeFrame