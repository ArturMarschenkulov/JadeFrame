#include "pch.h"
#include "vulkan_pipeline.h"
#include "vulkan_logical_device.h"
#include "vulkan_physical_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_descriptor_set.h"

// #include "extern/SPIRV-Cross/spirv_glsl.hpp"
// #include "extern/SPIRV-Cross/spirv_hlsl.hpp"
// #include "extern/SPIRV-Cross/spirv_msl.hpp"

#include "JadeFrame/utils/assert.h"


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
	if (result != VK_SUCCESS) assert(false);
	return shader_module;
}

#if 0
static auto debug_print_resources(const spirv_cross::ShaderResources& resources) -> void {

	for (const spirv_cross::Resource& resource : resources.uniform_buffers) {
		const std::string& name = resource.name;
		Logger::info("uniform_buffers {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.storage_buffers) {
		const std::string& name = resource.name;
		Logger::info("storage_buffers {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.stage_inputs) {
		const std::string& name = resource.name;
		Logger::info("stage_inputs {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.stage_outputs) {
		const std::string& name = resource.name;
		Logger::info("stage_outputs {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.subpass_inputs) {
		const std::string& name = resource.name;
		Logger::info("subpass_inputs {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.storage_images) {
		const std::string& name = resource.name;
		Logger::info("storage_images {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.sampled_images) {
		const std::string& name = resource.name;
		Logger::info("sampled_images {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.atomic_counters) {
		const std::string& name = resource.name;
		Logger::info("atomic_counters {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.acceleration_structures) {
		const std::string& name = resource.name;
		Logger::info("acceleration_structures {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.push_constant_buffers) {
		const std::string& name = resource.name;
		Logger::info("push_constant_buffers {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.separate_images) {
		const std::string& name = resource.name;
		Logger::info("separate_images {}", name);
	}
	for (const spirv_cross::Resource& resource : resources.separate_samplers) {
		const std::string& name = resource.name;
		Logger::info("separate_samplers {}", name);
	}

	for (const spirv_cross::BuiltInResource& resource : resources.builtin_inputs) {
		const std::string& name = resource.resource.name;
		Logger::info("builtin_inputs {}", name);
	}
	for (const spirv_cross::BuiltInResource& resource : resources.builtin_outputs) {
		const std::string& name = resource.resource.name;
		Logger::info("builtin_outputs {}", name);
	}
}
#endif

static auto from_SHADER_STAGE(SHADER_STAGE stage) -> VkShaderStageFlagBits {
	VkShaderStageFlagBits result = {};
	switch (stage) {
		case SHADER_STAGE::VERTEX:
		{
			result = VK_SHADER_STAGE_VERTEX_BIT;
		} break;
		case SHADER_STAGE::FRAGMENT:
		{
			result = VK_SHADER_STAGE_FRAGMENT_BIT;
		} break;
		default: assert(false);
	}
	return result;
}


struct ReflectedCode {
	struct Input {
		std::string name;
		u32 location;
		u32 size; //in bytes
		SHADER_TYPE type;
	};
	struct SampledImage {
		std::string name;
		u32 binding = 0;
		u32 set = 0;
		u32 DescriptorSet = 0;
		u32 ArraySize = 0;
	};
	struct UniformBuffer {
		std::string name;
		u32 size;
		u32 binding;
		u32 set;
	};
	struct Module {
		SHADER_STAGE m_stage;

		std::vector<Input> m_inputs;
		std::vector<UniformBuffer> m_uniform_buffers;
		std::vector<SampledImage> m_sampled_images;
		std::vector<VkPushConstantRange> m_push_constant_ranges;
	};
	std::vector<Module> m_modules;
};
#if 0
static auto to_SHADER_TYPE(const spirv_cross::SPIRType& type, u32 rows, u32 columns) -> SHADER_TYPE {
	SHADER_TYPE result = SHADER_TYPE::NONE;
	if (columns == 1) {
		switch (type.basetype) {
			case spirv_cross::SPIRType::Float:
			{

				SHADER_TYPE arr[] = { SHADER_TYPE::FLOAT , SHADER_TYPE::FLOAT_2, SHADER_TYPE::FLOAT_3, SHADER_TYPE::FLOAT_4 };
				if (rows < 5) {
					result = arr[rows - 1];
				} else {
					JF_ASSERT(false, "this should not be reached!");
				}

			} break;
			default: JF_ASSERT(false, "this should not be reached!");
		}
	} else {
		JF_ASSERT(false, "not implemented yet!");
	}
	return result;
}
#endif
static auto reflect(const ShadingCode& /*code*/) -> ReflectedCode {
	ReflectedCode result = {};
#if 0
	result.m_modules.resize(code.m_modules.size());
	for (u32 i = 0; i < code.m_modules.size(); i++) {
		auto& current_module = code.m_modules[i];
		auto& current_module_code = std::get<std::vector<u32>>(current_module.m_code);
		auto& current_result_module = result.m_modules[i];

		spirv_cross::Compiler compiler(current_module_code);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		debug_print_resources(resources);

		result.m_modules[i].m_inputs.resize(resources.stage_inputs.size());
		for (u32 j = 0; j < resources.stage_inputs.size(); j++) {
			const spirv_cross::Resource& resource = resources.stage_inputs[j];

			const std::string& name = resource.name;

			const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
			const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
			i32 member_count = static_cast<u32>(buffer_type.member_types.size());
			u32 location = compiler.get_decoration(resource.id, spv::DecorationLocation);
			u32 size = (buffer_type.width / 8) * buffer_type.vecsize * buffer_type.columns;


			std::vector<ReflectedCode::Input>& inputs = current_result_module.m_inputs;
			inputs[j].name = name;
			inputs[j].location = location;
			inputs[j].size = size;
			inputs[j].type = to_SHADER_TYPE(buffer_type, buffer_type.vecsize, buffer_type.columns);
		}

		result.m_modules[i].m_uniform_buffers.resize(resources.uniform_buffers.size());
		for (u32 j = 0; j < resources.uniform_buffers.size(); j++) {
			const spirv_cross::Resource& resource = resources.uniform_buffers[j];

			const std::string& name = resource.name;
			const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
			i32 member_count = static_cast<u32>(buffer_type.member_types.size());
			u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			u32 size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));

			std::vector<ReflectedCode::UniformBuffer>& uniform_buffers = current_result_module.m_uniform_buffers;
			uniform_buffers[j].binding = binding;
			uniform_buffers[j].set = set;
			uniform_buffers[j].name = name;
			uniform_buffers[j].size = size;
		}


		result.m_modules[i].m_sampled_images.resize(resources.sampled_images.size());
		for (u32 j = 0; j < resources.sampled_images.size(); j++) {
			const spirv_cross::Resource& resource = resources.sampled_images[j];

			const std::string& name = resource.name;
			const spirv_cross::SPIRType& base_type = compiler.get_type(resource.base_type_id);
			const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.type_id);
			i32 member_count = static_cast<u32>(buffer_type.member_types.size());
			u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			u32 set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

			u32 dimension = base_type.image.dim;
			u32 array_size = buffer_type.array[0];

			if (array_size == 0) {
				array_size = 1;
			} else {
                assert(false);
			}
			//__debugbreak();
		}


		result.m_modules[i].m_push_constant_ranges.resize(resources.push_constant_buffers.size());
		for (u32 j = 0; j < resources.push_constant_buffers.size(); j++) {
			const spirv_cross::Resource& resource = resources.push_constant_buffers[j];

			const std::string& buffer_name = resource.name;
			const spirv_cross::SPIRType& buffer_type = compiler.get_type(resource.base_type_id);
			u32 buffer_size = (u32)compiler.get_declared_struct_size(buffer_type);
			u32 member_count = uint32_t(buffer_type.member_types.size());
			u32 buffer_offset = 0;

			std::vector<VkPushConstantRange>& push_constant_ranges = current_result_module.m_push_constant_ranges;
			push_constant_ranges[j].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			push_constant_ranges[j].size = buffer_size - buffer_offset;
			push_constant_ranges[j].offset = buffer_offset;
		}
	}
#endif
	return result;
}

static auto extract_descriptor_set_layouts(const VulkanLogicalDevice& device, const ReflectedCode& code)->std::array<VulkanDescriptorSetLayout, static_cast<u8>(DESCRIPTOR_SET_FREQUENCY::MAX)> {
	std::array<VulkanDescriptorSetLayout, static_cast<u8>(DESCRIPTOR_SET_FREQUENCY::MAX)> set_layouts;
	for (u32 i = 0; i < code.m_modules.size(); i++) {
		auto& curr_module = code.m_modules[i];

		for(u32 i = 0; i < curr_module.m_uniform_buffers.size(); i++) {
			const auto& curr_buffer = curr_module.m_uniform_buffers[i];

			const bool update_per_object = curr_buffer.set == static_cast<u8>(DESCRIPTOR_SET_FREQUENCY::PER_OBJECT);

			const VkDescriptorType& type = curr_buffer.set == update_per_object ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			set_layouts[curr_buffer.set].add_binding(
				curr_buffer.binding, 
				type,
				1, 
				from_SHADER_STAGE(curr_module.m_stage)
			);
		}
	}
	for(u32 i = 0; i < set_layouts.size(); i++) {
		set_layouts[i].init(device);
	}
	return set_layouts;
}



static auto check_compatiblity(
	const std::vector<ReflectedCode::Module>& modules,
	const VkVertexInputBindingDescription& input_bindings,
	const std::vector<VkVertexInputAttributeDescription>& input_attributes
) -> bool {
	bool compatible = true;

	const ReflectedCode::Module* vertex_module = nullptr;
	for (u32 i = 0; i < modules.size(); i++) {
		if (modules[i].m_stage == SHADER_STAGE::VERTEX) {
			vertex_module = &modules[i];
			break;
		}
	}

	if (vertex_module != nullptr) {
		u32 stride = 0;
		for (u32 i = 0; i < vertex_module->m_inputs.size(); i++) {
			stride += vertex_module->m_inputs[i].size;
		}
		if (input_bindings.stride != stride) {
			compatible = false;
		}

		if (input_attributes.size() == vertex_module->m_inputs.size()) {
			for (u32 i = 0; i < input_attributes.size(); i++) {
				if (input_attributes[i].format != SHADER_TYPE_to_VkFormat(vertex_module->m_inputs[i].type))
					compatible = false;
				if (input_attributes[i].location != vertex_module->m_inputs[i].location)
					compatible = false;
			}
		} else {
            assert(false);
		}

	} else {
		compatible = false;
	}

	return compatible;
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
	// Convert GLSL to SPIRV and save it in member variables
	// TODO: Think whether to extract it to somewhere else
	{
		std::vector<std::future<std::vector<u32>>> spirvs;
		spirvs.resize(code.m_modules.size());
		for (u32 i = 0; i < code.m_modules.size(); i++) {
			const std::string& str = std::get<std::string>(code.m_modules[i].m_code);
			spirvs[i] = std::async(std::launch::async, string_to_SPIRV, str, code.m_modules[i].m_stage);
		}
		m_code.m_modules.resize(spirvs.size());
		for (u32 i = 0; i < m_code.m_modules.size(); i++) {
			m_code.m_modules[i].m_stage = code.m_modules[i].m_stage;
			m_code.m_modules[i].m_code = spirvs[i].get();
		}
	}

	const ReflectedCode reflected_code = reflect(m_code);

	const std::array<VulkanDescriptorSetLayout, 4> set_layouts = extract_descriptor_set_layouts(device, reflected_code);

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	shader_stages.resize(m_code.m_modules.size());
	for (u32 i = 0; i < shader_stages.size(); i++) {
		shader_stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[i].pNext = nullptr;
		shader_stages[i].flags = 0;
		shader_stages[i].stage = from_SHADER_STAGE(m_code.m_modules[i].m_stage);
		shader_stages[i].module = create_shader_module_from_spirv(device.m_handle, std::get<std::vector<u32>>(m_code.m_modules[i].m_code));
		shader_stages[i].pName = "main";
		shader_stages[i].pSpecializationInfo = nullptr;
	}

	const VkVertexInputBindingDescription binding_description = get_binding_description(vertex_format);
	const std::vector<VkVertexInputAttributeDescription> attribute_descriptions = get_attribute_descriptions(vertex_format);

	bool compatible = check_compatiblity(reflected_code.m_modules, binding_description, attribute_descriptions);
	JF_ASSERT(compatible == true, "");





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
	constexpr bool gl_compat = true;
	const VkViewport viewport = {
		.x = 0.0f,
		.y = gl_compat ? static_cast<f32>(extent.height) : 0.0f,
		.width = static_cast<f32>(extent.width),
		.height = gl_compat ? -static_cast<f32>(extent.height): static_cast<f32>(extent.height),
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

	VkDescriptorSetLayout set_layout_handles[4];
	for(u32 i = 0; i < set_layouts.size(); i++) {
		set_layout_handles[i] = set_layouts[i].m_handle;
	}
	const VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.setLayoutCount = set_layouts.size(),
		.pSetLayouts = set_layout_handles,
		.pushConstantRangeCount = static_cast<u32>(vulkan_push_constant_ranges.size()),
		.pPushConstantRanges = vulkan_push_constant_ranges.data(),
	};
	result = vkCreatePipelineLayout(device.m_handle, &pipeline_layout_info, nullptr, &m_layout);
	if (result != VK_SUCCESS) assert(false);

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
	if (result != VK_SUCCESS) assert(false);

	for (u32 i = 0; i < shader_stages.size(); i++) {
		vkDestroyShaderModule(device.m_handle, shader_stages[i].module, nullptr);
	}

	m_device = &device;
}

auto VulkanPipeline::deinit() -> void {
	vkDestroyPipeline(m_device->m_handle, m_handle, nullptr);
	vkDestroyPipelineLayout(m_device->m_handle, m_layout, nullptr);
}

auto VulkanPipeline::operator=(const VulkanPipeline& /*o*/) {
    assert(false);
}

}