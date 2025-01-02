
#include <set>
#ifdef WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
    #define VK_USE_PLATFORM_XLIB_KHR
#endif
#include "context.h"
#include "physical_device.h"
#include "debug.h"
#if defined(_WIN32)
    #include "JadeFrame/platform/windows/windows_window.h"
#elif defined(__linux__)
    #include "JadeFrame/platform/linux/linux_window.h"
#endif

#include "JadeFrame/prelude.h"

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

namespace vulkan {
auto Instance::allocator() -> VkAllocationCallbacks* {
    return Instance::default_allocator();
}

auto Instance::default_allocator() -> VkAllocationCallbacks* { return nullptr; }

auto Instance::check_validation_layer_support(const std::span<VkLayerProperties>& layers
) -> bool {
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
        bool layer_found = std::any_of(
            layers.begin(),
            layers.end(),
            [&desired_layer_name](const auto& a_layer) {
                return strcmp(desired_layer_name, a_layer.layerName) == 0;
            }
        );

        if (!layer_found) { return false; }
    }
    return true;
#endif
}

static auto is_device_suitable(vulkan::PhysicalDevice& physical_device) -> bool {

    return physical_device.m_chosen_queue_family_pointers.is_complete() &&
           physical_device.m_extension_support;
}

static auto choose_physical_device(std::span<vulkan::PhysicalDevice> devices
) -> vulkan::PhysicalDevice* {
    for (u32 i = 0; i < devices.size(); i++) {
        if (is_device_suitable(devices[i])) { return &devices[i]; }
    }
    assert(false);
}

auto Instance::query_layers() -> std::vector<VkLayerProperties> {
    u32 layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers;
    available_layers.resize(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    if (!this->check_validation_layer_support(available_layers)) { assert(false); }
    return available_layers;
}

auto Instance::query_extensions() -> std::vector<VkExtensionProperties> {
    VkResult result = VK_SUCCESS;

    u32 extension_count = 0;
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    if (result != VK_SUCCESS) { assert(false); }

    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
    if (result != VK_SUCCESS) { assert(false); }

    return extensions;
}

auto Instance::query_physical_devices() -> std::vector<vulkan::PhysicalDevice> {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
    if (device_count == 0) { assert(false); }

    std::vector<VkPhysicalDevice> phys_devices;
    phys_devices.resize(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, phys_devices.data());

    std::vector<vulkan::PhysicalDevice> physical_devices;
    physical_devices.resize(device_count);
    for (u32 i = 0; i < physical_devices.size(); i++) {
        physical_devices[i].m_handle = phys_devices[i];
    }
    return physical_devices;
}

auto Instance::setup_debug() -> void {
    VkResult result;
    if (!m_enable_validation_layers) { return; }
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    vulkan::populate_debug_messenger_create_info(create_info);

    result = vkCreateDebugUtilsMessengerEXT_(
        m_instance, &create_info, Instance::allocator(), &m_debug_messenger
    );
    if (result != VK_SUCCESS) { assert(false); }
}

auto Instance::init() -> void {
    Logger::trace("Instance::init start");

    VkResult result;

    constexpr bool                            enable_validation_layers = true;
    std::vector<VkValidationFeatureEnableEXT> vals;
    if (enable_validation_layers) {
        vals.push_back(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);
        // vals.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
        // vals.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
        // );
        // vals.push_back(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
        // vals.push_back(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT);
    }
    VkValidationFeaturesEXT features = {};
    features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    features.pNext = VK_NULL_HANDLE;
    features.enabledValidationFeatureCount = vals.size();
    features.pEnabledValidationFeatures = vals.data();
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
    for (u32 i = 0; i < m_extensions.size(); i++) {
        extension_names[i] = m_extensions[i].extensionName;
    }
    {
        Logger::debug("Printing Extension names:");
        for (auto& extension_name : extension_names) {
            Logger::debug("\t{}", extension_name);
        }
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

    result = vkCreateInstance(&create_info, Instance::allocator(), &m_instance);
    if (result != VK_SUCCESS) { assert(false); }

    {
        Logger::info(
            "Created Vulkan Instance {} at {}", fmt::ptr(this), fmt::ptr(m_instance)
        );
    }
    if (m_enable_validation_layers) {
        VkDebugUtilsMessengerCreateInfoEXT info;
        vulkan::populate_debug_messenger_create_info(info);

        result = vkCreateDebugUtilsMessengerEXT_(
            m_instance, &info, Instance::allocator(), &m_debug_messenger
        );
        if (result != VK_SUCCESS) { assert(false); }
    }

    Logger::debug("Querying Physical Devices");
    Logger::debug("There are {} physical devices", m_physical_devices.size());
    m_physical_devices = this->query_physical_devices();
    for (u32 i = 0; i < m_physical_devices.size(); i++) {
        m_physical_devices[i].init(*this);
    }

    m_physical_device = choose_physical_device(m_physical_devices);

    m_logical_device.init(*this, *m_physical_device);
    // m_logical_device = m_physical_device.create_logical_device();
    Logger::trace("Instance::init end");
}

auto Instance::deinit() -> void {
    if (m_enable_validation_layers) {
        vkDestroyDebugUtilsMessengerEXT_(
            m_instance, m_debug_messenger, Instance::allocator()
        );
    }
    vkDestroyInstance(m_instance, nullptr);
}

auto Instance::create_surface(const Window* window_handle) -> vulkan::Surface {
    vulkan::Surface surface(m_instance, window_handle);
    return surface;
}
} // namespace vulkan

Vulkan_Context::Vulkan_Context(const Window* window)
    : m_window_handle(window) {
    m_instance.init();
}

Vulkan_Context::~Vulkan_Context() {
    m_instance.m_logical_device.deinit();
    m_instance.deinit();
}

} // namespace JadeFrame