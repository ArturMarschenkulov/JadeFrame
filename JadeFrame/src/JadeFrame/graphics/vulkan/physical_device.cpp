#include "pch.h"
#include "physical_device.h"
#include "context.h"
#include "surface.h"

#include <set>
#include <string>
#include "JadeFrame/utils/logger.h"

#undef min
#undef max

namespace JadeFrame {

namespace vulkan {

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

auto PhysicalDevice::query_queue_family_properties() const
    -> std::vector<VkQueueFamilyProperties> {
    u32 count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, nullptr);
    std::vector<VkQueueFamilyProperties> properties;
    properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &count, properties.data());
    return properties;
}

auto PhysicalDevice::query_surface_capabilities(const Surface& surface) const
    -> VkSurfaceCapabilitiesKHR {
    VkSurfaceCapabilitiesKHR result;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle, surface.m_handle, &result);
    return result;
}

auto PhysicalDevice::query_surface_formats(const Surface& surface) const
    -> std::vector<VkSurfaceFormatKHR> {
    u32      count = 0;
    VkResult result =
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_handle, surface.m_handle, &count, nullptr);
    if (VK_SUCCESS != result || (count == 0)) { assert(false); }

    std::vector<VkSurfaceFormatKHR> formats(count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_handle, surface.m_handle, &count, formats.data()
    );
    if (VK_SUCCESS != result) { assert(false); }
    return formats;
}

auto PhysicalDevice::query_surface_present_modes(const Surface& surface) const
    -> std::vector<VkPresentModeKHR> {
    u32      count = 0;
    VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_handle, surface.m_handle, &count, nullptr
    );
    if (VK_SUCCESS != result || (count == 0)) { assert(false); }

    std::vector<VkPresentModeKHR> present_modes(count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_handle, surface.m_handle, &count, present_modes.data()
    );
    if (VK_SUCCESS != result) { assert(false); }
    return present_modes;
}

auto PhysicalDevice::query_memory_properties() const -> VkPhysicalDeviceMemoryProperties {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(m_handle, &memory_properties);
    return memory_properties;
}

auto PhysicalDevice::query_properties() const -> VkPhysicalDeviceProperties {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(m_handle, &properties);
    return properties;
}

auto PhysicalDevice::query_features() const -> VkPhysicalDeviceFeatures {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(m_handle, &features);
    return features;
}

auto PhysicalDevice::query_extension_properties() const
    -> std::vector<VkExtensionProperties> {
    std::vector<VkExtensionProperties> extension_properties;

    u32 count = 0;

    VkResult result =
        vkEnumerateDeviceExtensionProperties(m_handle, nullptr, &count, nullptr);
    extension_properties.resize(count);
    result = vkEnumerateDeviceExtensionProperties(
        m_handle, nullptr, &count, extension_properties.data()
    );
    if (VK_SUCCESS != result) { assert(false); }
    return extension_properties;
}

static auto to_format_string_queue_family(const QueueFamily& queue_family
) -> std::string {
    std::string result = "{ ";
    auto        flags = queue_family.m_properties.queueFlags;
    if (bit::check_flag(flags, VK_QUEUE_GRAPHICS_BIT)) { result += "Graphics "; }
    if (bit::check_flag(flags, VK_QUEUE_COMPUTE_BIT)) { result += "Compute "; }
    if (bit::check_flag(flags, VK_QUEUE_TRANSFER_BIT)) { result += "Transfer "; }
    if (bit::check_flag(flags, VK_QUEUE_SPARSE_BINDING_BIT)) {
        result += "SparseBinding ";
    }
    if (bit::check_flag(flags, VK_QUEUE_PROTECTED_BIT)) { result += "Protected "; }
    result += "}";
    return result;
}

