#include "pch.h"

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include "vulkan_context.h"
#include "vulkan_shared.h"
#include "vulkan_physical_device.h"
#include "vulkan_debug.h"
#include "JadeFrame/platform/windows/windows_window.h"


#include "JadeFrame/prelude.h"


#include <set>
#include <JadeFrame/base_app.h>
#include "JadeFrame/utils/assert.h"

namespace JadeFrame {


// static auto get_required_instance_extensions(u32* count) -> const char* {
//     const char* extensions[2];
//     extensions[0] = "VK_KHR_surface";
//     extensions[1] = "VK_KHR_win32_surface";
//     return *extensions;
// }

struct VulkanVersion {
    u32 variant;
    u32 major;
    u32 minor;
    u32 patch;
};
static auto vulkan_get_api_version(u32 version) -> VulkanVersion {
    VulkanVersion result;
    result.variant = VK_API_VERSION_VARIANT(version);
    result.major = VK_API_VERSION_MAJOR(version);
    result.minor = VK_API_VERSION_MINOR(version);
    result.patch = VK_API_VERSION_PATCH(version);
    return result;
}
static auto vulkan_get_device_type_string(const VkPhysicalDeviceType& device_type) -> const char* {
    const char* result = "";
    switch (device_type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: result = "VK_PHYSICAL_DEVICE_TYPE_OTHER"; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: result = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: result = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: result = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: result = "VK_PHYSICAL_DEVICE_TYPE_CPU"; break;
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            JF_ASSERT(false, "");
            result = "";
            break;
    }
    return result;
}
auto VulkanInstance::check_validation_layer_support(const std::vector<VkLayerProperties>& layers) -> bool {
#if 1
    for (u32 i = 0; i < m_desired_layer_names.size(); i++) {
        bool found = false;
        for (u32 j = 0; j < layers.size(); j++) {
            if (strcmp(m_desired_layer_names[i], layers[j].layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) { return false; }
    }
    return true;
#else
    for (const auto& desired_layer_name : m_desired_layer_names) {
        bool layer_found = std::any_of(layers.begin(), layers.end(), [&desired_layer_name](const auto& a_layer) {
            return strcmp(desired_layer_name, a_layer.layerName) == 0;
        });

        if (!layer_found) { return false; }
    }
    return true;
#endif
}

static auto is_device_suitable(const vulkan::PhysicalDevice& physical_device, vulkan::Surface& surface) -> bool {

    auto formats = physical_device.query_surface_formats(surface);
    auto present_modes = physical_device.query_surface_present_modes(surface);

    bool swapchain_adequate = false;
    if (physical_device.m_extension_support == true) {
        swapchain_adequate = !formats.empty() && !present_modes.empty();
    }
    return physical_device.m_queue_family_indices.is_complete() && physical_device.m_extension_support &&
           swapchain_adequate;
}

auto VulkanInstance::query_layers() -> std::vector<VkLayerProperties> {
    u32 layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers;
    available_layers.resize(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    if (!this->check_validation_layer_support(available_layers)) { assert(false); }
    return available_layers;
}

auto VulkanInstance::query_extensions() -> std::vector<VkExtensionProperties> {
    VkResult result;

    u32 extension_count = 0;
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    if (result != VK_SUCCESS) assert(false);

    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    if (result != VK_SUCCESS) assert(false);

    return extensions;
}

auto VulkanInstance::query_physical_devices() -> std::vector<vulkan::PhysicalDevice> {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
    if (device_count == 0) assert(false);

    std::vector<VkPhysicalDevice> phys_devices;
    phys_devices.resize(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, phys_devices.data());


    std::vector<vulkan::PhysicalDevice> physical_devices;
    physical_devices.resize(device_count);
    for (u32 i = 0; i < physical_devices.size(); i++) { physical_devices[i].m_handle = phys_devices[i]; }
    return physical_devices;
}

auto VulkanInstance::setup_debug() -> void {
    VkResult result;
    if (!m_enable_validation_layers) return;
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    vulkan::populate_debug_messenger_create_info(create_info);

    result = vkCreateDebugUtilsMessengerEXT_(m_instance, &create_info, nullptr, &m_debug_messenger);
    if (result != VK_SUCCESS) assert(false);
}

auto VulkanInstance::init(const IWindow* window_handle) -> void {
    Logger::trace("VulkanInstance::init start");

    VkResult result;

    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
    VkValidationFeaturesEXT      features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.pNext = VK_NULL_HANDLE;
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;
    features.disabledValidationFeatureCount = 0;
    features.pDisabledValidationFeatures = VK_NULL_HANDLE;

    const VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "JadeFrame Application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "JadeFrame",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };

    // layers
    m_layers = this->query_layers();
    if (m_enable_validation_layers && !this->check_validation_layer_support(m_layers)) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    std::vector<const char*> layer_names;
    layer_names.resize(m_layers.size());
    for (u32 i = 0; i < m_layers.size(); i++) { layer_names[i] = m_layers[i].layerName; }
    {
        Logger::debug("Printing Layer names:");
        for (auto& layer_name : layer_names) { Logger::debug("\t{}", layer_name); }
    }

    // extensions
    // std::vector<VkExtensionProperties> extensions = this->query_extensions();
    m_extensions = this->query_extensions();
    std::vector<const char*> extension_names;
    extension_names.resize(m_extensions.size());
    for (u32 i = 0; i < m_extensions.size(); i++) { extension_names[i] = m_extensions[i].extensionName; }
    {
        Logger::debug("Printing Extension names:");
        for (auto& extension_name : extension_names) { Logger::debug("\t{}", extension_name); }
    }
    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<u32>(m_layers.size()),
        .ppEnabledLayerNames = layer_names.data(),
        .enabledExtensionCount = static_cast<u32>(m_extensions.size()),
        .ppEnabledExtensionNames = extension_names.data(),
    };


    if (m_enable_validation_layers) {
        create_info.enabledLayerCount = static_cast<u32>(m_desired_layer_names.size());
        create_info.ppEnabledLayerNames = m_desired_layer_names.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
        vulkan::populate_debug_messenger_create_info(debug_create_info);
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS) assert(false);

    { Logger::info("Created Vulkan Instance {} at {}", fmt::ptr(this), fmt::ptr(m_instance)); }
    if (m_enable_validation_layers) {
        VkDebugUtilsMessengerCreateInfoEXT info;
        vulkan::populate_debug_messenger_create_info(info);

        result = vkCreateDebugUtilsMessengerEXT_(m_instance, &info, nullptr, &m_debug_messenger);
        if (result != VK_SUCCESS) assert(false);
    }
    m_surface = this->create_surface(window_handle);


    Logger::debug("Querying Physical Devices");
    Logger::debug("There are {} physical devices", m_physical_devices.size());
    m_physical_devices = this->query_physical_devices();
    for (u32 i = 0; i < m_physical_devices.size(); i++) { m_physical_devices[i].init(*this, m_surface); }
    for (u32 i = 0; i < m_physical_devices.size(); i++) {
        if (is_device_suitable(m_physical_devices[i], m_surface)) {
            m_physical_device = &m_physical_devices[i];
            break;
        }
    }

    m_logical_device.init(*this, *m_physical_device, m_surface);
    // m_logical_device = m_physical_device.create_logical_device();
    Logger::trace("VulkanInstance::init end");
}

auto VulkanInstance::deinit() -> void {
    if (m_enable_validation_layers) { vkDestroyDebugUtilsMessengerEXT_(m_instance, m_debug_messenger, nullptr); }
    m_surface.deinit();
    vkDestroyInstance(m_instance, nullptr);
}

auto VulkanInstance::create_surface(const IWindow* window_handle) -> vulkan::Surface {
    vulkan::Surface surface;
    surface.init(m_instance, window_handle);
    return surface;
}

Vulkan_Context::Vulkan_Context(const IWindow* window) {
    m_window_handle = window;
    m_instance.init(window);
}

Vulkan_Context::~Vulkan_Context() {
    m_instance.m_logical_device.deinit();
    m_instance.deinit();
}




} // namespace JadeFrame