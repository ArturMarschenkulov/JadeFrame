#include "pch.h"
#include "vulkan_pipeline.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_descriptor_set.h"

#include "extern/SPIRV-Cross/spirv_glsl.hpp"
#include "extern/SPIRV-Cross/spirv_hlsl.hpp"
#include "extern/SPIRV-Cross/spirv_msl.hpp"


#include <array>


namespace JadeFrame {


static auto create_shader_module_from_spirv(VkDevice device, const std::vector<u32>& spirv) -> VkShaderModule {
	VkResult result;
	const VkShaderModuleCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = spirv.size() * 4, // NOTE: It has to be multiplied by 4!
		.pCode = spirv.data(),
	};

	VkShaderModule shader_module;
	result = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
	if (result != VK_SUCCESS) __debugbreak();
	return shader_module;
}

static auto debug_print_resources(const spirv_cross::ShaderResources& resources) -> void {

	for (const spirv_cross::Resource& resource : resources.uniform_buffers) {
		const const std::string& name = resource.name;
		Logger::log("uniform_buffers {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.storage_buffers) {
		const const std::string& name = resource.name;
		Logger::log("storage_buffers {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.stage_inputs) {
		const const std::string& name = resource.name;
		Logger::log("stage_inputs {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.stage_outputs) {
		const const std::string& name = resource.name;
		Logger::log("stage_outputs {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.subpass_inputs) {
		const const std::string& name = resource.name;
		Logger::log("subpass_inputs {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.storage_images) {
		const const std::string& name = resource.name;
		Logger::log("storage_images {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.sampled_images) {
		const const std::string& name = resource.name;
		Logger::log("sampled_images {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.atomic_counters) {
		const const std::string& name = resource.name;
		Logger::log("atomic_counters {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.acceleration_structures) {
		const const std::string& name = resource.name;
		Logger::log("acceleration_structures {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.push_constant_buffers) {
		const const std::string& name = resource.name;
		Logger::log("push_constant_buffers {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.separate_images) {
		const const std::string& name = resource.name;
		Logger::log("separate_images {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.separate_samplers) {
		const const std::string& name = resource.name;
		Logger::log("separate_samplers {}", name);
	}

	for (const spirv_cross::BuiltInResource& resource : resources.builtin_inputs) {
		const const std::string& name = resource.resource.name;
		Logger::log("builtin_inputs {}", name);
	}
	for (const spirv_cross::BuiltInResource& resource : resources.builtin_outputs) {
		const const std::string& name = resource.resource.name;
		Logger::log("builtin_outputs {}", name);
	}
}

static auto from_SHADER_STAGE(SHADER_STAGE stage) -> VkShaderStageFlagBits {
	VkShaderStageFlagBits result;
	switch (stage) {
		case SHADER_STAGE::VERTEX:
		{
			result = VK_SHADER_STAGE_VERTEX_BIT;
		} break;
		case SHADER_STAGE::FRAGMENT:
		{
			result = VK_SHADER_STAGE_FRAGMENT_BIT;
		} break;
		default: __debugbreak();
	}
	return result;
}

struct UB {
	std::string name;
	u32 binding;
	u32 set;
	u32 size;
};
static std::map<u32, UB> s_uniform_buffers;
static std::map<u32, std::map<u32, VulkanDescriptor>> s_descriptors;
static std::map<u32, std::map<i32, f32>> ss;

static auto add_uniform_buffer(const std::map<u32, UB>& map, const UB& uniform) {
	ss.contains(2_u32);
}

auto VulkanPipeline::init(
	const VulkanLogicalDevice& device,
	const VkExtent2D& extent,
	const VulkanDescriptorSetLayout& descriptor_set_layout,
	const VulkanRenderPass& render_pass,
	const ShadingCode& code,
	const VertexFormat& vertex_format) -> void {
	m_device = &device;
	m_render_pass = &render_pass;
	m_descriptor_set_layout = &descriptor_set_layout;

	VkResult result;

	std::vector<std::future<std::vector<u32>>> spirvs;
	spirvs.resize(code.m_modules.size());
	for (u32 i = 0; i < code.m_modules.size(); i++) {
		const std::string& str = std::get<std::string>(code.m_modules[i].m_code);
		spirvs[i] = std::async(std::launch::async, string_to_SPIRV, str, code.m_modules[i].m_stage);
	}

	m_code.m_modules.resize(spirvs.size());
	m_code.m_modules[0].m_stage = SHADER_STAGE::VERTEX;
	m_code.m_modules[0].m_code = spirvs[0].get();
	m_code.m_modules[1].m_stage = SHADER_STAGE::FRAGMENT;
	m_code.m_modules[1].m_code = spirvs[1].get();


	{
		//std::vector cs = { m_vert_shader_spirv, m_frag_shader_spirv };

		for (u32 i = 0; i < m_code.m_modules.size(); i++) {
			auto& current_module = m_code.m_modules[i];
			auto& current_module_code = std::get<std::vector<u32>>(current_module.m_code);
			spirv_cross::Compiler compiler(current_module_code);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();

			debug_print_resources(resources);



			for (const spirv_cross::Resource& resource : resources.uniform_buffers) {
				const const std::string& name = resource.name;
				const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
				i32 member_count = static_cast<u32>(buffer_type.member_types.size());
				u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				u32 size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));

				const UB ub = {
					.name = name,
					.binding = binding,
					.set = descriptor_set,
					.size = size,
				};

				if (s_uniform_buffers.contains(ub.binding) == false) {
					s_uniform_buffers.insert({ ub.binding, ub });
				} else {
					//__debugbreak();
				}

				VulkanDescriptor vd = {};
				//if (false == s_descriptors.contains(ub.set)) 
				{
					if (false == s_descriptors[ub.set].contains(ub.binding)) {
						s_descriptors[ub.set][ub.binding] = vd;
						Logger::log("{}.{}", ub.set, ub.binding);
					}
				}
			}

			for (const spirv_cross::Resource& resource : resources.sampled_images) {
				const const std::string& name = resource.name;
				const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
				const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
				i32 member_count = static_cast<u32>(buffer_type.member_types.size());
				u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

				u32 dimension = base_type.image.dim;
				u32 array_size = buffer_type.array[0];

				if (array_size == 0) {
					array_size = 1;
				} else {
					__debugbreak();
				}
				//__debugbreak();
			}

			//
			for (const spirv_cross::Resource& resource : resources.push_constant_buffers) {
				__debugbreak();
				const std::string& buffer_name = resource.name;
				const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
				u32 buffer_size = (u32)compiler.get_declared_struct_size(buffer_type);
				u32 member_count = uint32_t(buffer_type.member_types.size());
				u32 buffer_offset = 0;
				if (m_push_constant_ranges.size() != 0) {
					buffer_offset = m_push_constant_ranges.back().offset + m_push_constant_ranges.back().size;
				}
				PushConstantRange& push_constant_range = m_push_constant_ranges.emplace_back();
				push_constant_range.shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
				push_constant_range.size = buffer_size - buffer_offset;
				push_constant_range.offset = buffer_offset;
			}
		}

	}

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	shader_stages.resize(m_code.m_modules.size());
	for (u32 i = 0; i < shader_stages.size(); i++) {
		const VkPipelineShaderStageCreateInfo stage_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = from_SHADER_STAGE(m_code.m_modules[i].m_stage),
			.module = create_shader_module_from_spirv(device.m_handle, std::get<std::vector<u32>>(m_code.m_modules[i].m_code)),
			.pName = "main",
			.pSpecializationInfo = nullptr,
		};
		shader_stages[i] = stage_info;
	}

	const VkVertexInputBindingDescription binding_description = get_binding_description(vertex_format);
	const std::vector<VkVertexInputAttributeDescription> attribute_descriptions = get_attribute_descriptions(vertex_format);

	const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding_description,
		.vertexAttributeDescriptionCount = static_cast<u32>(attribute_descriptions.size()),
		.pVertexAttributeDescriptions = attribute_descriptions.data(),
	};

	const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	//NOTE: For OpenGL compatibility. make .height *= -1 and .y += .height
	const VkViewport viewport = {
		.x = 0.0f,
		.y = static_cast<f32>(extent.height),
		.width = static_cast<f32>(extent.width),
		.height = -static_cast<f32>(extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const VkRect2D scissor = {
		.offset = { 0, 0 },
		.extent = extent,
	};

	const VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE, // VK_CULL_MODE_BACK_BIT, //NOTE: after debugging make it cull mode back again
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = {},
		.depthBiasClamp = {},
		.depthBiasSlopeFactor = {},
		.lineWidth = 1.0f,
	};

	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 0,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = 0,
		.alphaToOneEnable = 0,
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
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &color_blend_attachment,
		.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
	};

	std::vector<VkPushConstantRange> vulkan_push_constant_ranges(m_push_constant_ranges.size());
	for (u32 i = 0; i < m_push_constant_ranges.size(); i++) {
		const PushConstantRange& push_constant_range = m_push_constant_ranges[i];
		VkPushConstantRange& vulkan_push_constant_range = vulkan_push_constant_ranges[i];

		vulkan_push_constant_range.stageFlags = push_constant_range.shader_stage;
		vulkan_push_constant_range.offset = push_constant_range.offset;
		vulkan_push_constant_range.size = push_constant_range.size;
	}
	const VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptor_set_layout.m_handle,
		.pushConstantRangeCount = static_cast<u32>(vulkan_push_constant_ranges.size()),
		.pPushConstantRanges = vulkan_push_constant_ranges.data(),
	};
	result = vkCreatePipelineLayout(device.m_handle, &pipeline_layout_info, nullptr, &m_layout);
	if (result != VK_SUCCESS) __debugbreak();

	const VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stageCount = static_cast<u32>(shader_stages.size()),
		.pStages = shader_stages.data(),
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly,
		.pTessellationState = nullptr,
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &color_blending,
		.pDynamicState = nullptr,
		.layout = m_layout,
		.renderPass = render_pass.m_handle,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0,
	};

	result = vkCreateGraphicsPipelines(device.m_handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) __debugbreak();

	//vkDestroyShaderModule(device.m_handle, frag_shader_module, nullptr);
	//vkDestroyShaderModule(device.m_handle, vert_shader_module, nullptr);
	m_device = &device;
}

auto VulkanPipeline::deinit() -> void {
	vkDestroyPipeline(m_device->m_handle, m_handle, nullptr);
	vkDestroyPipelineLayout(m_device->m_handle, m_layout, nullptr);
}

auto VulkanPipeline::operator=(const VulkanPipeline& o) {
	__debugbreak();
}

}