static auto log_physical_device(PhysicalDevice& pd) -> void {
    Logger::info("Physical device: {}", pd.m_properties.deviceName);
    Logger::info("\tDevice Type: {}", to_string(pd.m_properties.deviceType));
    Logger::info(
        "\tVendor (ID): {} ({})",
        to_string_vendor_id(pd.m_properties.vendorID),
        pd.m_properties.vendorID
    );
    Logger::info("\tDevice ID: {}", pd.m_properties.deviceID);
    Logger::info("\tDriver Version: {}", pd.m_properties.driverVersion);
    Logger::info(
        "\tVulkan Api Version: {}.{}.{}",
        VK_VERSION_MAJOR(pd.m_properties.apiVersion),
        VK_VERSION_MINOR(pd.m_properties.apiVersion),
        VK_VERSION_PATCH(pd.m_properties.apiVersion)
    );
    Logger::info(
        "\tPipeline Cache UUID: {}", to_string(pd.m_properties.pipelineCacheUUID)
    );

    Logger::info("\tlimits:");
    Logger::info(
        "\t\tmax image dimension 1D: {}", pd.m_properties.limits.maxImageDimension1D
    );
    Logger::info(
        "\t\tmax image dimension 2D: {}", pd.m_properties.limits.maxImageDimension2D
    );
    Logger::info(
        "\t\tmax image dimension 3D: {}", pd.m_properties.limits.maxImageDimension3D
    );
    Logger::info(
        "\t\tvertex input bindings: {}", pd.m_properties.limits.maxVertexInputBindings
    );
    Logger::info(
        "\t\tvertex input attributes: {}", pd.m_properties.limits.maxVertexInputAttributes
    );
    Logger::info(
        "\t\tvertex input attribute offset: {}",
        pd.m_properties.limits.maxVertexInputAttributeOffset
    );
    // push constants
    Logger::info(
        "\t\tpush constant size: {}", pd.m_properties.limits.maxPushConstantsSize
    );

    // max descriptor set uniform buffers
    Logger::info(
        "\t\tmax descriptor set uniform buffers: {}",
        pd.m_properties.limits.maxDescriptorSetUniformBuffers
    );
    // max descriptor set uniform buffers dynamic
    Logger::info(
        "\t\tmax descriptor set uniform buffers dynamic: {}",
        pd.m_properties.limits.maxDescriptorSetUniformBuffersDynamic
    );
    // max descriptor set storage buffers
    Logger::info(
        "\t\tmax descriptor set storage buffers: {}",
        pd.m_properties.limits.maxDescriptorSetStorageBuffers
    );
    // max descriptor set storage buffers dynamic
    Logger::info(
        "\t\tmax descriptor set storage buffers dynamic: {}",
        pd.m_properties.limits.maxDescriptorSetStorageBuffersDynamic
    );
    // max descriptor set samplers
    Logger::info(
        "\t\tmax descriptor set samplers: {}",
        pd.m_properties.limits.maxDescriptorSetSamplers
    );
    // max descriptor set sampled images
    Logger::info(
        "\t\tmax descriptor set sampled images: {}",
        pd.m_properties.limits.maxDescriptorSetSampledImages
    );
    // max descriptor set storage images
    Logger::info(
        "\t\tmax descriptor set storage images: {}",
        pd.m_properties.limits.maxDescriptorSetStorageImages
    );
    // // max descriptor set combined images samplers
    // Logger::info(
    //     "\t\tmax descriptor set combined images samplers: {}",
    //     pd.m_properties.limits.maxDescriptorSetCombinedImageSamplers
    // );
    // max descriptor set input attachments
    Logger::info(
        "\t\tmax descriptor set input attachments: {}",
        pd.m_properties.limits.maxDescriptorSetInputAttachments
    );

    // Logger::info(
    //     "\t\tmax vertex input attribute stride: {}",
    //     pd.m_properties.limits.maxVertexInputAttributeStride
    // );
    Logger::info(
        "\t{} memory types from {}:",
        pd.m_memory_properties.memoryTypeCount,
        VK_MAX_MEMORY_TYPES
    );
    for (u32 i = 0; i < pd.m_memory_properties.memoryTypeCount; ++i) {
        VkMemoryType memory_type = pd.m_memory_properties.memoryTypes[i];
        Logger::info("\t\t{}: {} {}", i, memory_type.heapIndex, to_string(memory_type));
    }
    Logger::info(
        "\t{} memory heaps from {}:",
        pd.m_memory_properties.memoryHeapCount,
        VK_MAX_MEMORY_HEAPS
    );
    for (u32 i = 0; i < pd.m_memory_properties.memoryHeapCount; ++i) {
        VkMemoryHeap memory_heap = pd.m_memory_properties.memoryHeaps[i];
        Logger::info(
            "\t\t{}: {} with {} bytes", i, to_string(memory_heap), memory_heap.size
        );
    }
    for (size_t i = 0; i < pd.m_queue_families.size(); i++) {
        QueueFamily& queue_family = pd.m_queue_families[i];
        Logger::debug(
            "Queue family {} has {} queues capable of {}",
            i,
            queue_family.m_properties.queueCount,
            to_format_string_queue_family(queue_family)
        );
    }
}

