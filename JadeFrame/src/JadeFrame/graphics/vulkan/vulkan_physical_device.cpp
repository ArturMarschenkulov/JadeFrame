#include "pch.h"
#include "vulkan_physical_device.h"

#include "vulkan_context.h"
#include "vulkan_surface.h"

#include <set>
#include <string>
#ifdef _WIN32
#include "Windows.h" // TODO: Try to remove it. Used in "choose_swap_extent()"
#endif
#include "JadeFrame/utils/logger.h"

#undef min
#undef max

namespace JadeFrame {

static auto to_format_string_queue_family(const VulkanQueueFamily& queue_family) -> std::string {
    std::string result = "{ ";
    if (queue_family.m_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) { result += "Graphics "; }
    if (queue_family.m_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) { result += "Compute "; }
    if (queue_family.m_properties.queueFlags & VK_QUEUE_TRANSFER_BIT) { result += "Transfer "; }
    if (queue_family.m_properties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) { result += "SparseBinding "; }
    if (queue_family.m_properties.queueFlags & VK_QUEUE_PROTECTED_BIT) { result += "Protected "; }
    if (queue_family.m_present_support == VK_TRUE) { result += "Present "; }
    result += "}";
    return result;
}

static auto query_surface_support_details(const VulkanPhysicalDevice& physical_device, const VulkanSurface& surface)
    -> SurfaceSupportDetails {
    VkResult              result;
    SurfaceSupportDetails surface_support_details;
    u32                   count = 0;

    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device.m_handle, surface.m_handle, &surface_support_details.m_capabilities);



    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.m_handle, surface.m_handle, &count, nullptr);
    if (VK_SUCCESS != result || (count == 0)) assert(false);

    surface_support_details.m_formats.resize(count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device.m_handle, surface.m_handle, &count, surface_support_details.m_formats.data());
    if (VK_SUCCESS != result) assert(false);



    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.m_handle, surface.m_handle, &count, nullptr);
    if (VK_SUCCESS != result || (count == 0)) assert(false);

    surface_support_details.m_present_modes.resize(count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device.m_handle, surface.m_handle, &count, surface_support_details.m_present_modes.data());
    if (VK_SUCCESS != result) assert(false);

    return surface_support_details;
}
// template<typename Flag>
// auto decode_bitmask(const Flag&) -> std::vector<Flag> {
//
// }
auto to_string(const VkPhysicalDeviceType& device_type) -> std::string {
    switch (device_type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        default: return "Unknown";
    }
}
auto to_string(uint8_t pipeline_cache_UUID[16]) -> std::string {
    std::string result;
    for (auto i = 0; i < 16; ++i) {
        result += std::to_string(pipeline_cache_UUID[i]);
        if (i != 15) { result += ", "; }
    }
    return result;
}
auto to_string_vendor_id(uint32_t vendor_id) -> std::string {
    switch (vendor_id) {
        case 0x1002: return "AMD";
        case 0x10DE: return "NVIDIA";
        case 0x8086: return "Intel";
        case 0x13B5: return "ARM";
        case 0x5143: return "Qualcomm";
        case 0x1010: return "ImgTec";
        case 0x1AE0: return "Vivante";
        case 0x1B36: return "Broadcom";
        case 0x1D11: return "Imagination";
        case 0x1FC9: return "Mediatek";
        case 0x534D: return "Mesa";
        case 0x8087: return "Xilinx";
        default: return "Unknown";
    }
}
auto to_string(const VkMemoryType& memory_type) -> std::string {
    std::string result = "{ ";
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { result += "DeviceLocal "; }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { result += "HostVisible "; }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { result += "HostCoherent "; }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { result += "HostCached "; }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) { result += "LazilyAllocated "; }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) { result += "Protected "; }
    // AMD Specific
    // if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) { result += "DeviceCoherent "; }
    // if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) { result += "DeviceUncached "; }
    // NV Specific
    // if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV ) { result += "Protected "; }
    result += "}";
    return result;
}
auto to_string(const VkMemoryHeap& memory_heap) -> std::string {
    std::string result = "{ ";
    if (memory_heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) { result += "DeviceLocal "; }
    if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) { result += "MultiInstance "; }
    if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR) { result += "MultiInstance_KHR "; }
    result += "}";
    return result;
}
auto VulkanPhysicalDevice::init(VulkanInstance& instance, const VulkanSurface& surface) -> void {
    m_instance_p = &instance;
    VkResult result;
    vkGetPhysicalDeviceProperties(m_handle, &m_properties);
    vkGetPhysicalDeviceFeatures(m_handle, &m_features);
    vkGetPhysicalDeviceMemoryProperties(m_handle, &m_memory_properties);

    m_surface_support_details = query_surface_support_details(*this, surface);

    /*
        Quick Vulkan Queue Family Guide:
        - Graphics: Used for rendering. (VK_QUEUE_GRAPHICS_BIT)
        - Compute: Used for general purpose computation. E.g. Ray tracing, physics simulation, etc.
       (VK_QUEUE_COMPUTE_BIT)
        - Transfer: Used for data transfer operations. (VK_QUEUE_TRANSFER_BIT)
        - Sparse Binding: Used for sparse resources. (VK_QUEUE_SPARSE_BINDING_BIT)
        - Protected: Used for protected memory. (VK_QUEUE_PROTECTED_BIT)
        - Present: Used for presenting to a surface.

        Our GPU/Physical device has to be able to at least do graphics and presenting.

    */
    m_queue_families = this->query_queue_families(surface);
    m_queue_family_indices = this->find_queue_families(m_queue_families);




    { // Query_extension_properties
        u32 count = 0;
        result = vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, nullptr);
        m_extension_properties.resize(count);
        result = vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, m_extension_properties.data());
        if (VK_SUCCESS != result) assert(false);
    }
    m_extension_support = this->check_extension_support(m_device_extensions);

    {
        Logger::info("Physical device: {}", m_properties.deviceName);
        Logger::info("\tDevice Type: {}", to_string(m_properties.deviceType));
        Logger::info("\tVendor (ID): {} ({})", to_string_vendor_id(m_properties.vendorID), m_properties.vendorID);
        Logger::info("\tDevice ID: {}", m_properties.deviceID);
        Logger::info("\tDriver Version: {}", m_properties.driverVersion);
        Logger::info(
            "\tVulkan Api Version: {}.{}.{}", VK_VERSION_MAJOR(m_properties.apiVersion),
            VK_VERSION_MINOR(m_properties.apiVersion), VK_VERSION_PATCH(m_properties.apiVersion));
        Logger::info("\tPipeline Cache UUID: {}", to_string(m_properties.pipelineCacheUUID));
        Logger::info("\t{} memory types from {}:", m_memory_properties.memoryTypeCount, VK_MAX_MEMORY_TYPES);
        for (auto i = 0; i < m_memory_properties.memoryTypeCount; ++i) {
            VkMemoryType memory_type = m_memory_properties.memoryTypes[i];
            Logger::info("\t\t{}: {} {}", i, memory_type.heapIndex, to_string(memory_type));
        }
        Logger::info("\t{} memory heaps from {}:", m_memory_properties.memoryHeapCount, VK_MAX_MEMORY_HEAPS);
        for (auto i = 0; i < m_memory_properties.memoryHeapCount; ++i) {
            VkMemoryHeap memory_heap = m_memory_properties.memoryHeaps[i];
            Logger::info("\t\t{}: {} with {} bytes", i, to_string(memory_heap), memory_heap.size);
        }
        for (int i = 0; i < m_queue_families.size(); i++) {
            VulkanQueueFamily& queue_family = m_queue_families[i];
            Logger::debug(
                "Queue family {} has {} queues capable of {}", i, queue_family.m_properties.queueCount,
                to_format_string_queue_family(queue_family));
        }
    }
}

