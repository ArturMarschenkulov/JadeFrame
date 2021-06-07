#include "vulkan_logical_device.h"
#include "vulkan_shared.h" // TODO: This should be temprary
#include "vulkan_context.h"

#include <Windows.h> // TODO: Try to remove it

#include <iostream>

#include <thread>
#include <future>
#include <set>

#undef min
#undef max

static const i32 MAX_FRAMES_IN_FLIGHT = 2;



auto VkResult_to_string(VkResult x) {
	std::string str;
	switch (x) {
		case VK_SUCCESS: str = "VK_SUCCESS"; break;
		case VK_NOT_READY: str = "VK_NOT_READY"; break;
		case VK_TIMEOUT: str = "VK_TIMEOUT"; break;
		case VK_EVENT_SET: str = "VK_EVENT_SET"; break;
		case VK_EVENT_RESET: str = "VK_EVENT_RESET"; break;
		case VK_INCOMPLETE: str = "VK_INCOMPLETE"; break;
		case VK_SUBOPTIMAL_KHR: str = "VK_SUBOPTIMAL_KHR"; break;
		case VK_ERROR_OUT_OF_DATE_KHR: str = "VK_ERROR_OUT_OF_DATE_KHR"; break;
		default: __debugbreak(); str = ""; break;
	}
	return str;
}

//const std::vector<Vertex> g_vertices = {
//	{{+0.0f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
//	{{+0.5f, +0.5f}, {+0.0f, +1.0f, +0.0f}},
//	{{-0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}}
//};

const std::vector<VVertex> g_vertices = {
	{{-0.5f, -0.5f}, {+1.0f, +0.0f, +0.0f}},
	{{+0.5f, -0.5f}, {+0.0f, +1.0f, +0.0f}},
	{{+0.5f, +0.5f}, {+0.0f, +0.0f, +1.0f}},

	{{-0.5f, +0.5f}, {+1.0f, +1.0f, +1.0f}},
};
const std::vector<u16> g_indices = {
	0, 1, 2, 
	2, 3, 0,
};

