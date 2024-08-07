#pragma once
#include "../graphics_shared.h"
#include "pipeline.h"
#include <tuple>
#include <unordered_map>

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

    auto write_ub(
        vulkan::FREQUENCY frequency,
        u32               index,
        const void*       data,
        size_t            size,
        size_t            offset
    ) -> void;

    auto set_dynamic_ub_num(u32 num) -> void;

public:
    const vulkan::LogicalDevice* m_device = nullptr;
    vulkan::Pipeline             m_pipeline;

    std::array<vulkan::DescriptorSet, static_cast<u8>(vulkan::FREQUENCY::MAX)> m_sets;

    template<typename K, typename V>
    using HashMap = std::unordered_map<K, V>;

    template<typename K0, typename V>
    using Hashmap2 = std::unordered_map<K0, HashMap<K0, V>>;

    Hashmap2<u32, vulkan::Buffer*> m_uniform_buffers;
};
} // namespace JadeFrame