auto VulkanPhysicalDevice::check_extension_support(const std::vector<const char*>& extensions) -> bool {
    std::set<std::string> required_extensions(extensions.begin(), extensions.end());
    for (u32 i = 0; i < m_extension_properties.size(); i++) {
        required_extensions.erase(m_extension_properties[i].extensionName);
    }
    return required_extensions.empty();
}


auto VulkanPhysicalDevice::find_queue_families(const std::vector<VulkanQueueFamily>& queue_families)
    -> QueueFamilyIndices {

    QueueFamilyIndices indices;
    for (u32 i = 0; i < queue_families.size(); i++) {
        if (queue_families[i].m_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.m_graphics_family = queue_families[i].m_index;
        }
        VkBool32 present_support = false;
        present_support = queue_families[i].m_present_support;


        if (present_support) { indices.m_present_family = queue_families[i].m_index; }
        if (indices.is_complete()) { break; }
    }
    return indices;
}
auto VulkanPhysicalDevice::find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties) const -> u32 {
    const VkPhysicalDeviceMemoryProperties& mem_props = m_memory_properties;
    for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}
auto VulkanPhysicalDevice::query_queue_families(const VulkanSurface& surface) -> std::vector<VulkanQueueFamily> {
    u32 count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_properties;
    queue_family_properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, queue_family_properties.data());

    std::vector<VulkanQueueFamily> families;
    families.resize(count);
    for (u32 i = 0; i < count; i++) {
        families[i].m_index = i;
        families[i].m_properties = queue_family_properties[i];
        vkGetPhysicalDeviceSurfaceSupportKHR(
            m_handle, families[i].m_index, surface.m_handle, &families[i].m_present_support);
    }

    return families;
}
} // namespace JadeFrame