auto PhysicalDevice::init(Instance& instance) -> void {
    m_instance_p = &instance;
    m_properties = this->query_properties();
    m_features = this->query_features();
    m_memory_properties = this->query_memory_properties();

    /*
        Quick Vulkan Queue Family Guide:
        - Graphics: Used for rendering. (VK_QUEUE_GRAPHICS_BIT)
        - Compute: Used for general purpose computation. (VK_QUEUE_COMPUTE_BIT)
        - Transfer: Used for data transfer operations. (VK_QUEUE_TRANSFER_BIT)
        - Sparse Binding: Used for sparse resources. (VK_QUEUE_SPARSE_BINDING_BIT)
        - Protected: Used for protected memory. (VK_QUEUE_PROTECTED_BIT)
        - Present: Used for presenting to a surface.

        Our GPU/Physical device has to be able to at least do graphics and presenting.

    */
    m_queue_families = this->query_queue_families();
    m_extension_properties = this->query_extension_properties();
    m_extension_support = this->check_extension_support(m_device_extensions);

    m_chosen_queue_family_pointers =
        PhysicalDevice::choose_fitting_queue_families(m_queue_families);

    { log_physical_device(*this); }
}

auto PhysicalDevice::check_extension_support(const std::vector<const char*>& extensions
) const -> bool {
    std::set<std::string> required_extensions(extensions.begin(), extensions.end());
    for (u32 i = 0; i < m_extension_properties.size(); i++) {
        required_extensions.erase(m_extension_properties[i].extensionName);
    }
    return required_extensions.empty();
}

// /// Returns a pair of queue family indices, one that supports graphics and one that
// /// supports presenting.
auto PhysicalDevice::choose_fitting_queue_families(
    std::vector<QueueFamily>& queue_families
) -> QueueFamilyPointers {
    // TODO: Make it more efficient. As of right now we simply find the first queue
    // family which does the job.

    QueueFamilyPointers fam_pointers;
    for (u32 i = 0; i < queue_families.size(); i++) {
        auto& family = queue_families[i];

        const bool is_graphics = family.supports_graphics();
        if (is_graphics) { fam_pointers.m_graphics_family = &family; }
    }
    for (u32 i = 0; i < queue_families.size(); i++) {
        auto& family = queue_families[i];

        const bool is_compute = family.supports_compute();
        if (is_compute) { fam_pointers.m_compute_family = &family; }
    }

    for (u32 i = 0; i < queue_families.size(); i++) {
        auto& family = queue_families[i];

        const bool is_transfer = family.supports_compute();
        if (is_transfer) { fam_pointers.m_transfer_family = &family; }
    }

    return fam_pointers;
}

auto PhysicalDevice::find_memory_type(u32 type_filter, VkMemoryPropertyFlags properties)
    const -> u32 {
    const VkPhysicalDeviceMemoryProperties& mem_props = m_memory_properties;
    for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
        if (bit::check(type_filter, i) &&
            (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

auto PhysicalDevice::query_queue_families() const -> std::vector<QueueFamily> {
    std::vector<VkQueueFamilyProperties> properties =
        this->query_queue_family_properties();

    std::vector<QueueFamily> families;
    families.resize(properties.size());
    for (u32 i = 0; i < properties.size(); i++) {
        families[i].m_index = i;
        families[i].m_properties = properties[i];
        families[i].m_physical_device = this;
    }

    return families;
}

auto PhysicalDevice::limits() const -> VkPhysicalDeviceLimits {
    return m_properties.limits;
}

auto PhysicalDevice::create_logical_device() -> LogicalDevice {
    LogicalDevice ld;
    ld.init(*m_instance_p, *this);
    return ld;
}

auto PhysicalDevice::find_depth_format() const -> VkFormat {
    std::vector<VkFormat> depth_formats = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_D16_UNORM_S8_UINT,
    };

    for (u32 i = 0; i < depth_formats.size(); i++) {
        auto& format = depth_formats[i];

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_handle, format, &props);
        if (props.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }
    return VK_FORMAT_UNDEFINED;
}
} // namespace vulkan
} // namespace JadeFrame