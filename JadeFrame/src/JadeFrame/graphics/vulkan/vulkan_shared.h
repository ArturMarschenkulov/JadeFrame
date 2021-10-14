#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include "JadeFrame/math/mat_4.h"
#include "../Mesh.h"
#include "../graphics_shared.h"

//TODO: Look whether this file is needed. This is file was mainly created as a quick fix for some globals

namespace JadeFrame {


//auto get_binding_description(Mesh m) -> VkVertexInputBindingDescription {
//	VkVertexInputBindingDescription binding_description = {};
//	binding_description.binding = 0;
//	binding_description.stride = sizeof(Mesh);
//	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//	return binding_description;
//}
//
//auto get_attribute_descriptions(Mesh m) -> std::vector< VkVertexInputAttributeDescription> {
//	return {};
//}


inline auto get_binding_description(const VertexFormat& vertex_format) -> VkVertexInputBindingDescription{
	u32 stride = 0;
	for(const VertexAttribute& attribute : vertex_format.m_attributes) {
		stride += attribute.size;
	}

	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = stride;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_description;
};
inline auto SHADER_TYPE_to_VkFormat(const SHADER_TYPE& shader_type) ->VkFormat {
	VkFormat result;

	switch(shader_type) {
		case SHADER_TYPE::FLOAT_2:
		{
			result = VK_FORMAT_R32G32_SFLOAT;
		} break;
		case SHADER_TYPE::FLOAT_3:
		{
			result = VK_FORMAT_R32G32B32_SFLOAT;
		} break;
		case SHADER_TYPE::FLOAT_4:
		{
			result = VK_FORMAT_R32G32B32A32_SFLOAT;
		} break;
		default: __debugbreak();
	}

	return result;
}

inline auto get_attribute_descriptions(const VertexFormat& vertex_format) -> std::vector<VkVertexInputAttributeDescription> {
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
	for(u32 i = 0; i < vertex_format.m_attributes.size(); i++) {
		VkVertexInputAttributeDescription attribute_description;
		attribute_description.binding = 0;
		attribute_description.location = i;
		attribute_description.format = SHADER_TYPE_to_VkFormat(vertex_format.m_attributes[i].type);
		attribute_description.offset = vertex_format.m_attributes[i].offset;
		attribute_descriptions.push_back(attribute_description);
	}

	return attribute_descriptions;
};

struct UniformBufferObject {
	Matrix4x4 view_projection;
	//Matrix4x4 model;
};

struct UniformBufferObject_bkp {
	Matrix4x4 model;
	Matrix4x4 view;
	Matrix4x4 proj;
};
}