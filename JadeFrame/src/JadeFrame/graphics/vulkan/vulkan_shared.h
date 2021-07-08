#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include "JadeFrame/math/vec_2.h"
#include "JadeFrame/math/vec_3.h"
#include "JadeFrame/math/mat_4.h"


//TODO: Look whether this file is needed. This is file was mainly created as a quick fix for some globals

namespace JadeFrame {

struct VVertex {
	Vec2 pos;
	Vec3 color;

	static auto get_binding_description() -> VkVertexInputBindingDescription {
		VkVertexInputBindingDescription binding_description = {};
		binding_description.binding = 0;
		binding_description.stride = sizeof(VVertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_description;
	}

	static auto get_attribute_descriptions()->std::array<VkVertexInputAttributeDescription, 2> {
		std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = {};

		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(VVertex, pos);

		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(VVertex, color);

		return attribute_descriptions;
	}
};

struct UniformBufferObject {
	Matrix4x4 model;
	Matrix4x4 view;
	Matrix4x4 proj;
};
}