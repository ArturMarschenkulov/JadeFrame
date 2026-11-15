
#include <set>
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
    #define VK_USE_PLATFORM_XLIB_KHR
#endif
#include "context.h"
#include "JadeFrame/utils/logger.h"
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

namespace vulkan {
Instance::~Instance() { this->deinit(); }

auto Instance::allocator() -> VkAllocationCallbacks* {
    return Instance::default_allocator();
}

auto Instance::default_allocator() -> VkAllocationCallbacks* { return nullptr; }

auto Instance::check_validation_layer_support(const std::span<VkLayerProperties>& layers)
    -> bool {
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
            layers.begin(), layers.end(), [&desired_layer_name](const auto& a_layer) {
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

static auto choose_physical_device(std::span<vulkan::PhysicalDevice> devices)
    -> vulkan::PhysicalDevice* {
    for (u32 i = 0; i < devices.size(); i++) {
        if (is_device_suitable(devices[i])) { return &devices[i]; }
    }
    return nullptr;
}

auto Instance::query_layers() -> std::vector<VkLayerProperties> {
    u32 layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers;
    available_layers.resize(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    return available_layers;
}

auto Instance::query_extensions() -> std::vector<VkExtensionProperties> {
    VkResult result = VK_SUCCESS;

    u32 extension_count = 0;
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    if (result != VK_SUCCESS) { return {}; }

    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extension_count);
    result = vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_count, extensions.data()
    );
    if (result != VK_SUCCESS) { return {}; }
    extensions.resize(extension_count);

    return extensions;
}

auto Instance::query_physical_devices() -> std::vector<vulkan::PhysicalDevice> {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
    if (device_count == 0) { return {}; }

    std::vector<VkPhysicalDevice> phys_devices;
    phys_devices.resize(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, phys_devices.data());
    phys_devices.resize(device_count);

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

static auto context_error(std::string message) -> Result<void, InitError> {
    return err<void, InitError>(InitError{
        InitErrorCode::CONTEXT_CREATION_FAILED,
        GRAPHICS_API::VULKAN,
        std::move(message),
    });
}

auto Instance::init(Window* window_handle) -> Result<void, InitError> {
    Logger::trace("Instance::init start");
    auto result = this->create(window_handle);
    // `create` has returned, so the surface it used for the device selection is gone
    // and the instance can be destroyed.
    if (result.is_err()) { this->deinit(); }
    Logger::trace("Instance::init end");
    return result;
}

auto Instance::create(Window* window_handle) -> Result<void, InitError> {
    if (window_handle == nullptr) {
        return err<void, InitError>(InitError{
            InitErrorCode::INVALID_ARGUMENT,
            GRAPHICS_API::VULKAN,
            "Vulkan cannot be initialized without a window",
        });
    }

    VkResult result;

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
        return context_error(
            "this build requests the Vulkan validation layer "
            "VK_LAYER_KHRONOS_validation, which is not installed"
        );
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
    if (result != VK_SUCCESS) {
        m_instance = VK_NULL_HANDLE;
        return context_error(std::string("vkCreateInstance failed: ") + to_string(result));
    }

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
        if (result != VK_SUCCESS) {
            m_debug_messenger = VK_NULL_HANDLE;
            return context_error(
                std::string("failed to create the Vulkan debug messenger: ") +
                to_string(result)
            );
        }
    }

    // Present support is a property of a queue family and a surface. Create the
    // surface before selecting a physical device so the selected present family
    // can be included in vkCreateDevice's queue-create-info list.
    auto selection_surface = this->create_surface(window_handle);
    if (selection_surface.is_err()) {
        return err<void, InitError>(std::move(selection_surface).error());
    }

    Logger::debug("Querying Physical Devices");
    m_physical_devices = this->query_physical_devices();
    Logger::debug("There are {} physical devices", m_physical_devices.size());
    if (m_physical_devices.empty()) {
        return context_error("no Vulkan physical device was found");
    }
    for (u32 i = 0; i < m_physical_devices.size(); i++) {
        m_physical_devices[i].init(*this, selection_surface.value());
    }

    m_physical_device = choose_physical_device(m_physical_devices);
    if (m_physical_device == nullptr) {
        return context_error(
            "none of the " + std::to_string(m_physical_devices.size()) +
            " Vulkan physical devices provides graphics, present and transfer queues "
            "for the window together with the VK_KHR_swapchain extension"
        );
    }

    return m_logical_device.init(*this, *m_physical_device);
}

auto Instance::deinit() -> void {
    if (m_instance == VK_NULL_HANDLE) { return; }

    m_logical_device.deinit();

    if (m_enable_validation_layers && m_debug_messenger != VK_NULL_HANDLE) {
        vkDestroyDebugUtilsMessengerEXT_(
            m_instance, m_debug_messenger, Instance::allocator()
        );
        m_debug_messenger = VK_NULL_HANDLE;
    }
    vkDestroyInstance(m_instance, Instance::allocator());
    m_instance = VK_NULL_HANDLE;
    m_physical_device = nullptr;
    m_physical_devices.clear();
}

auto Instance::create_surface(Window* window_handle) const
    -> Result<vulkan::Surface, InitError> {
    return vulkan::Surface::create(m_instance, window_handle);
}
} // namespace vulkan

auto Vulkan_Context::init(Window* window) -> Result<void, InitError> {
    m_window_handle = window;
    return m_instance.init(window);
}

Vulkan_Context::~Vulkan_Context() = default;

} // namespace JadeFrame
