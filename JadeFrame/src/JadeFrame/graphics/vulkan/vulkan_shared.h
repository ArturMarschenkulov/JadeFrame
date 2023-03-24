#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include "JadeFrame/math/mat_4.h"
#include "../mesh.h"
#include "../graphics_shared.h"
#include "JadeFrame/utils/assert.h"

// TODO: Look whether this file is needed. This is file was mainly created as a quick fix for some globals

namespace JadeFrame {
namespace vulkan {

// auto get_binding_description(Mesh m) -> VkVertexInputBindingDescription {
//	VkVertexInputBindingDescription binding_description = {};
//	binding_description.binding = 0;
//	binding_description.stride = sizeof(Mesh);
//	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//	return binding_description;
// }
//
// auto get_attribute_descriptions(Mesh m) -> std::vector< VkVertexInputAttributeDescription> {
//	return {};
// }


inline auto get_binding_description(const VertexFormat& vertex_format) -> VkVertexInputBindingDescription {
    u32 stride = 0;
    for (const VertexAttribute& attribute : vertex_format.m_attributes) { stride += attribute.size; }

    const VkVertexInputBindingDescription binding_description = {
        .binding = 0,
        .stride = stride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return binding_description;
};
inline auto SHADER_TYPE_to_VkFormat(const SHADER_TYPE& shader_type) -> VkFormat {
    VkFormat result = VK_FORMAT_UNDEFINED;

    switch (shader_type) {
        case SHADER_TYPE::V_2_F32: {
            result = VK_FORMAT_R32G32_SFLOAT;
        } break;
        case SHADER_TYPE::V_3_F32: {
            result = VK_FORMAT_R32G32B32_SFLOAT;
        } break;
        case SHADER_TYPE::V_4_F32: {
            result = VK_FORMAT_R32G32B32A32_SFLOAT;
        } break;
        default: JF_ASSERT(false, "not implemented yet!");
    }

    return result;
}

inline auto get_attribute_descriptions(const VertexFormat& vertex_format)
    -> std::vector<VkVertexInputAttributeDescription> {
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    attribute_descriptions.resize(vertex_format.m_attributes.size());
    for (u32 i = 0; i < vertex_format.m_attributes.size(); i++) {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].format = SHADER_TYPE_to_VkFormat(vertex_format.m_attributes[i].type);
        attribute_descriptions[i].offset = static_cast<u32>(vertex_format.m_attributes[i].offset);
    }

    return attribute_descriptions;
};

struct UniformBufferObject {
    Matrix4x4 view_projection;
    // Matrix4x4 model;
};

struct UniformBufferObject_bkp {
    Matrix4x4 model;
    Matrix4x4 view;
    Matrix4x4 proj;
};
inline auto to_string(const VkDescriptorType& type) -> const char* {
    switch (type) {
        case VK_DESCRIPTOR_TYPE_SAMPLER: return "SAMPLER";
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return "COMBINED_IMAGE_SAMPLER";
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return "SAMPLED_IMAGE";
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return "STORAGE_IMAGE";
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return "UNIFORM_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return "STORAGE_TEXEL_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return "UNIFORM_BUFFER";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return "STORAGE_BUFFER";
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return "UNIFORM_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return "STORAGE_BUFFER_DYNAMIC";
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return "INPUT_ATTACHMENT";
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT: return "INLINE_UNIFORM_BLOCK_EXT";
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return "ACCELERATION_STRUCTURE_KHR";
        default: return "UNKNOWN";
    }
}
inline auto to_string(const VkResult& result) -> std::string {
    std::string str;
    switch (result) {
        case VK_SUCCESS: str = "VK_SUCCESS"; break;
        case VK_NOT_READY: str = "VK_NOT_READY"; break;
        case VK_TIMEOUT: str = "VK_TIMEOUT"; break;
        case VK_EVENT_SET: str = "VK_EVENT_SET"; break;
        case VK_EVENT_RESET: str = "VK_EVENT_RESET"; break;
        case VK_INCOMPLETE: str = "VK_INCOMPLETE"; break;
        case VK_SUBOPTIMAL_KHR: str = "VK_SUBOPTIMAL_KHR"; break;
        case VK_ERROR_OUT_OF_DATE_KHR: str = "VK_ERROR_OUT_OF_DATE_KHR"; break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: str = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: str = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
        case VK_ERROR_INITIALIZATION_FAILED: str = "VK_ERROR_INITIALIZATION_FAILED"; break;
        case VK_ERROR_DEVICE_LOST: str = "VK_ERROR_DEVICE_LOST"; break;
        case VK_ERROR_MEMORY_MAP_FAILED: str = "VK_ERROR_MEMORY_MAP_FAILED"; break;
        case VK_ERROR_LAYER_NOT_PRESENT: str = "VK_ERROR_LAYER_NOT_PRESENT"; break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: str = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
        case VK_ERROR_FEATURE_NOT_PRESENT: str = "VK_ERROR_FEATURE_NOT_PRESENT"; break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: str = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
        case VK_ERROR_TOO_MANY_OBJECTS: str = "VK_ERROR_TOO_MANY_OBJECTS"; break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED: str = "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
        case VK_ERROR_FRAGMENTED_POOL: str = "VK_ERROR_FRAGMENTED_POOL"; break;
        case VK_ERROR_SURFACE_LOST_KHR: str = "VK_ERROR_SURFACE_LOST_KHR"; break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: str = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"; break;
        case VK_ERROR_OUT_OF_POOL_MEMORY: str = "VK_ERROR_OUT_OF_POOL_MEMORY"; break;
        // case VK_ERROR_OUT_OF_POOL_MEMORY_KHR: str = "VK_ERROR_OUT_OF_POOL_MEMORY_KHR"; break;
        default:
            Logger::err("Unknown VkResult: {}", result);
            assert(false);
            str = "";
            break;
    }
    return str;
}

inline auto to_string_from_shader_stage_flags(const VkShaderStageFlags& flags) -> std::string {
    std::string result;
    result += "{ ";
    if (flags & VK_SHADER_STAGE_VERTEX_BIT) result += "VERTEX ";
    if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) result += "TESSELLATION_CONTROL ";
    if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) result += "TESSELLATION_EVALUATION ";
    if (flags & VK_SHADER_STAGE_GEOMETRY_BIT) result += "GEOMETRY ";
    if (flags & VK_SHADER_STAGE_FRAGMENT_BIT) result += "FRAGMENT ";
    if (flags & VK_SHADER_STAGE_COMPUTE_BIT) result += "COMPUTE ";
    if (flags & VK_SHADER_STAGE_ALL_GRAPHICS) result += "ALL_GRAPHICS ";
    if (flags & VK_SHADER_STAGE_ALL) result += "ALL ";

