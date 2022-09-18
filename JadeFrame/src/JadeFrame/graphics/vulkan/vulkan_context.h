#pragma once
#include <vulkan/vulkan.h>
#include "vulkan_surface.h"
#include "vulkan_physical_device.h"
#include "vulkan_logical_device.h"
#include <vector>


namespace JadeFrame {

#ifdef _WIN32
namespace win32 {
class Window;
}

using Window = win32::Window;
#elif __linux__
class Linux_Window;
using Window = Linux_Window;
#endif
class VulkanPhysicalDevice;

class VulkanInstance {

private:
    auto query_layers() -> std::vector<VkLayerProperties>;
    auto query_extensions() -> std::vector<VkExtensionProperties>;
    auto query_physical_devices() -> std::vector<VulkanPhysicalDevice>;
    auto setup_debug() -> void;
    auto check_validation_layer_support(const std::vector<VkLayerProperties>& available_layers) -> bool;

public:
    VulkanInstance() = default;
    auto init(HWND window_handle) -> void;
    auto deinit() -> void;

public:
    VkInstance m_instance = VK_NULL_HANDLE;
    // HWND m_window_handle;
    VulkanSurface m_surface;

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

    std::vector<VulkanPhysicalDevice> m_physical_devices;
    VulkanPhysicalDevice              m_physical_device;
    VulkanLogicalDevice               m_logical_device;
};

struct Vulkan_Context {
    Vulkan_Context(const Vulkan_Context&) = delete;
    Vulkan_Context(Vulkan_Context&&) = delete;
    auto operator=(const Vulkan_Context&) -> Vulkan_Context& = delete;
    auto operator=(Vulkan_Context&&) -> Vulkan_Context& = delete;

    Vulkan_Context() = default;
    Vulkan_Context(const Window& window);
    ~Vulkan_Context();

public:
    VulkanInstance m_instance;
    HWND           m_window_handle;

public:
};
} // namespace JadeFrame