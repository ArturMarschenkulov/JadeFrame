#include "pch.h"
#include "vulkan_pipeline.h"
#include "vulkan_logical_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_descriptor_set_layout.h"
#include "vulkan_render_pass.h"

#include "../to_spirv.h"

#include <array>


namespace JadeFrame {

static const char* vs_ =
R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}
)";

static const char* fs_ =
R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)";

static auto create_shader_module_from_spirv(VkDevice device, const std::vector<u32>& spirv) -> VkShaderModule {
	VkResult result;
	const VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.codeSize = spirv.size() * 4, // NOTE: It has to be multiplied by 4!
		.pCode = spirv.data(),
	};

	VkShaderModule shader_module;
	result = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
	if (result != VK_SUCCESS) __debugbreak();
	return shader_module;
}


auto VulkanPipeline::init(
	const VulkanLogicalDevice& device, 
	const VulkanSwapchain& swapchain,
	const VulkanDescriptorSetLayout& descriptor_set_layout,
	const VulkanRenderPass& render_pass,
	const GLSLCode& code) -> void {

	VkResult result;
	//auto tm = &JadeFrame::get_singleton()->m_apps[0]->m_time_manager;
	//auto time_0 = tm->get_time();

	if (m_is_compiled == false) {
		std::future<std::vector<u32>> vert_shader_spirv = std::async(std::launch::async, string_to_SPIRV, code.m_vertex_shader.c_str(), 0);
		std::future<std::vector<u32>> frag_shader_spirv = std::async(std::launch::async, string_to_SPIRV, code.m_fragment_shader.c_str(), 1);

		m_vert_shader_spirv = vert_shader_spirv.get();
		m_frag_shader_spirv = frag_shader_spirv.get();

		m_is_compiled = true;
	}

	VkShaderModule vert_shader_module = create_shader_module_from_spirv(device.m_handle, m_vert_shader_spirv);
	VkShaderModule frag_shader_module = create_shader_module_from_spirv(device.m_handle, m_frag_shader_spirv);



	//auto time_1 = tm->get_time();
	//std::cout << "it took " << time_1 - time_0 << std::endl;

	const VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vert_shader_module,
		.pName = "main",
		.pSpecializationInfo = nullptr,
	};

	const VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = frag_shader_module,
		.pName = "main",
		.pSpecializationInfo = nullptr,
	};

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		vert_shader_stage_info,
		frag_shader_stage_info,
	};

	VkVertexInputBindingDescription binding_description = VVertex::get_binding_description();
	std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = VVertex::get_attribute_descriptions();

	const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding_description,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
		.pVertexAttributeDescriptions = attribute_descriptions.data(),
	};

	const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	const VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<f32>(swapchain.m_extent.width),
		.height = static_cast<f32>(swapchain.m_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = swapchain.m_extent,
	};

	const VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = {},
		.depthBiasClamp = {},
		.depthBiasSlopeFactor = {},
		.lineWidth = 1.0f,
	};

	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = {},
		.pSampleMask = {},
		.alphaToCoverageEnable = {},
		.alphaToOneEnable = {},
	};

	const VkPipelineColorBlendAttachmentState color_blend_attachment = {
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = {},
		.dstColorBlendFactor = {},
		.colorBlendOp = {},
		.srcAlphaBlendFactor = {},
		.dstAlphaBlendFactor = {},
		.alphaBlendOp = {},
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};

	const VkPipelineColorBlendStateCreateInfo color_blending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
		.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
	};

	const VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_set_layout.m_handle,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = {},
	};

	result = vkCreatePipelineLayout(device.m_handle, &pipeline_layout_info, nullptr, &m_pipeline_layout);
	if (result != VK_SUCCESS) __debugbreak();

	const VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = {},
		.flags = {},
		.stageCount = 2,
		.pStages = shader_stages,
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly,
		.pTessellationState = {},
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = {},
		.pColorBlendState = &color_blending,
		.pDynamicState = {},
		.layout = m_pipeline_layout,
		.renderPass = render_pass.m_handle,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = {},
	};

	result = vkCreateGraphicsPipelines(device.m_handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline);
	if (result != VK_SUCCESS) __debugbreak();

	vkDestroyShaderModule(device.m_handle, frag_shader_module, nullptr);
	vkDestroyShaderModule(device.m_handle, vert_shader_module, nullptr);
	m_device = &device;
}

auto VulkanPipeline::deinit() -> void {
	vkDestroyPipeline(m_device->m_handle, m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device->m_handle, m_pipeline_layout, nullptr);
}

auto VulkanPipeline::operator=(const VulkanPipeline& o) {
	//m_vert_shader_spirv = std::move(o.m_vert_shader_spirv);
	//m_frag_shader_spirv = o.m_frag_shader_spirv;
}

}