    if (flags & VK_SHADER_STAGE_RAYGEN_BIT_KHR) result += "RAYGEN_KHR ";
    if (flags & VK_SHADER_STAGE_ANY_HIT_BIT_KHR) result += "ANY_HIT_KHR ";
    if (flags & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) result += "CLOSEST_HIT_KHR ";
    if (flags & VK_SHADER_STAGE_MISS_BIT_KHR) result += "MISS_KHR ";
    if (flags & VK_SHADER_STAGE_INTERSECTION_BIT_KHR) result += "INTERSECTION_KHR ";
    if (flags & VK_SHADER_STAGE_CALLABLE_BIT_KHR) result += "CALLABLE_KHR ";

    if (flags & VK_SHADER_STAGE_TASK_BIT_NV) result += "TASK_KHR ";
    if (flags & VK_SHADER_STAGE_MESH_BIT_NV) result += "MESH_KHR ";
    if (flags & VK_SHADER_STAGE_RAYGEN_BIT_NV) result += "RAYGEN_KHR ";
    if (flags & VK_SHADER_STAGE_ANY_HIT_BIT_NV) result += "ANY_HIT_KHR ";
    if (flags & VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV) result += "CLOSEST_HIT_KHR ";
    if (flags & VK_SHADER_STAGE_MISS_BIT_NV) result += "MISS_KHR ";
    result += "}";
    return result;
}

inline auto to_string(const VkMemoryHeap& memory_heap) -> std::string {
    std::string result = "{ ";
    if (memory_heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) { result += "DeviceLocal "; }
    if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) { result += "MultiInstance "; }
    if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR) { result += "MultiInstance_KHR "; }
    result += "}";
    return result;
}
inline auto to_string(const VkPhysicalDeviceType& device_type) -> std::string {
    switch (device_type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER: return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        default: return "Unknown";
    }
}
inline auto to_string(const VkMemoryType& memory_type) -> std::string {
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
} // namespace vulkan
} // namespace JadeFrame