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

    auto get_location(const std::string& name) -> std::tuple<u32, u32>;

public:
    const vulkan::LogicalDevice* m_device = nullptr;
    vulkan::Pipeline             m_pipeline;
};

class Vulkan_Material {
public:
    Vulkan_Material() = default;
    ~Vulkan_Material() = default;
    Vulkan_Material(const Vulkan_Material&) = delete;
    auto operator=(const Vulkan_Material&) -> Vulkan_Material& = delete;
    Vulkan_Material(Vulkan_Material&&) noexcept = default;
    auto operator=(Vulkan_Material&&) -> Vulkan_Material& = default;

    Vulkan_Material(
        vulkan::LogicalDevice&  device,
        Vulkan_Shader&          shader,
        vulkan::Vulkan_Texture* texture
    );

public:
    auto bind_buffer(
        u32                   set,
        u32                   binding,
        const vulkan::Buffer& buffer,
        VkDeviceSize          offset,
        VkDeviceSize          range
    ) -> void;
    auto rebind_buffer(u32 set, u32 binding, const vulkan::Buffer& buffer) -> void;

    auto write_ub(
        vulkan::FREQUENCY frequency,
        u32               index,
        const void*       data,
        size_t            size,
        size_t            offset
    ) -> void;

    auto set_dynamic_ub_num(u32 num) -> void;

public:
    vulkan::LogicalDevice*  m_device = nullptr;
    Vulkan_Shader*          m_shader = nullptr;
    vulkan::Vulkan_Texture* m_texture = nullptr;
    std::array<vulkan::DescriptorSet, static_cast<u8>(vulkan::FREQUENCY::MAX)> m_sets;

    template<typename K, typename V>
    using HashMap = std::unordered_map<K, V>;

    template<typename K0, typename V>
    using Hashmap2 = std::unordered_map<K0, HashMap<K0, V>>;

    Hashmap2<u32, vulkan::Buffer*> m_uniform_buffers;
};
} // namespace JadeFrame