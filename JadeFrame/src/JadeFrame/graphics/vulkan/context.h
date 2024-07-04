#pragma once
#include <vulkan/vulkan.h>
#include "surface.h"
#include "physical_device.h"
#include "logical_device.h"
#include <vector>

namespace JadeFrame {

class Window;

namespace vulkan {
class PhysicalDevice;

class Instance {

public:
    Instance() = default;
    ~Instance() = default;
    Instance(const Instance&) = delete;
    auto operator=(const Instance&) -> Instance& = delete;
    Instance(Instance&& other) = delete;
    auto operator=(Instance&& other) -> Instance& = delete;

    auto init(const Window* window_handle) -> void;
    auto deinit() -> void;

private:
    auto query_layers() -> std::vector<VkLayerProperties>;
    auto query_extensions() -> std::vector<VkExtensionProperties>;
    auto query_physical_devices() -> std::vector<vulkan::PhysicalDevice>;
    auto setup_debug() -> void;
    auto
    check_validation_layer_support(const std::span<VkLayerProperties>& available_layers)
        -> bool;

public:
    auto create_surface(const Window* window_handle) -> vulkan::Surface;

public:
    VkInstance m_instance = VK_NULL_HANDLE;
    Surface    m_surface;

    std::vector<VkLayerProperties> m_layers;
    std::vector<const char*>       m_desired_layer_names =
        {"VK_LAYER_KHRONOS_validation" /*, "VK_LAYER_LUNARG_monitor"*/};
    // std::vector<VkLayerProperties> m_desired_layers;

    std::vector<VkExtensionProperties> m_extensions;

    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
#ifdef NDEBUG
    bool m_enable_validation_layers = false;
#else
    bool m_enable_validation_layers = true;
#endif

    std::vector<PhysicalDevice> m_physical_devices;
    vulkan::PhysicalDevice*     m_physical_device;
    vulkan::LogicalDevice       m_logical_device;

public:
    static auto allocator() -> VkAllocationCallbacks*;
    static auto default_allocator() -> VkAllocationCallbacks*;
};
} // namespace vulkan

struct Vulkan_Context {
    Vulkan_Context() = default;
    ~Vulkan_Context();

    Vulkan_Context(const Vulkan_Context&) = delete;
    auto operator=(const Vulkan_Context&) -> Vulkan_Context& = delete;

    Vulkan_Context(Vulkan_Context&&) = delete;
    auto operator=(Vulkan_Context&&) -> Vulkan_Context& = delete;

    Vulkan_Context(const Window* window);

public:
    vulkan::Instance m_instance;
    const Window*    m_window_handle;

public:
};
} // namespace JadeFrame