static auto find_memory_type(const VulkanPhysicalDevice & physical_device, u32 type_filter, VkMemoryPropertyFlags properties) -> u32 {
	auto& mem_props = physical_device.m_memory_properties;
	for (u32 i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type!");
}

//class VulkanBuffer {
//	VkBuffer m_buffer = VK_NULL_HANDLE;
//	VkDeviceMemory m_memory = VK_NULL_HANDLE;
//	VkDeviceSize m_buffer_size = 0;
//
//	VkDevice m_device = VK_NULL_HANDLE;
//	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
//	VkBufferUsageFlags m_usage = 0;
//};

static auto choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) -> VkSurfaceFormatKHR {
	for (u32 i = 0; i < available_formats.size(); i++) {
		if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return available_formats[i];
		}
	}
	return available_formats[0];
}
static auto choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) -> VkPresentModeKHR {
	for (u32 i = 0; i < available_present_modes.size(); i++) {
		if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return available_present_modes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

static auto choose_swap_extent(HWND window_handle, const VkSurfaceCapabilitiesKHR& capabilities) -> VkExtent2D {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		RECT area;
		GetClientRect(window_handle, &area);
		width = area.right;
		height = area.bottom;
		//glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actual_extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}


auto VulkanLogicalDevice::create_swapchain(const VulkanInstance& instance) -> void {
	VkResult result;

	const auto& pd = instance.m_physical_device;
	//SwapChainSupportDetails& swapchain_support = m_physical_device_p->m_swapchain_support_details;

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(pd.m_surface_formats);
	VkPresentModeKHR present_mode = choose_swap_present_mode(pd.m_present_modes);
	VkExtent2D extent = choose_swap_extent(instance.m_window_handle, pd.m_surface_capabilities);

	u32 image_count = pd.m_surface_capabilities.minImageCount + 1;
	if (pd.m_surface_capabilities.maxImageCount > 0 && image_count > pd.m_surface_capabilities.maxImageCount) {
		image_count = pd.m_surface_capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	//create_info.pNext;
	//create_info.flags;
	create_info.surface = instance.m_surface.m_surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//create_info.imageSharingMode;
	//create_info.queueFamilyIndexCount;
	//create_info.pQueueFamilyIndices;
	//create_info.preTransform;
	//create_info.compositeAlpha;
	//create_info.presentMode;
	//create_info.clipped;
	//create_info.oldSwapchain;


	const QueueFamilyIndices& indices = instance.m_physical_device.m_queue_family_indices;
	u32 queue_family_indices[] = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = pd.m_surface_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(m_handle, &create_info, nullptr, &m_swapchain);
	if (result != VK_SUCCESS) {
		std::cout << "failed to create swap chain!" << std::endl;
		__debugbreak();
	}

	result = vkGetSwapchainImagesKHR(m_handle, m_swapchain, &image_count, nullptr);
	m_swapchain_images.resize(image_count);
	result = vkGetSwapchainImagesKHR(m_handle, m_swapchain, &image_count, m_swapchain_images.data());
	if (VK_SUCCESS != result) {
		__debugbreak();
	}

	m_swapchain_image_format = surface_format.format;
	m_swapchain_extent = extent;
}

auto VulkanLogicalDevice::create_image_views() -> void {
	VkResult result;

	m_swapchain_image_views.resize(m_swapchain_images.size());

	for (size_t i = 0; i < m_swapchain_images.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchain_images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchain_image_format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(m_handle, &createInfo, nullptr, &m_swapchain_image_views[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}
const char* vs =
R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

//vec2 positions[3] = vec2[](
//    vec2(0.0, -0.5),
//    vec2(0.5, 0.5),
//    vec2(-0.5, 0.5)
//);
//
//vec3 colors[3] = vec3[](
//    vec3(1.0, 0.0, 0.0),
//    vec3(0.0, 1.0, 0.0),
//    vec3(0.0, 0.0, 1.0)
//);

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
	fragColor = inColor;
}
)";

const char* fs =
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
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = spirv.size() * 4; // NOTE: It has to be multiplied by 4!
	create_info.pCode = spirv.data();
	VkShaderModule shader_module;
	result = vkCreateShaderModule(device, &create_info, nullptr, &shader_module);
	if (result != VK_SUCCESS) {
		__debugbreak();
	}
	return shader_module;
}
#include <shaderc/shaderc.hpp>

static auto string_to_SPIRV(const char* code, u32 i) -> std::vector<u32> {
	using namespace shaderc;
	Compiler compiler;
	CompileOptions options;
	options.SetOptimizationLevel(shaderc_optimization_level_size);
	SpvCompilationResult result = compiler.CompileGlslToSpv(std::string(code), (shaderc_shader_kind)i /*shaderc_vertex_shader*/, "", options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		__debugbreak();
		return std::vector<u32>();
	}

	std::vector<u32> res = { result.cbegin(), result.cend() };
	return res;
}

auto VulkanLogicalDevice::create_render_pass() -> void {
	VkResult result;

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = m_swapchain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	result = vkCreateRenderPass(m_handle, &render_pass_info, nullptr, &m_render_pass);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

auto VulkanLogicalDevice::create_graphics_pipeline() -> void {
	VkResult result;
	//auto tm = &JadeFrame::get_singleton()->m_apps[0]->m_time_manager;
	//auto time_0 = tm->get_time();

#if 1
	std::future<std::vector<u32>> vert_shader_spirv = std::async(std::launch::async, string_to_SPIRV, vs, 0);
	std::future<std::vector<u32>> frag_shader_spirv = std::async(std::launch::async, string_to_SPIRV, fs, 1);

	VkShaderModule vert_shader_module = create_shader_module_from_spirv(m_handle, vert_shader_spirv.get());
	VkShaderModule frag_shader_module = create_shader_module_from_spirv(m_handle, frag_shader_spirv.get());
#else
	std::vector<u32> vert_shader_spirv = string_to_SPIRV(vs, 0);
	std::vector<u32> frag_shader_spirv = string_to_SPIRV(fs, 1);

	VkShaderModule vert_shader_module = create_shader_module_from_spirv(m_device, vert_shader_spirv);
	VkShaderModule frag_shader_module = create_shader_module_from_spirv(m_device, frag_shader_spirv);
#endif

	//auto time_1 = tm->get_time();
	//std::cout << "it took " << time_1 - time_0 << std::endl;

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.pNext = nullptr;
	vert_shader_stage_info.flags = 0;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";
	vert_shader_stage_info.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.pNext = nullptr;
	frag_shader_stage_info.flags = 0;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";
	frag_shader_stage_info.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shader_stages[] = {
		vert_shader_stage_info,
		frag_shader_stage_info,
	};

	VkVertexInputBindingDescription binding_description = VVertex::get_binding_description();
	std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = VVertex::get_attribute_descriptions();

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.pNext = nullptr;
	vertex_input_info.flags = 0;
	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.pNext = nullptr;
	input_assembly.flags = 0;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchain_extent.width;
	viewport.height = (float)m_swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchain_extent;

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	result = vkCreatePipelineLayout(m_handle, &pipeline_layout_info, nullptr, &m_pipeline_layout);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = m_pipeline_layout;
	pipeline_info.renderPass = m_render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	result = vkCreateGraphicsPipelines(m_handle, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(m_handle, frag_shader_module, nullptr);
	vkDestroyShaderModule(m_handle, vert_shader_module, nullptr);
}

auto VulkanLogicalDevice::create_framebuffers() -> void {
	VkResult result;

	m_framebuffers.resize(m_swapchain_image_views.size());

	for (size_t i = 0; i < m_swapchain_image_views.size(); i++) {
		VkImageView attachments[] = {
			m_swapchain_image_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.pNext = nullptr;
		framebuffer_info.flags = 0;
		framebuffer_info.renderPass = m_render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = m_swapchain_extent.width;
		framebuffer_info.height = m_swapchain_extent.height;
		framebuffer_info.layers = 1;

		result = vkCreateFramebuffer(m_handle, &framebuffer_info, nullptr, &m_framebuffers[i]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
struct VulFrameBuffer {
	VulFrameBuffer(VkDevice device, VkRenderPass render_pass, VkImageView* attachments, VkExtent2D extend) {
		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.pNext = nullptr;
		framebuffer_info.flags = 0;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = extend.width;
		framebuffer_info.height = extend.height;
		framebuffer_info.layers = 1;

		VkResult result = vkCreateFramebuffer(device, &framebuffer_info, nullptr, &m_handle);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
	~VulFrameBuffer() {

	}
	VkFramebuffer m_handle;
};

auto VulkanLogicalDevice::create_command_pool(const VulkanPhysicalDevice& physical_device) -> void {
	VkResult result;
	QueueFamilyIndices queue_family_indices = physical_device.m_queue_family_indices;

	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.pNext = nullptr;
	pool_info.flags = 0; // Optional
	pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

	result = vkCreateCommandPool(m_handle, &pool_info, nullptr, &m_command_pool);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

auto VulkanLogicalDevice::create_command_buffers() -> void {
	VkResult result;
	m_command_buffers.resize(m_framebuffers.size());

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.commandPool = m_command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)m_command_buffers.size();

	result = vkAllocateCommandBuffers(m_handle, &alloc_info, m_command_buffers.data());
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < m_command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.pNext = nullptr;
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		result = vkBeginCommandBuffer(m_command_buffers[i], &begin_info);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = m_render_pass;
		render_pass_info.framebuffer = m_framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = m_swapchain_extent;
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clear_color;

		vkCmdBeginRenderPass(m_command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		{
			vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);
			VkBuffer vertex_buffers[] = { m_vertex_buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertex_buffers, offsets);
			//vkCmdDraw(m_command_buffers[i], static_cast<u32>(g_vertices.size()), 1, 0, 0);

			vkCmdBindIndexBuffer(m_command_buffers[i], m_index_buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(m_command_buffers[i], static_cast<uint32_t>(g_indices.size()), 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(m_command_buffers[i]);

		if (vkEndCommandBuffer(m_command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

auto VulkanLogicalDevice::create_sync_objects() -> void {
	m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	m_images_in_flight.resize(m_swapchain_images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_info.pNext = nullptr;
	semaphore_info.flags = 0;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkResult res_0 = vkCreateSemaphore(m_handle, &semaphore_info, nullptr, &m_image_available_semaphores[i]);
		VkResult res_1 = vkCreateSemaphore(m_handle, &semaphore_info, nullptr, &m_render_finished_semaphores[i]);
		VkResult res_2 = vkCreateFence(m_handle, &fence_info, nullptr, &m_in_flight_fences[i]);

		if (res_0 != VK_SUCCESS ||
			res_1 != VK_SUCCESS ||
			res_2 != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

auto VulkanLogicalDevice::recreate_swapchain() -> void {
	VkResult result;
	std::cout << " lololo " << std::endl;
	i32 width = 0;
	i32 height = 0;
	RECT area;
	GetClientRect(m_instance_p->m_window_handle, &area);
	width = area.right;
	height = area.bottom;

	result = vkDeviceWaitIdle(m_handle);

	this->cleanup_swapchain();

	this->create_swapchain(*m_instance_p);
	this->create_image_views();
	this->create_render_pass();
	this->create_graphics_pipeline();
	this->create_framebuffers();
	this->create_command_buffers();

	m_images_in_flight.resize(m_swapchain_images.size(), VK_NULL_HANDLE);
}
auto VulkanLogicalDevice::cleanup_swapchain() -> void {
	for (auto framebuffer : m_framebuffers) {
		vkDestroyFramebuffer(m_handle, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(m_handle, m_command_pool, static_cast<uint32_t>(m_command_buffers.size()), m_command_buffers.data());

	vkDestroyPipeline(m_handle, m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_handle, m_pipeline_layout, nullptr);
	vkDestroyRenderPass(m_handle, m_render_pass, nullptr);

	for (auto image_view : m_swapchain_image_views) {
		vkDestroyImageView(m_handle, image_view, nullptr);
	}

	vkDestroySwapchainKHR(m_handle, m_swapchain, nullptr);
}


auto VulkanLogicalDevice::create_vertex_buffer(const std::vector<VVertex>& vertices) -> void {
	VkResult result;

	VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	this->create_buffer(
		buffer_size, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer,
		staging_buffer_memory
	);

	void* data;
	result = vkMapMemory(m_handle, staging_buffer_memory, 0, buffer_size, 0, &data);
	{
		memcpy(data, vertices.data(), (size_t)buffer_size);
	}
	vkUnmapMemory(m_handle, staging_buffer_memory);
	this->create_buffer(
		buffer_size, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		m_vertex_buffer,
		m_vertex_buffer_memory
	);
	this->copy_buffer(staging_buffer, m_vertex_buffer, buffer_size);

	vkDestroyBuffer(m_handle, staging_buffer, nullptr);
	vkFreeMemory(m_handle, staging_buffer_memory, nullptr);
}

auto VulkanLogicalDevice::create_index_buffer(const std::vector<u16>& indices) -> void {
	VkResult result;

	VkDeviceSize buffer_size = sizeof(indices[0]) * indices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	this->create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer,
		staging_buffer_memory
	);

	void* data;
	result = vkMapMemory(m_handle, staging_buffer_memory, 0, buffer_size, 0, &data);
	{
		memcpy(data, indices.data(), (size_t)buffer_size);
	}
	vkUnmapMemory(m_handle, staging_buffer_memory);
	this->create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_index_buffer,
		m_index_buffer_memory
	);
	this->copy_buffer(staging_buffer, m_index_buffer, buffer_size);

	vkDestroyBuffer(m_handle, staging_buffer, nullptr);
	vkFreeMemory(m_handle, staging_buffer_memory, nullptr);
}

auto VulkanLogicalDevice::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory) -> void {
	VkResult result;
	
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.pNext = nullptr;
	buffer_info.flags = 0;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_info.queueFamilyIndexCount = 0;
	buffer_info.pQueueFamilyIndices = nullptr;

	result = vkCreateBuffer(m_handle, &buffer_info, nullptr, &buffer);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}


	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(m_handle, buffer, &mem_requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = find_memory_type(*m_physical_device_p, mem_requirements.memoryTypeBits, properties);

	result = vkAllocateMemory(m_handle, &alloc_info, nullptr, &buffer_memory);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	result = vkBindBufferMemory(m_handle, buffer, buffer_memory, 0);
}

auto VulkanLogicalDevice::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void {
	VkResult result;

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = m_command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	result = vkAllocateCommandBuffers(m_handle, &alloc_info, &command_buffer);
	if (result != VK_SUCCESS){
		__debugbreak();
	}

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	result = vkBeginCommandBuffer(command_buffer, &begin_info);
	if (result != VK_SUCCESS) {
		__debugbreak();
	}

	{
		VkBufferCopy copy_region = {};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;
		vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
	}
	result = vkEndCommandBuffer(command_buffer);
	if (result != VK_SUCCESS) {
		__debugbreak();
	}

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphics_queue);

	vkFreeCommandBuffers(m_handle, m_command_pool, 1, &command_buffer);
}

auto VulkanLogicalDevice::draw_frame() -> void {
	VkResult result;

	//prepare buffers
	u32 image_index;
	result = vkWaitForFences(m_handle, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);
	{
		result = vkAcquireNextImageKHR(m_handle, m_swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &image_index);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
			this->recreate_swapchain();
			return;
		} else if (result == VK_SUBOPTIMAL_KHR) {
			//std::cout << "VK_SUBOPTIMAL_KHR" << std::endl;
			//this->recreate_swapchain();
		} else if (result == VK_SUCCESS) {
		} else {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}
	//~prepare buffers

	if (m_images_in_flight[image_index] != VK_NULL_HANDLE) {
		result = vkWaitForFences(m_handle, 1, &m_images_in_flight[image_index], VK_TRUE, UINT64_MAX);
	}
	VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame] };
	VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSwapchainKHR swapchains[] = { m_swapchain };
	// swap buffers

	{
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &m_command_buffers[image_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		result = vkResetFences(m_handle, 1, &m_in_flight_fences[m_current_frame]);

		result = vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fences[m_current_frame]);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}

	{
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr;


		result = vkQueuePresentKHR(m_present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebuffer_resized) {
			m_framebuffer_resized = false;
			std::cout << "recreate because of vkQueuePresentKHR" << std::endl;
			__debugbreak();
			this->recreate_swapchain();
		} else if (result != VK_SUCCESS) {
			std::cout << "failed to present swap chain image!" << std::endl;
			__debugbreak();
		}
	}
	m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

	//~swap buffer
}

auto VulkanLogicalDevice::init(const VulkanInstance& instance) -> void {
	VkResult  result;

	QueueFamilyIndices indices = instance.m_physical_device.m_queue_family_indices;

	std::set<u32> unique_queue_families = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	f32 queue_priority = 1.0_f32;
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	for (u32 queue_familiy : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_familiy;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures devices_features = {};
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.queueCreateInfoCount = static_cast<u32>(queue_create_infos.size());;
	create_info.pEnabledFeatures = &devices_features;
	create_info.enabledExtensionCount = static_cast<u32>(g_device_extensions.size());
	create_info.ppEnabledExtensionNames = g_device_extensions.data();
	create_info.enabledLayerCount = g_enable_validation_layers ? static_cast<uint32_t>(g_validation_layers.size()) : 0;
	create_info.ppEnabledLayerNames = g_enable_validation_layers ? g_validation_layers.data() : nullptr;

	result = vkCreateDevice(instance.m_physical_device.m_handle, &create_info, nullptr, &m_handle);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(m_handle, indices.graphics_family.value(), 0, &m_graphics_queue);
	vkGetDeviceQueue(m_handle, indices.present_family.value(), 0, &m_present_queue);

	m_physical_device_p = &instance.m_physical_device;
	m_instance_p = &instance;

	this->create_swapchain(instance);
	this->create_image_views();
	this->create_render_pass();
	this->create_graphics_pipeline();
	this->create_framebuffers();
	this->create_command_pool(instance.m_physical_device);
	this->create_vertex_buffer(g_vertices);
	this->create_index_buffer(g_indices);
	this->create_command_buffers();
	this->create_sync_objects();

}

auto VulkanLogicalDevice::deinit() -> void {
	this->cleanup_swapchain();
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_handle, m_render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(m_handle, m_image_available_semaphores[i], nullptr);
		vkDestroyFence(m_handle, m_in_flight_fences[i], nullptr);
	}
	vkDestroyCommandPool(m_handle, m_command_pool, nullptr);
	vkDestroyPipeline(m_handle, m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_handle, m_pipeline_layout, nullptr);
	vkDestroyRenderPass(m_handle, m_render_pass, nullptr);
	for (u32 i = 0; i < m_swapchain_image_views.size(); i++) {
		vkDestroyImageView(m_handle, m_swapchain_image_views[i], nullptr);
	}
	vkDestroySwapchainKHR(m_handle, m_swapchain, nullptr);
	vkDestroyDevice(m_handle, nullptr);
}