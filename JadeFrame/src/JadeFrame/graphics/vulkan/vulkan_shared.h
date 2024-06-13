#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <span>
#include "JadeFrame/math/mat_4.h"
#include "../mesh.h"
#include "../graphics_shared.h"
#include "JadeFrame/utils/assert.h"

// TODO: Look whether this file is needed. This is file was mainly created as a quick fix
// for some globals

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
// auto get_attribute_descriptions(Mesh m) -> std::vector<
// VkVertexInputAttributeDescription> {
//	return {};
// }

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
        default: JF_UNIMPLEMENTED();
    }

    return result;
}

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
    const char* str = "";
#define foo(x)                                                                           \
    case x: str = &#x[sizeof("VK_DESCRIPTOR_TYPE_") - 1];
    switch (type) {
        foo(VK_DESCRIPTOR_TYPE_SAMPLER);
        foo(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        foo(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
        foo(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
        foo(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
        foo(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
        foo(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        foo(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
        foo(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
        foo(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
        foo(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        foo(VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT);
        foo(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
        default: return "UNKNOWN";
    }
#undef foo
    return str;
}

inline auto to_string(const VkResult& result) -> const char* {
    const char* str = "";
#define foo(x)                                                                           \
    case x: str = &#x[sizeof("VK_") - 1];
    switch (result) {
        foo(VK_SUCCESS);
        foo(VK_NOT_READY);
        foo(VK_TIMEOUT);
        foo(VK_EVENT_SET);
        foo(VK_EVENT_RESET);
        foo(VK_INCOMPLETE);
        foo(VK_SUBOPTIMAL_KHR);
        foo(VK_ERROR_OUT_OF_DATE_KHR);
        foo(VK_ERROR_OUT_OF_HOST_MEMORY);
        foo(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        foo(VK_ERROR_INITIALIZATION_FAILED);
        foo(VK_ERROR_DEVICE_LOST);
        foo(VK_ERROR_MEMORY_MAP_FAILED);
        foo(VK_ERROR_LAYER_NOT_PRESENT);
        foo(VK_ERROR_EXTENSION_NOT_PRESENT);
        foo(VK_ERROR_FEATURE_NOT_PRESENT);
        foo(VK_ERROR_INCOMPATIBLE_DRIVER);
        foo(VK_ERROR_TOO_MANY_OBJECTS);
        foo(VK_ERROR_FORMAT_NOT_SUPPORTED);
        foo(VK_ERROR_FRAGMENTED_POOL);
        foo(VK_ERROR_SURFACE_LOST_KHR);
        foo(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
        foo(VK_ERROR_OUT_OF_POOL_MEMORY);
        // foo(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
        default:
            Logger::err("Unknown VkResult: {}", result);
            assert(false);
            str = "";
            break;
    }
#undef foo
    return str;
}

inline auto to_string_from_shader_stage_flags(const VkShaderStageFlags& flags)
    -> std::string {
    std::string result;
    result += "{ ";
    if (flags & VK_SHADER_STAGE_VERTEX_BIT) { result += "VERTEX "; }
    if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
        result += "TESSELLATION_CONTROL ";
    }
    if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
        result += "TESSELLATION_EVALUATION ";
    }
    if (flags & VK_SHADER_STAGE_GEOMETRY_BIT) { result += "GEOMETRY "; }
    if (flags & VK_SHADER_STAGE_FRAGMENT_BIT) { result += "FRAGMENT "; }
    if (flags & VK_SHADER_STAGE_COMPUTE_BIT) { result += "COMPUTE "; }
    if (flags & VK_SHADER_STAGE_ALL_GRAPHICS) { result += "ALL_GRAPHICS "; }
    if (flags & VK_SHADER_STAGE_ALL) { result += "ALL "; }

    if (flags & VK_SHADER_STAGE_RAYGEN_BIT_KHR) { result += "RAYGEN_KHR "; }
    if (flags & VK_SHADER_STAGE_ANY_HIT_BIT_KHR) { result += "ANY_HIT_KHR "; }
    if (flags & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) { result += "CLOSEST_HIT_KHR "; }
    if (flags & VK_SHADER_STAGE_MISS_BIT_KHR) { result += "MISS_KHR "; }
    if (flags & VK_SHADER_STAGE_INTERSECTION_BIT_KHR) { result += "INTERSECTION_KHR "; }
    if (flags & VK_SHADER_STAGE_CALLABLE_BIT_KHR) { result += "CALLABLE_KHR "; }

    if (flags & VK_SHADER_STAGE_TASK_BIT_NV) { result += "TASK_KHR "; }
    if (flags & VK_SHADER_STAGE_MESH_BIT_NV) { result += "MESH_KHR "; }
    if (flags & VK_SHADER_STAGE_RAYGEN_BIT_NV) { result += "RAYGEN_KHR "; }
    if (flags & VK_SHADER_STAGE_ANY_HIT_BIT_NV) { result += "ANY_HIT_KHR "; }
    if (flags & VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV) { result += "CLOSEST_HIT_KHR "; }
    if (flags & VK_SHADER_STAGE_MISS_BIT_NV) { result += "MISS_KHR "; }
    result += "}";
    return result;
}

inline auto to_string(const VkMemoryHeap& memory_heap) -> std::string {
    std::string result = "{ ";
    if (memory_heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) { result += "DeviceLocal "; }
    if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
        result += "MultiInstance ";
    }
    if (memory_heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR) {
        result += "MultiInstance_KHR ";
    }
    result += "}";
    return result;
}

inline auto to_string(const VkPhysicalDeviceType& device_type) -> const char* {
    const char* result = "";
#define foo(x)                                                                           \
    case x: result = &#x[sizeof("VK_PHYSICAL_DEVICE_TYPE_") - 1]; break
    switch (device_type) {
        foo(VK_PHYSICAL_DEVICE_TYPE_OTHER);
        foo(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
        foo(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        foo(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
        foo(VK_PHYSICAL_DEVICE_TYPE_CPU);
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            JF_ASSERT(false, "");
            result = "";
            break;
    }
#undef foo
    return result;
}

inline auto to_string(const VkMemoryType& memory_type) -> std::string {
    std::string result = "{ ";
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        result += "DeviceLocal ";
    }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        result += "HostVisible ";
    }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        result += "HostCoherent ";
    }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) {
        result += "HostCached ";
    }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
        result += "LazilyAllocated ";
    }
    if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
        result += "Protected ";
    }
    // AMD Specific
    // if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
    // result += "DeviceCoherent "; } if (memory_type.propertyFlags &
    // VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) { result += "DeviceUncached "; } NV
    // Specific if (memory_type.propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV ) {
    // result += "Protected
    // "; }
    result += "}";
    return result;
}
} // namespace vulkan
} // namespace JadeFrame