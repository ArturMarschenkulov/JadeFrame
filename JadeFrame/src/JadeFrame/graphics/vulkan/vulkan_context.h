#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_surface.h"
#include "vulkan_physical_device.h"
#include "vulkan_logical_device.h"
#include <vector>


namespace JadeFrame {

#ifdef _WIN32

class IWindow;

#elif __linux__
class Linux_Window;
using Window = Linux_Window;
#endif
namespace vulkan {
class PhysicalDevice;
}
class VulkanInstance {


public:
    VulkanInstance() = default;
    ~VulkanInstance() = default;
    VulkanInstance(const VulkanInstance&) = delete;
    auto operator=(const VulkanInstance&) -> VulkanInstance& = delete;
    VulkanInstance(VulkanInstance&& other) = delete;
    auto operator=(VulkanInstance&& other) -> VulkanInstance& = delete;

    auto init(const IWindow* window_handle) -> void;
    auto deinit() -> void;

private:
    auto query_layers() -> std::vector<VkLayerProperties>;
    auto query_extensions() -> std::vector<VkExtensionProperties>;
    auto query_physical_devices() -> std::vector<vulkan::PhysicalDevice>;
    auto setup_debug() -> void;
    auto check_validation_layer_support(const std::vector<VkLayerProperties>& available_layers) -> bool;

    auto create_surface(const IWindow* window_handle) -> vulkan::Surface;

public:
    VkInstance      m_instance = VK_NULL_HANDLE;
    vulkan::Surface m_surface;

    std::vector<VkLayerProperties> m_available_layers;
    const std::vector<const char*> m_desired_layer_names = {
        "VK_LAYER_KHRONOS_validation" /*, "VK_LAYER_LUNARG_monitor"*/};
    // std::vector<VkLayerProperties> m_desired_layers;

    std::vector<VkExtensionProperties> m_available_extensions;

    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
#ifdef NDEBUG
    const bool m_enable_validation_layers = false;
#else
    const bool m_enable_validation_layers = true;
#endif

    std::vector<vulkan::PhysicalDevice> m_physical_devices;
    vulkan::PhysicalDevice              m_physical_device;
    vulkan::LogicalDevice               m_logical_device;
    
public:
    static auto default_allocator() -> VkAllocationCallbacks* { return nullptr; }
};

struct Vulkan_Context {
    Vulkan_Context() = default;
    ~Vulkan_Context();

    Vulkan_Context(const Vulkan_Context&) = delete;
    auto operator=(const Vulkan_Context&) -> Vulkan_Context& = delete;

    Vulkan_Context(Vulkan_Context&&) = delete;
    auto operator=(Vulkan_Context&&) -> Vulkan_Context& = delete;

    Vulkan_Context(const IWindow* window);

public:
    VulkanInstance m_instance;
    const IWindow* m_window_handle;

public:
};
} // namespace JadeFrame