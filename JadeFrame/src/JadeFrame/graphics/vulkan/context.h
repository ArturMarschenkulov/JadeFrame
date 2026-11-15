#pragma once
#include <vector>

#include <vulkan/vulkan.h>

#include "logical_device.h"
#include "physical_device.h"
#include "surface.h"

namespace JadeFrame {

class Window;

namespace vulkan {
class PhysicalDevice;

class Instance {

public:
    Instance() = default;
    ~Instance();
    Instance(const Instance&) = delete;
    auto operator=(const Instance&) -> Instance& = delete;
    Instance(Instance&& other) = delete;
    auto operator=(Instance&& other) -> Instance& = delete;

    /// Creates the Vulkan instance, selects a physical device that can render and
    /// present to `window_handle` and creates the logical device.
    ///
    /// On failure everything created up to the failing step is released again before
    /// the call returns; the instance is then in its default constructed state.
    [[nodiscard]] auto init(Window* window_handle) -> Result<void, InitError>;
    /// Safe to call in any state, also repeatedly.
    auto deinit() -> void;

private:
    auto create(Window* window_handle) -> Result<void, InitError>;
    auto query_layers() -> std::vector<VkLayerProperties>;
    auto query_extensions() -> std::vector<VkExtensionProperties>;
    auto query_physical_devices() -> std::vector<vulkan::PhysicalDevice>;
    auto setup_debug() -> void;
    auto
    check_validation_layer_support(const std::span<VkLayerProperties>& available_layers)
        -> bool;

public:
    [[nodiscard]] auto create_surface(Window* window_handle) const
        -> Result<vulkan::Surface, InitError>;

public:
    VkInstance m_instance = VK_NULL_HANDLE;

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
    vulkan::PhysicalDevice*     m_physical_device = nullptr;
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

    /// See `vulkan::Instance::init`.
    [[nodiscard]] auto init(Window* window) -> Result<void, InitError>;

public:
    vulkan::Instance m_instance;
    Window* m_window_handle = nullptr;

public:
};
} // namespace JadeFrame
